# Vijeth signal requirement evidence

Updated 11 November 2026. Module names identify team responsibility; Git history records actual commit authors.
All 115 Linux CTest cases pass on the final implementation branch. Earlier execution notes remain in Git history.

| Requirement | Source | Executed evidence | Result |
|---|---|---|---|
| FR-05 | src/signals/signal_controller.cpp | `SignalControllerTest.UT_VI01_StartsEachIntersectionInItsConfiguredInitialPhase` | Pass |
| FR-06 | src/signals/signal_controller.cpp | `SignalControllerTest.UT_VI03_UsesYellowAndAllRedClearanceBeforeNextGreen` | Pass |
| FR-07 | src/signals/signal_controller.cpp | `SignalControllerTest.TC_07_ExtendsGreenForLongQueuesUpToMaximum` | Pass |
| FR-08 | src/signals/signal_controller.cpp; src/network/movement_engine.cpp | `SignalControllerTest.TC_08_PrioritySkipsToRequestedPhaseAfterFullClearance` | Pass |

FR-18 integrated reset clears lane occupancy, priority queues, incidents, counters and seeded generation; CSV replay is identical.
Module tests run independently, while integrated scenarios exercise the combined policies.
Current coverage, source fingerprints and per-case outcomes are in [reports](reports); the authoritative full mapping is [traceability.md](traceability.md).
Remote CI for this branch, Docker/Jenkins execution, human usability and owner/evaluator approvals are separate acceptance activities.
See [submission-checklist.md](submission-checklist.md) for pending actions.
