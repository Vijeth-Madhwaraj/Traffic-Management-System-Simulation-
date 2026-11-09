# Vijeth signal test coverage

This matrix follows the project plan's `UT-VI01` to `UT-VI10` identifiers. A
test status describes the automated test artifact; it is not a claim that the
feature itself is fully complete. `Pass` is used only after a test actually
runs successfully.

| Requirement | Test ID | Test behavior | Source | Status |
|---|---|---|---|---|
| FR-05 | UT-VI01 | Each configured intersection starts in its first configured Green phase and permits only that phase's movements. | `tests/unit/signals/signal_controller_test.cpp` | Pass (1 test) |
| FR-05, FR-06 | UT-VI04 | Conflicting movements cannot both be permitted; a phase containing a declared conflict is rejected. | `tests/unit/signals/signal_controller_test.cpp` | Pass (1 test) |
| FR-06 | UT-VI02 | Green remains active before its duration expires and changes to Yellow at the exact boundary. | `tests/unit/signals/signal_controller_test.cpp` | Pass (1 test) |
| FR-06 | UT-VI03 | Yellow and all-red clearance occur before the next phase becomes Green. | `tests/unit/signals/signal_controller_test.cpp` | Pass (1 test) |
| FR-06, NFR-04 | UT-VI08 | Invalid or unsafe plans and backwards ticks are rejected; failed reconfiguration preserves the last valid plan. | `tests/unit/signals/signal_controller_test.cpp` | Pass (4 tests) |
| FR-07 | TC-07 | Ready-to-cross queues extend green by a configured step, bounded by the maximum. | `TC_07_ExtendsGreenForLongQueuesUpToMaximum`; `TC_07_ReadyQueueExtendsTheNextGreenWithoutSkippingClearance` | Pass (2 tests) |
| FR-07 | TC-07 | Empty movement queues shorten green by a configured step, bounded by the minimum. | `TC_07_ShortensEmptyGreenToMinimumAndCanExtendAgain` | Pass |
| FR-08 | UT-VI07 | Emergency priority is accepted only when safe. | Emergency-priority request/API is not implemented. | Blocked |
| FR-06 | UT-VI09 | Reset restores the initial phase and timer. | `SignalController` has no reset API. | Blocked |
| FR-07, FR-06 | TC-07 | Adaptive timing preserves phase movement safety and yellow/all-red clearance. | `TC_07_AdaptiveTimingPreservesConflictAndClearanceSafety` | Pass |
| FR-07 | TC-07 | Invalid limits and base green durations outside configured bounds are rejected. | `TC_07_RejectsInvalidAdaptiveLimits`; `TC_07_RejectsInvalidOrDuplicateAdaptiveSignalPolicies` | Pass |

## Execution notes

- GoogleTest is enabled through `BUILD_TESTING` in the root CMake project and
  each test is registered with CTest.
- Local Windows MinGW Debug run: 12/12 signal tests passed after FR-07 implementation.
- The Windows CI workflow builds with the runner's Visual C++ toolchain and
  runs the same suite. The local PowerShell instructions use MinGW GCC.
- `UT-VI08` includes input-validation and timer-order robustness checks because
  the current signal API has no separate reset operation.
- FR-08 emergency priority remains blocked. Do not change a blocked row to `Pass`
  without executing a real test.
