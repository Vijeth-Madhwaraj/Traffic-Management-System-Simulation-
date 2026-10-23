# Traffic Management System (Simulation)

A C++17 discrete-time traffic simulation for an academic software-engineering project.
It does not control real traffic infrastructure.

## Current status

Implemented: validated text scenarios, scheduled/seeded vehicle generation, unique IDs,
vehicle/route validation, FIFO entry queues, capacity-limited admission and deferred retries,
reset/reproducibility, and timed signal phases with safety checks.

Routing, vehicle movement, adaptive signals, emergency priority, congestion and incident
response remain incomplete. Metrics have basic counters/CSV helpers, but full trip metrics
are not integrated. The CLI reports **input/admission counts**, not completed journeys.

## Build, test, and run

Requires CMake 3.16+, a C++17 compiler, and network access for the first GoogleTest download.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/traffic_sim --scenario config/demo_scenario.txt
./build/traffic_sim --scenario config/seeded_scenario.txt
```

The scheduled demo generates 3 vehicles, admits 1, and defers 2. The seeded demo prints its
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
- **Vaibhav:** road network, routing, and movement, FR-09–FR-12.
- **Vainik:** congestion, incidents, metrics, and reporting, FR-13–FR-17.

See [architecture](docs/architecture.md), [requirement traceability](docs/traceability.md),
and [Aarya's test evidence and remaining work](docs/aarya-test-matrix.md).
The first complete vehicle-to-destination milestone remains blocked on routing/movement.

Use focused feature branches, meaningful commits, and teammate review before merge.
Local tests, CI configuration, and successful remote CI runs are separate evidence.
