# Traffic Management System (Simulation)

A C++17 discrete-time traffic simulation for an academic software-engineering project.
It does not control real traffic infrastructure.

## Current status

Implemented: validated text scenarios, scheduled/seeded vehicle generation with optional
batch arrivals, unique IDs,
vehicle/route validation, FIFO entry queues, capacity-limited admission and deferred retries,
reset/reproducibility, timed signal phases with safety checks, validated road-network edges,
and movement over explicit or automatically selected minimum-travel-time routes through
permitted green phases, with road and configured downstream-lane capacity limits.

Queue-responsive green timing, congestion detection, scheduled road blocking, completed-trip
metrics and CSV reports are implemented. Emergency priority and incident-driven rerouting
of vehicles already travelling remain incomplete.

## Build, test, and run

Requires CMake 3.16+, a C++17 compiler, and network access for the first GoogleTest download.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/traffic_sim --validate config/demo_scenario.txt
./build/traffic_sim --scenario config/demo_scenario.txt --lane-status
./build/traffic_sim --scenario config/traffic_demo_scenario.txt --show-scenario
./build/traffic_sim --scenario config/traffic_demo_scenario.txt --trace
./build/traffic_sim --scenario config/demo_scenario.txt --trace
./build/traffic_sim --scenario config/traffic_demo_scenario.txt --lane-status
./build/traffic_sim --scenario config/seeded_scenario.txt
./build/traffic_sim --scenario config/batch_scenario.txt --lane-status
```

`--validate <file>` checks the complete scenario and prints its configuration counts without
generating vehicles or advancing the simulation. Invalid input returns a nonzero exit code.

The scheduled demo generates and admits 3 vehicles; its entry queue and deferred backlog
are empty at the end of the run. Add `--lane-status` to
show each lane's capacity, queued IDs, and deferred IDs in FIFO order. Without this flag,
output remains a concise aggregate summary. The seeded demo prints its
seed and generates a repeatable sequence. Vehicles may use explicit routes or automatic minimum-travel-time routing. See [scenario format](config/README.md).

`--show-scenario` prints the configured roads, capacities, entry lanes, arrival patterns,
routes, and signal phases without running the simulation. `--trace` prints that overview
first, then each tick's signal phase, generated and admitted IDs, vehicle road/state changes,
accumulated waiting time, and deferred IDs. This is a terminal view, not a graphical map.
The CLI shows a cumulative reached-destination count during the trace and final
reached-destination and in-transit totals, plus completed/unfinished counts and waiting metrics.
Add `--report <path.csv>` to save the final metrics as CSV.

For a compact traffic showcase, `traffic_demo_scenario.txt` uses 4 intersections, 4 roads,
two explicit routes, and two arrival streams. It generates 6 vehicles over 3 ticks. Run it
with `--show-scenario` to review the setup or add `--trace` to see each tick.

For a negative demo, run `traffic_sim --validate tests/fixtures/invalid_capacity.txt`.
The scenario deliberately gives a road a negative capacity; validation should reject it with
a line-numbered error and exit code 1. An unsupported CLI option is also rejected with exit
code 2, for example `traffic_sim --scenario config/demo_scenario.txt --bad-option`.

To build without downloading test dependencies, configure with `-DBUILD_TESTING=OFF`.
For static analysis, install Clang and configure a separate build:

```sh
cmake -S . -B build-analysis -DBUILD_TESTING=OFF -DTRAFFIC_ENABLE_ANALYZER=ON
cmake --build build-analysis --parallel 2
```

## Ownership and evidence

- **Aarya:** scenario/vehicle input and lane queues, FR-01–FR-04; shared FR-18 reproducibility.
- **Vijeth:** signal control, FR-05–FR-08.
- **Vaibhav:** road network representation and vehicle movement, FR-09–FR-12 (road network, movement, lane capacity and routing; [coverage matrix](docs/vaibhav-test-matrix.md)).
- **Vainik:** congestion, incidents, metrics, and reporting, FR-13–FR-17.

See [architecture](docs/architecture.md), [requirement traceability](docs/traceability.md),
and [Aarya's test evidence](docs/aarya-test-matrix.md). See [remaining work and current limitations](docs/remaining-work.md)
for known missing features and usability gaps.
Explicitly and automatically routed vehicles can move through green intersections while
respecting road and configured downstream-lane capacity. See the requirement matrices for
verification scope and remaining integration limitations.

Use focused feature branches, meaningful commits, and teammate review before merge.
Local tests, CI configuration, and successful remote CI runs are separate evidence.
