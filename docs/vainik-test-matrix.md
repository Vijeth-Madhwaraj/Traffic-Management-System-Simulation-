# Vainik FR-13–FR-17 Test Matrix

This matrix records tests added for the congestion, incident, routing, metrics, and report contribution. Results are generated in the local Linux C++17 build; it does not claim that trip movement is complete.

| Requirement | Test coverage | Evidence / limitation |
|---|---|---|
| FR-13 Congestion detection | `CongestionDetectorTest.*` | Checks occupancy threshold, queue threshold, and invalid threshold updates. Congestion is sampled in `Simulation::step()` before signal advancement. Road occupancy changes still depend on movement logic, which is currently a stub. |
| FR-14 Incidents and blockages | `IncidentManagerTest.*` | Checks half-open activation/expiration interval, preserving a pre-disabled road, and rejecting invalid intervals. Unknown roads are safely ignored. |
| FR-15 Routing and rerouting | `RoadNetworkRoutingTest.*` | Checks shortest travel-time route, disabled-road avoidance, unreachable destination and same endpoint. Incident manager has rerouting integration, but full journey execution remains limited because `MovementEngine::advance()` is still a stub. |
| FR-16 Metrics | `MetricsCollectorTest.*`, `IncidentMetricsIntegrationTest.*` | Checks duplicate-completion protection, waiting average, generated/completed/unfinished counts, queue maximum and congestion-event count. Completion metrics only update when a vehicle is marked `Completed`; current movement engine does not yet complete trips. |
| FR-17 Reports | `ReportWriterTest.*`, CLI tests | Checks text output and CSV headers/values. CLI supports `--report <path>` and `--csv <path>`. |

## Build and test commands

```bash
cmake -S . -B build-tests -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST=./_vendor/googletest
cmake --build build-tests --parallel
ctest --test-dir build-tests --output-on-failure
```

The `FETCHCONTENT_SOURCE_DIR_GOOGLETEST` override is only needed in offline environments where CMake cannot fetch GoogleTest. In a normal online environment, the existing CMake FetchContent configuration can download the pinned dependency.

## Known integration limitation

`src/network/movement_engine.cpp` still contains a TODO stub. Consequently, vehicles do not yet traverse roads, consume travel time, update road occupancy, or transition to `VehicleState::Completed` in a real simulation. FR-13–FR-17 management logic, routing, metrics collection hooks and report generation are integrated, but full end-to-end trip completion cannot be claimed until movement behavior is implemented and tested.
