# Starter architecture

## Boundaries

- `Simulation` owns the clock and orchestrates one deterministic tick at a time.
- `VehicleManager` owns vehicle IDs and state; `LaneQueue` enforces FIFO and capacity.
- `RoadNetwork` owns road data and will provide enabled-edge routing.
- `SignalController` owns per-intersection phases and safety decisions.
- `MovementEngine` coordinates vehicles, signals, and road capacity.
- `IncidentManager` schedules road restrictions and later requests management responses.
- `MetricsCollector` records counters and exports a summary.
- `main` is the console entry point; parsing and user interaction should stay outside domain modules.

## Vehicle/input contract (FR-01–FR-04)

- Roads are directed edges. Entry lanes are FIFO waiting queues keyed by lane ID;
  their capacity is separate from occupancy while traveling. Scenario lanes reference
  their entry road and have capacity no greater than that road's capacity.
- `ScenarioLoader` returns a validated `Scenario` value. `Simulation::configureScenario`
  constructs replacement state before applying it, so invalid setup preserves the old run.
- `VehicleManager` owns IDs, vehicle templates, seeded RNG, entry queues and pending arrivals.
  Configure topology, then lanes, then schedules/streams. IDs start at 1 after reset.
- `generate(tick)` is an isolated generation API. Integrated callers use
  `advanceArrivals(tick, routes)` instead; mixing the two APIs in a run is rejected.
  Ticks start at zero and are consecutive; duplicate calls for the latest tick are no-ops.
- Scheduled arrivals run in stable tick/file order, followed by streams in sorted ID order.
  Each active stream consumes one RNG draw per tick, including probability 0 or 1.
  A successful decision creates `vehiclesPerArrival` vehicles (default 1, maximum 1000)
  with separate IDs. Batch size does not consume additional random draws. Scenario limits
  count interval length times batch size before accepting the configuration.
  Compare the upper 32 bits of `mt19937_64` against `floor(probability * 2^32)`.
- Admission uses supplied validated routes or invokes `RouteProvider(source, destination)`.
  Providers must return an enabled connected route beginning on the configured entry road.
  An empty result marks that vehicle `Unroutable` with `RouteFailure::NoRoute`.
  Invalid provider results (unknown/disabled/disconnected roads, wrong destination or entry
  road) mark only that vehicle `Unroutable` with `RouteFailure::InvalidRoute`; the bad route
  is cleared and valid arrivals continue. Explicit scenario routes are still validated
  before the run. Exceptions thrown by the provider roll back the whole admission tick,
  preserving IDs, queues and RNG state for retry.
- Full queues retain pending IDs in FIFO order. Each tick admits old pending vehicles
  before newer vehicles; emergency class uses the same FIFO policy.
- `laneSnapshots()` returns independent value snapshots sorted by lane ID, including capacity,
  queued IDs and pending IDs in FIFO order. Consumers can inspect them without mutating
  queues or advancing arrivals. `LaneQueue::enqueue` rejects zero and duplicate queued IDs.
- `lane(id)` exposes front/pop for movement integration. Reacquire lane/vehicle pointers
  after `advanceArrivals`, reset, or reconfiguration; those operations may replace storage.
  After removing an ID, the movement owner updates its registry state and road occupancy.
- `Simulation::step()` applies incidents, generates/admit arrivals, measures congestion,
  counts ready-to-cross vehicles by signal movement, advances signals with those queues, calls
  movement, observes queues, then increments the tick. Waiting/travel/complete-trip metrics
  remain integration work after movement.
- `run()` continues from the current tick; repeated calls at completion do nothing. `reset()`
  reloads the original scenario and restores seeded input state while retaining an injected
  route provider. Loading another scenario clears an injected provider.
- The CLI reports cumulative admission, vehicles that reached their destination, in-transit
  vehicles, and current queue/pending/unroutable counts. Admission is not completion. A run
  ends at its configured duration even with pending or in-transit vehicles.

Scenario grammar, bounds, and examples are in [`../config/README.md`](../config/README.md).
Signal movement IDs and complete conflict tables remain the intersection owner's contract;
FR-07 adaptive timing uses the optional scenario policy and does not change phase movements or
clearance intervals. Safe emergency-priority behavior remains separate work.

## Signal module integration contract (FR-05 / FR-06 / FR-07)

- The intersection/network owner must define stable movement IDs and provide a complete list of conflicting movement pairs for each controlled intersection. IDs in the signal plan must exactly match those used by `MovementEngine`.
- The scenario/configuration owner must create a non-empty phase plan with positive green, yellow, and all-red durations, then call `Simulation::configureSignals` before `run()`. Invalid plans throw `std::invalid_argument`; the CLI/configuration boundary should catch this and show a readable error.
- `Simulation` configures signals before the run and passes ready-to-cross movement queue counts to `SignalController::advance(tick, queues)` before `MovementEngine::advance` for that tick.
- When an `adaptive` policy is configured, a Green phase extends by its adjustment step when its permitted movements' ready queue reaches the configured threshold, and shortens by one step when that queue is empty. Each phase's green duration remains within the configured minimum and maximum. Yellow and all-red durations and the phase's permitted movements do not change.
- `MovementEngine` must call `permits(intersection, movement)` immediately before crossing. Only movements in the current Green phase are allowed; Yellow and Red deny movement.
- A phase cannot contain both sides of any declared conflict pair. The controller validates the supplied table; the intersection owner is responsible for making that table complete and correct.
- Emergency priority is separate work (FR-08). Its owner must request safe priority through the signal module and must not bypass phase or conflict checks.

## Movement contract (FR-10 / NFR-02)

- `MovementEngine::advance` is called once per non-decreasing simulation tick. Repeating the same tick is a no-op; moving backwards is rejected.
- Entry-lane vehicles begin on the first road in their validated route. Their initial entry does not cross an intersection signal.
- At a downstream intersection, the movement ID is `<incoming-road-id>-to-<outgoing-road-id>`. A vehicle changes roads only after the current road's travel time has elapsed, the next road is enabled and connected, the matching signal is Green, and the next road has capacity.
- A blocked vehicle remains on its current road, retains its route position, and accrues waiting ticks. Reaching the destination releases the occupied road and marks the vehicle `Completed`.
- Unit tests call `MovementEngine` directly with road, signal, and vehicle modules; they do not need to construct `Simulation`.

## First integration milestone

Use a fixed seed and schedule: create one vehicle, enqueue it, permit its movement under a green phase, traverse a connected route, mark it complete, and increment throughput. Keep each step callable without running the full simulation so module behavior remains testable.
