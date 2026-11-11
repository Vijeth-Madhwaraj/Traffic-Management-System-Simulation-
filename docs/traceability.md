# Requirement source and test traceability

Updated 11 November 2026. Historical evidence remains in Git history. Current results cover the final implementation branch; review and merge approval are separate.

| Requirement | Owner | Source | Exact executed test | Result |
|---|---|---|---|---|
| FR-01 | Aarya | src/input/scenario.cpp | `ScenarioTest.TC_01_LoadsForwardReferencesClassesRoutesAndSignals` | Pass |
| FR-02 | Aarya | src/vehicle/vehicle_manager.cpp | `VehicleInputTest.UT_A01_AssignsUniqueIdsAndPreservesAttributes` | Pass |
| FR-03 | Aarya / Vaibhav | src/vehicle/vehicle_manager.cpp; src/simulation/simulation.cpp | `ScenarioInputIntegrationTest.RoutesThroughPrescribedEntryDespiteCheaperOtherRoad` | Pass |
| FR-04 | Aarya | src/vehicle/lane_queue.cpp; src/vehicle/vehicle_manager.cpp | `VehicleInputTest.UT_A04_A05_A06_RetriesPendingBeforeNewArrivalsWithoutChangingIds` | Pass |
| FR-05 | Vijeth | src/signals/signal_controller.cpp | `SignalControllerTest.UT_VI01_StartsEachIntersectionInItsConfiguredInitialPhase` | Pass |
| FR-06 | Vijeth | src/signals/signal_controller.cpp | `SignalControllerTest.UT_VI03_UsesYellowAndAllRedClearanceBeforeNextGreen` | Pass |
| FR-07 | Vijeth | src/signals/signal_controller.cpp | `SignalControllerTest.TC_07_ExtendsGreenForLongQueuesUpToMaximum` | Pass |
| FR-08 | Vijeth | src/signals/signal_controller.cpp; src/network/movement_engine.cpp | `SignalControllerTest.TC_08_PrioritySkipsToRequestedPhaseAfterFullClearance` | Pass |
| FR-09 | Vaibhav | src/network/road_network.cpp | `RoadNetworkTest.UT_VA01_RegistersConnectedIntersectionsAndExposesRoadProperties` | Pass |
| FR-10 | Vaibhav | src/network/movement_engine.cpp | `MovementEngineTest.UT_VA05_EntersRoadAndCrossesOnlyOnPermittedGreen` | Pass |
| FR-11 | Vaibhav | src/network/movement_engine.cpp | `MovementEngineTest.UT_VA13_FR11_NFR02_DoesNotEnterFullDownstreamLane` | Pass |
| FR-12 | Vaibhav | src/network/road_network.cpp | `RoadNetworkTest.UT_VA02_ReturnsConnectedRouteWithMinimumTravelTime` | Pass |
| FR-13 | Vainik | src/management/congestion_detector.cpp | `CongestionDetectorTest.UT_VN01_DetectsRoadAtConfiguredOccupancyThreshold` | Pass |
| FR-14 | Vainik | src/management/incident_manager.cpp | `ScenarioInputIntegrationTest.ParsesAndValidatesScheduledIncidents` | Pass |
| FR-15 | Vainik / Vaibhav | src/simulation/simulation.cpp | `ScenarioInputIntegrationTest.TC_15_ReroutesExistingVehicleAfterLateIncidentAndReplays` | Pass |
| FR-16 | Vainik | src/management/metrics_collector.cpp; src/simulation/simulation.cpp | `ScenarioInputIntegrationTest.TC_16_CountsDeferredWaitingAndPeakQueueBeforeMovement` | Pass |
| FR-17 | Vainik | src/management/report_writer.cpp; src/main.cpp | `cli_report_write_failure` | Pass |
| FR-18 | Shared | src/vehicle/vehicle_manager.cpp; src/simulation/simulation.cpp | `ScenarioInputIntegrationTest.LaneCapacityPreservesSignalsIncidentsReportsAndReset` | Pass |

Additional FR-15 tests cover congestion without incidents and no-alternative road restoration. FR-17 open/write failures are checked by CLI regressions. FR-18 replay checks compare complete CSV output after reset.

| NFR | Evidence | Status |
|---|---|---|
| NFR-01 | Public headers, controlled Simulation ownership; architecture review | Reviewed |
| NFR-02 | Independently invoked GoogleTest module suites | Pass |
| NFR-03 | scripts/validate.py; four intersections, 500 generated vehicles; measured environment/time | Pass for recorded workload |
| NFR-04 | ScenarioTest invalid cases and signal connection rejection | Pass |
| NFR-05 | Linux GCC C++17/CMake build; Docker recipe | Native pass; container execution blocked locally |
| NFR-06 | CLI help/status/errors/reports; unfamiliar-user exercise | Automated pass; human review pending |
| NFR-07 | Bounded file parser, RAII and output failure regressions | Pass for reviewed scope |
| NFR-08 | All 18 rows resolve to existing source and executed exact test names | Pass mapping check; formal review pending |
| NFR-09 | clang-format 18.1.8; warnings as errors; 12 Clang analyses, zero diagnostics | Pass locally |
| NFR-10 | Printed/exported seed; deterministic arrival/reset and identical CSV checks | Pass |

Final evidence: [unit tests](reports/unit-tests.md), [system validation](reports/system-validation.md), [code coverage](reports/coverage/code-coverage.md), [branch coverage](reports/coverage/branch-coverage.md).
External/human acceptance items: [submission checklist](submission-checklist.md).
