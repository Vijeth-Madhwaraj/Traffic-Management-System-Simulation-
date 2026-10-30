# Requirement traceability

The source requirements are FR-01 through FR-18 and NFR-01 through NFR-10 in the project SRS. This table combines verified input-module evidence with planned teammate coverage. A source file or test ID alone does not imply implementation.

| Requirement | Module/source | Tests/evidence |
|---|---|---|
| FR-01 | `src/input/scenario.cpp`, `src/simulation/simulation.cpp`, `src/main.cpp` | TC-01; `ScenarioTest.*`; CLI tests (passed) |
| FR-02 | `src/vehicle/vehicle_manager.cpp` | UT-A01, UT-A02, UT-A08 (passed) |
| FR-03 | `src/vehicle/vehicle_manager.cpp` | UT-A01, UT-A03, UT-A09 (module passed; production routing blocked) |
| FR-04 | `src/vehicle/lane_queue.cpp`, `src/vehicle/vehicle_manager.cpp` | UT-A04–UT-A07, UT-A10 (passed) |
| FR-18 | `src/vehicle/vehicle_manager.cpp`, `src/simulation/simulation.cpp`, `src/main.cpp` | UT-A02; TC-18 input event reproducibility (passed); full-trip comparison blocked |
| FR-05 | `SignalController::configure`, `SignalController::status` | TC-05 (planned), UT-VI01, UT-VI04 |
| FR-06 | `SignalController::advance`, `SignalController::permits` | TC-06 (planned), UT-VI02, UT-VI03, UT-VI04, UT-VI08 |
| FR-07, FR-08 | `SignalController` (planned) | UT-VI05 to UT-VI07, UT-VI10 |
| FR-09 | `RoadNetwork` | UT-VA01, UT-VA11; `vaibhav-fr09-test-cases.md` (passed in Windows/Linux CI) |
| FR-12 | `RoadNetwork` | UT-VA02 to UT-VA04 (planned) |
| FR-10 | `MovementEngine` | UT-VA05 to UT-VA07, UT-VA09 to UT-VA10 (passed in Windows/Linux CI) |
| FR-11 | `MovementEngine` | UT-VA08 road-capacity behavior (passed; lane coverage remains) |
| NFR-02 | `MovementEngine::advance` | UT-VA10 (passed in Windows/Linux CI) |
| FR-13 to FR-15 | `IncidentManager` and congestion policy | UT-VN01 to UT-VN06 |
| FR-16, FR-17 | `MetricsCollector` and report output | UT-VN07 to UT-VN10 |

The FR-05/06 implementations are in `include/traffic/signals/signal_controller.hpp` and `src/signals/signal_controller.cpp`; their automated tests are in `tests/unit/signals/signal_controller_test.cpp`. The signal matrix records an eight-test MinGW GCC Windows pass; the Linux CI job also passed those signal tests. FR-07/08 tests remain blocked pending implementation. Windows and Linux CI for PR #68 passed; continue adding integration/system results as features land.


FR-09–FR-12 evidence is in [Vaibhav's test matrix](vaibhav-test-matrix.md). FR-09 road data and FR-10 explicit-route movement passed Windows/Linux CI. FR-11 road-capacity checks are covered; downstream-lane coverage and FR-12 automatic route selection remain incomplete. `RoadNetwork::shortestRoute` is still a stub.

## Aarya implementation evidence

See [Aarya's matrix](aarya-test-matrix.md) for source/test mapping, execution environment,
NFR-01–NFR-10 evidence, and integration blockers. Aarya's recorded Linux run passed 38 CTest
cases, including the eight signal tests at that time. FR-01–FR-04 input behavior is implemented;
automatic routing, complete downstream lane-capacity handling, and completed-trip reporting
remain in progress.
Neither IT-01 nor IT-04 is claimed as passed. Module paths in the starter rows for other
owners remain plans unless their specific evidence establishes otherwise.
