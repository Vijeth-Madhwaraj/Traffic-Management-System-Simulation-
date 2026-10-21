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

## First integration milestone

Use a fixed seed and schedule: create one vehicle, enqueue it, permit its movement under a green phase, traverse a connected route, mark it complete, and increment throughput. Keep each step callable without running the full simulation so module behavior remains testable.
