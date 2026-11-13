# Vaibhav network and movement requirement evidence

Updated 11 November 2026. Module names identify team responsibility; Git history records actual commit authors.
All 115 Linux CTest cases pass on the final implementation branch. Earlier execution notes remain in Git history.

| Requirement | Source | Executed evidence | Result |
|---|---|---|---|
| FR-09 | src/network/road_network.cpp | `RoadNetworkTest.UT_VA01_RegistersConnectedIntersectionsAndExposesRoadProperties` | Pass |
| FR-10 | src/network/movement_engine.cpp | `MovementEngineTest.UT_VA05_EntersRoadAndCrossesOnlyOnPermittedGreen` | Pass |
| FR-11 | src/network/movement_engine.cpp | `MovementEngineTest.UT_VA13_FR11_NFR02_DoesNotEnterFullDownstreamLane` | Pass |
| FR-12 | src/network/road_network.cpp | `RoadNetworkTest.UT_VA02_ReturnsConnectedRouteWithMinimumTravelTime` | Pass |

FR-18 integrated reset clears lane occupancy, priority queues, incidents, counters and seeded generation; CSV replay is identical.
Module tests run independently, while integrated scenarios exercise the combined policies.
Current coverage, source fingerprints and per-case outcomes are in [reports](reports); the authoritative full mapping is [traceability.md](traceability.md).
Remote CI for this branch, Docker/Jenkins execution, human usability and owner/evaluator approvals are separate acceptance activities.
See [submission-checklist.md](submission-checklist.md) for pending actions.
