# Traffic Management System (Simulation)

A C++17 discrete-time traffic simulation for an academic software-engineering project.
It does not control real traffic infrastructure.

## Current status

Implemented: validated text scenarios, scheduled/seeded vehicle generation, unique IDs,
vehicle/route validation, FIFO entry queues, capacity-limited admission and deferred retries,
reset/reproducibility, timed signal phases with safety checks, validated road-network edges,
and movement over explicit routes through permitted green phases.

Automatic route selection, full downstream lane-capacity handling, adaptive signals, emergency
priority, and incident-driven rerouting remain incomplete. Congestion detection and
scheduled road blocking are implemented. Metrics have basic counters/CSV
helpers, but completed-trip reporting is not integrated. The CLI reports **input/admission counts**.

## Build, test, and run

Requires CMake 3.16+, a C++17 compiler, and network access for the first GoogleTest download.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/traffic_sim --validate config/demo_scenario.txt
./build/traffic_sim --scenario config/demo_scenario.txt --lane-status
./build/traffic_sim --scenario config/seeded_scenario.txt
```

`--validate <file>` checks the complete scenario and prints its configuration counts without
generating vehicles or advancing the simulation. Invalid input returns a nonzero exit code.

The scheduled demo generates and admits 3 vehicles; its entry queue and deferred backlog
are empty at the end of the run. Add `--lane-status` to
show each lane's capacity, queued IDs, and deferred IDs in FIFO order. Without this flag,
output remains a concise aggregate summary. The seeded demo prints its
seed and generates a repeatable sequence. Explicit routes permit these input demonstrations
while automatic routing is still a stub. See [scenario format](config/README.md).

To build without downloading test dependencies, configure with `-DBUILD_TESTING=OFF`.
For static analysis, install Clang and configure a separate build:

```sh
cmake -S . -B build-analysis -DBUILD_TESTING=OFF -DTRAFFIC_ENABLE_ANALYZER=ON
cmake --build build-analysis --parallel 2
```

## Ownership and evidence

- **Aarya:** scenario/vehicle input and lane queues, FR-01–FR-04; shared FR-18 reproducibility.
- **Vijeth:** signal control, FR-05–FR-08.
- **Vaibhav:** road network representation and vehicle movement, FR-09–FR-10 (implemented; [coverage matrix](docs/vaibhav-test-matrix.md)); additional lane-capacity enforcement and routing, FR-11–FR-12.
- **Vainik:** congestion, incidents, metrics, and reporting, FR-13–FR-17.

See [architecture](docs/architecture.md), [requirement traceability](docs/traceability.md),
and [Aarya's test evidence and remaining work](docs/aarya-test-matrix.md).
Explicitly routed vehicles can now move through green intersections. Automatic route
selection, downstream lane-capacity handling, and completed-trip metrics remain in progress.

Use focused feature branches, meaningful commits, and teammate review before merge.
Local tests, CI configuration, and successful remote CI runs are separate evidence.
