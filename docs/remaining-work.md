# Remaining Work and Current Limitations

This note summarizes gaps visible in the current implementation and project requirements. It separates missing simulation behavior from usability features that can currently be worked around with configuration files or terminal commands.

## User-facing and scenario-building gaps

### Graphical front end

- The application is a command-line program. It has no graphical map, animated vehicles, clickable controls, or live dashboard.
- `--show-scenario` prints the network and configuration as text; `--trace` prints text updates for each tick.
- A front end could show intersections and directed roads, animate vehicles, color signal states, and display queues and destination counts.

### Interactive custom-scenario builder

- Scenarios can already be customized by editing text files using the format in [`../config/README.md`](../config/README.md).
- There is no form-based editor, visual road-network designer, or save/load workflow for people who do not want to edit the scenario text manually.
- The CLI can validate a scenario and preview its configuration, but it does not guide a user through creating one.

### Two-way road convenience

- Roads are represented as directed edges, such as `AB: A -> B`.
- A two-way connection can currently be approximated by defining two directed roads, one in each direction, with separate IDs and capacities (for example, `AB` and `BA`). Routes and signal movements must refer to the correct direction-specific roads.
- There is no first-class two-way-road record or interface that creates and manages both directions together. Shared physical-lane capacity between directions is also not modeled.

## Simulation behavior still missing or incomplete

### Routing and lane-capacity integration

- Automatic minimum-travel-time routing and deterministic equal-cost route selection are implemented.
- Configured downstream lanes limit in-transit occupancy separately from source waiting queues; road capacity remains the aggregate limit.
- Automatic routing still needs entry-road awareness when the globally shortest route starts on a different road from the configured entry lane.

### Adaptive signals and emergency priority — partial

- Configured fixed signal phases, green/yellow/all-red timing, and conflict safety checks are implemented.
- FR-07 queue-responsive green timing is implemented. Each phase's green duration can be
  extended for a ready queue or shortened when empty, within configured minimum and maximum
  limits. Phase order, permitted movements, yellow, and all-red clearance stay intact.
- Emergency vehicles are accepted as a vehicle class, but do not receive signal priority and use the same FIFO admission policy as standard vehicles.

### Incident response and rerouting — incomplete

- Congestion can be detected, and scheduled incidents can temporarily block roads.
- New arrivals can route around roads already blocked by an incident. Vehicles already travelling do not recalculate their assigned routes after a later blockage. Congestion detection does not yet trigger rerouting.

### Detailed trip metrics and reports — incomplete

- The CLI reports generated, admitted, queued, deferred, unroutable, reached-destination, and in-transit counts.
- Completed/unfinished counts, average waiting ticks, maximum queue length and CSV output are connected to the simulation. Waiting-time semantics need clarification, and the CLI must check buffered flush/close failures before reporting that a CSV was saved. Completed-trip travel times and throughput over time are not reported.

### Whole-system performance evidence — incomplete

- A performance workload scenario exists, but it does not by itself establish full-system performance compliance.
- Performance should be measured with movement, queues, signals, incidents, and reporting active, using documented machine and workload details.

## Implemented foundation

- Validated text scenarios, scheduled and seeded streams, batch arrivals, and unique vehicle IDs.
- FIFO entry queues with capacity limits and deferred retries.
- Fixed signal phase plans with safety checks and queue-responsive green durations bounded by configured limits.
- Directed road networks, deterministic minimum-travel-time routing and movement through permitted green phases, subject to road and configured downstream-lane capacity.
- Congestion detection, scheduled road blocking, scenario preview, tick trace, and basic destination counts.

## Possible implementation order

1. Add a visual scenario editor and map preview so users can create and inspect networks without hand-editing configuration files.
2. Add first-class two-way-road support with clearly defined capacity semantics for each direction.
3. Fix entry-road-aware automatic routing and validate downstream-lane integration.
4. Add safe emergency priority and incident-driven rerouting; consider further adaptive-signal
   refinements after FR-07 use and review.
5. Integrate complete trip metrics and run documented end-to-end performance evaluations.
