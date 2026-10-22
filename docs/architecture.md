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

## Shared decisions to settle before feature work

1. Whether a road is a directed edge and how multiple lanes are represented.
2. What occupancy/capacity means and whether vehicles consume capacity during travel.
3. Tick order and when arrivals, signal transitions, movement, and metrics are recorded.
4. Movement identifiers and the conflict-free phase table per intersection.
5. Whether yellow/all-red clearance is modeled and how emergency priority is requested safely.
6. Scenario file format and the minimum standard demo network.

## Signal module integration contract (FR-05 / FR-06)

- The intersection/network owner must define stable movement IDs and provide a complete list of conflicting movement pairs for each controlled intersection. IDs in the signal plan must exactly match those used by `MovementEngine`.
- The scenario/configuration owner must create a non-empty phase plan with positive green, yellow, and all-red durations, then call `Simulation::configureSignals` before `run()`. Invalid plans throw `std::invalid_argument`; the CLI/configuration boundary should catch this and show a readable error.
- `Simulation` must configure signals before the run and call `SignalController::advance(tick)` in non-decreasing tick order, before `MovementEngine::advance` for that tick.
- `MovementEngine` must call `permits(intersection, movement)` immediately before crossing. Only movements in the current Green phase are allowed; Yellow and Red deny movement.
- A phase cannot contain both sides of any declared conflict pair. The controller validates the supplied table; the intersection owner is responsible for making that table complete and correct.
- Emergency priority is separate work (FR-08). Its owner must request safe priority through the signal module and must not bypass phase or conflict checks.

## First integration milestone

Use a fixed seed and schedule: create one vehicle, enqueue it, permit its movement under a green phase, traverse a connected route, mark it complete, and increment throughput. Keep each step callable without running the full simulation so module behavior remains testable.
