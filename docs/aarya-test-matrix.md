# Aarya input and vehicle requirement evidence

Updated 11 November 2026. Module names identify team responsibility; Git history records actual commit authors.
All 115 Linux CTest cases pass on the final implementation branch. Earlier execution notes remain in Git history.

| Requirement | Source | Executed evidence | Result |
|---|---|---|---|
| FR-01 | src/input/scenario.cpp | `ScenarioTest.TC_01_LoadsForwardReferencesClassesRoutesAndSignals` | Pass |
| FR-02 | src/vehicle/vehicle_manager.cpp | `VehicleInputTest.UT_A01_AssignsUniqueIdsAndPreservesAttributes` | Pass |
| FR-03 | src/vehicle/vehicle_manager.cpp; src/simulation/simulation.cpp | `ScenarioInputIntegrationTest.RoutesThroughPrescribedEntryDespiteCheaperOtherRoad` | Pass |
| FR-04 | src/vehicle/lane_queue.cpp; src/vehicle/vehicle_manager.cpp | `VehicleInputTest.UT_A04_A05_A06_RetriesPendingBeforeNewArrivalsWithoutChangingIds` | Pass |

FR-18 integrated reset clears lane occupancy, priority queues, incidents, counters and seeded generation; CSV replay is identical.
Module tests run independently, while integrated scenarios exercise the combined policies.
Current coverage, source fingerprints and per-case outcomes are in [reports](reports); the authoritative full mapping is [traceability.md](traceability.md).
Remote CI for this branch, Docker/Jenkins execution, human usability and owner/evaluator approvals are separate acceptance activities.
See [submission-checklist.md](submission-checklist.md) for pending actions.
