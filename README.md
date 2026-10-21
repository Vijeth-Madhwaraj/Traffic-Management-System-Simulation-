# Traffic Management System (Simulation)

A C++17 discrete-time simulation of traffic moving through a configurable road network. It is intended for academic demonstrations of modular design, routing, signal control, incident handling, and measurable traffic performance. It does not control real traffic infrastructure.

## Current status

This repository is a project skeleton. The modules and shared types are laid out; traffic generation, routing, signal policies, movement, incident response, metrics, and scenario loading still need implementation.

## Requirements

- C++17 compiler (GCC or Clang recommended)
- CMake 3.16 or newer

## Build and run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/traffic_sim --help
```

The first working milestone is a deterministic path from vehicle generation through a lane queue and green signal to a destination, with throughput recorded. See [docs/architecture.md](docs/architecture.md) for module boundaries and [docs/traceability.md](docs/traceability.md) for the starter requirement mapping.

## Repository structure

```text
include/traffic/   Shared types and module interfaces
src/               Module implementations and CLI entry point
config/            Example scenario placeholder
docs/              Architecture and requirement traceability
tests/             Starter area for unit, integration, and system tests
```

## Team ownership

The project plan assigns vehicle/input management, signal control, road network/movement, and congestion/incidents/metrics to four members. Confirm names and ownership with the team before implementation; see `docs/architecture.md` for the proposed module split.

## Contribution workflow

Use short feature branches, focused pull requests, and teammate review. Agree shared IDs, tick ordering, capacity semantics, and signal safety rules before implementing module internals. Update traceability and test IDs as requirements are completed.

## Planned capabilities

Scenario configuration, seeded vehicle arrivals, capacity-limited lane queues, safe timed/adaptive signals, emergency priority, graph routing, movement, congestion/incident response, metrics, and text/CSV reporting are planned. None should be considered implemented until its behavior and evidence are added.
