# Aarya: vehicle/input implementation and evidence

Validation date: **23 October 2026**. Branch: `feature/aarya-vehicle-input`.
Environment: Ubuntu 26.04.1 x86_64, GCC 15.2.0, CMake 4.2.3, C++17 Debug,
GoogleTest v1.15.2. Clang 21.1.8 is used for static analysis.

## Requirement status

| Requirement | Implementation / evidence | Status |
|---|---|---|
| FR-01 | `ScenarioLoader`, text format, CLI, atomic `Simulation::configureScenario`; scenario and CLI tests | Pass for input capability |
| FR-02 | Scheduled arrivals and bounded probabilistic streams; UT-A01, UT-A02, UT-A08 | Pass |
| FR-03 | Unique IDs, known endpoints/class, connected explicit routes; replaceable route provider; UT-A01, UT-A03, UT-A09 | Pass for module; production automatic routing blocked |
| FR-04 | FIFO lane operations, admission limits, stable pending IDs and retries; UT-A04–UT-A06 | Pass |
| FR-18 | Seed recorded, actual arrival-event sequences repeat after reset; TC-18 | Pass for input subsystem; complete simulation results blocked |

## Planned unit-test IDs

Tests live in `tests/unit/vehicle/vehicle_manager_test.cpp` and
`tests/unit/vehicle/lane_queue_test.cpp`. IDs may be grouped in a test name where the
same scenario verifies several related behaviors; each row below has real assertions.

| ID | Behavior verified | Result |
|---|---|---|
| UT-A01 | Unique IDs, preserved class and route attributes | Pass |
| UT-A02 | Seeded stream produces the same arrival ticks after reset | Pass |
| UT-A03 | Unknown/equal endpoints rejected without consuming an ID | Pass |
| UT-A04 | Vehicle enters a lane with free capacity | Pass |
| UT-A05 | Full queue defers arrival; retry preserves ID | Pass |
| UT-A06 | FIFO order, including older pending vehicles before new arrivals | Pass |
| UT-A07 | Zero capacities and negative/overflow/malformed scenario numbers rejected | Pass |
| UT-A08 | Scheduled tick boundaries; repeated calls do not duplicate arrivals | Pass |
| UT-A09 | Correct source/destination sent to route provider; empty route is Unroutable | Pass with injected provider |
| UT-A10 | Reset restores IDs, RNG, schedules, queues and pending state | Pass |

Additional tests cover malformed records, unknown references, duplicate IDs, unsafe signal
plans, input limits, read failures, atomic scenario replacement, provider exceptions,
stream bounds/order, missing files, and CLI argument errors. Scenario tests are in
`tests/unit/input/scenario_test.cpp`; integration tests are in
`tests/integration/scenario_input_test.cpp`.

## Actual execution

```sh
cmake -S . -B /tmp/se-jackfruit-aarya-build -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/se-jackfruit-aarya-build --parallel 4
ctest --test-dir /tmp/se-jackfruit-aarya-build --output-on-failure
```

**38/38 CTest cases passed:** 8 existing signal tests, 25 vehicle/scenario/integration
GoogleTests, and 5 CLI tests. This is not 38 completed requirements.

- Scheduled CLI: seed 42; generated 3, admitted 1, queued 1, deferred 2, unroutable 0.
- Seeded CLI: seed 496; generated 53, admitted 5, queued 5, deferred 48, unroutable 0.
- `config/performance_scenario.txt`: 4 intersections, 500 generated/admitted vehicles;
  500-tick input-only Debug run measured **0.090 s** with Python `perf_counter`, including
  process startup. Vehicles remain queued: this does **not** establish full-system NFR-03.

Static analysis executed successfully with:

```sh
cmake -S . -B /tmp/se-jackfruit-aarya-clang-analysis -DBUILD_TESTING=OFF -DTRAFFIC_ENABLE_ANALYZER=ON
cmake --build /tmp/se-jackfruit-aarya-clang-analysis --parallel 2
```

All **10 project translation units** produced zero Clang analyzer findings, and the
application built with `-Wall -Wextra -Wpedantic -Werror`. A preliminary GCC `-fanalyzer`
attempt reported allocator/empty-allocator paths in libstdc++ and was not accepted as a
passing check. The committed check uses Clang, without broad diagnostic suppressions.

Linux test and Clang analyzer jobs are configured in GitHub Actions; no remote CI result
is claimed because this branch has not been pushed. Windows execution of these new tests
has not been performed locally.

## NFR evidence and limits

| NFR | Evidence / remaining requirement |
|---|---|
| 01 Maintainability | Module-owned state, public input/admission APIs, no mutable globals; inspect architecture contract |
| 02 Testability | Independent generation/queue tests and injectable routing; real simulation only for integration fixtures |
| 03 Performance | Input-only 500-vehicle workload measured above; full movement/metrics benchmark blocked |
| 04 Reliability | Invalid input and scenario replacement leave existing state intact; controlled errors at CLI |
| 05 Portability | Linux GCC C++17 build/tests passed; Windows CI configured but unverified for this branch |
| 06 Usability | CLI help, seed, counters, and readable errors exercised; unfamiliar-user review remains pending |
| 07 Robustness | Bounded file/line/count/range checks, unsigned parsing rejects negatives, RAII, read-failure tests |
| 08 Traceability | This matrix and `traceability.md` link requirements to actual source/tests |
| 09 Code quality | Warning-enabled build and Clang static analysis passed on all 10 project translation units, zero findings |
| 10 Reproducibility | Seed printed; repeated actual seeded event sequences tested; whole-system comparison pending |

## Remaining work with teammates

1. Vaibhav: implement routing and movement, agree exact signal movement IDs, and consume
   entry queues through the documented APIs. Automatic routing must return a route that
   starts on the configured entry road.
2. Vijeth: integrate queue demand and safe emergency requests; emergency class currently
   preserves FIFO and does not bypass phase safety.
3. Vainik: integrate congestion, incidents, waiting/travel updates and completed-trip metrics.
4. Aarya with teammates: complete IT-01, IT-04, standard system scenarios and NFR-03;
   compare full reports for FR-18, run usability review, and obtain teammate PR review.
5. Link actual tracker issues and PRs when created. Project-plan SE-05/06/07 labels are
   suggested backlog entries, not verified live issue keys.

No full-trip, adaptive-signal, incident-response, completed-report, coverage percentage,
or final acceptance claim is made by this branch.


## 30 October 2026: FR-01 scenario preview

`traffic_sim --validate <file>` validates and previews scenario configuration without
starting the simulation. `cli_validate_only` checks the preview counts, absence of run
output, and rejection of mixed command modes. `cli_validate_invalid` and
`cli_validate_missing` verify nonzero results for invalid data and unreadable files.
Linux GCC 15.2 Debug build: **43/43 CTest cases passed**, including the existing signal
and road-network tests. This extends FR-01; it does not add routing or movement functionality.


## 30 October 2026: FR-04 queue safety and per-lane inspection

Queue insertion now rejects ID zero and duplicate IDs already in the same queue without
consuming capacity. IDs can re-enter after removal. `LaneQueue::snapshot()` and
`VehicleManager::laneSnapshots()` expose independent FIFO snapshots, including deferred IDs
and lane capacities; lanes are sorted by ID. `--scenario <file> --lane-status` displays this
information on request. These snapshots do not establish global vehicle membership across
lanes; placement remains owned by `VehicleManager`.

Added tests:

- `LaneQueueTest.FR04_RejectsZeroAndDuplicateIdsWithoutUsingCapacity`
- `LaneQueueTest.FR04_SnapshotPreservesOrderWithoutExposingMutableStorage`
- `VehicleInputTest.FR04_PerLaneSnapshotsTrackIndependentFifoRetriesAndReset`
- `cli_lane_status` (also checks concise default output and rejection with validation mode)

Linux GCC 15.2 C++17 Debug: **47/47 CTest cases passed**, including all current signal
and road-network tests. The new work extends FR-01 and FR-04 already implemented in the
previous PR; it does not claim two previously unimplemented requirements as newly complete.

The 30 October warning-as-error build and Clang static analysis also passed: all 10
project translation units produced zero findings. Remote CI for today's new branch has
not run because these commits have not been pushed.


## 31 October 2026: PR #69 integration

Merged current master into the movement branch while preserving scenario preview, FIFO
safeguards, per-lane snapshots, congestion detection and incident tests. Queue/deferral
counts from the 23/30 October runs above describe the earlier movement stub. With FR-10
explicit-route movement, the standard demo now generates and admits 3 vehicles and ends with
zero queued/deferred vehicles.

`cli_lane_status` checks that drained result and a separate slow-road fixture that ends
with vehicle 2 queued and vehicle 3 deferred. Linux GCC 15.2 C++17 Debug execution passed
**63/63 CTest cases**, including signal, vehicle, scenario, management, network, movement
and CLI coverage. The required `Basic CI Check` remains a unique aggregate job that
requires Windows tests, Linux tests and static analysis to succeed.

Clang static analysis passed for all 11 project translation units with zero findings;
the application also built with compiler warnings treated as errors.


## 31 October 2026: FR-02 configurable batch arrivals

One functional improvement for FR-02 on `feature/aarya-vehicle-generation`: an optional
`batch <stream-id> <count>` setting allows 1–1000 vehicles per successful probability
decision, instead of requiring repeated streams to model higher traffic volumes. Existing
scenarios default to one vehicle. Batches preserve deterministic RNG draws, unique IDs,
stream ordering, capacity enforcement and FIFO deferral. The scenario's 100000-vehicle
budget includes batch sizes and is checked before multiplication to avoid overflow.

New tests:

- `VehicleInputTest.FR02_BatchArrivalsHaveUniqueIdsAndRespectTickOrdering`
- `VehicleInputTest.FR02_BatchArrivalsDeferThroughExistingCapacityAndFifoRules`
- `VehicleInputTest.FR02_RejectsInvalidBatchSizeWithoutReplacingValidStreams`
- `VehicleInputTest.FR02_IndependentSeededBatchRunsAreReproducible`
- `VehicleInputTest.FR02_BatchSizeDoesNotChangeOtherStreamsRandomDecisions`
- `ScenarioTest.FR02_LoadsBatchSettingsWithForwardReferencesAndDefaultCompatibility`
- `ScenarioTest.FR02_RejectsInvalidDuplicateAndUnknownBatchSettings`
- `ScenarioTest.FR02_CountsBatchVehiclesAgainstScenarioLimitBeforeMultiplying`
- `cli_batch_arrivals`

Linux GCC 15.2 C++17 Debug: **72/72 CTest cases passed** (63 existing and 9 new). The batch
CLI fixture generates 12 vehicles, admits 4, and retains deferred IDs 5–12 in order. This
is an FR-02 capability extension; FR-03 work is left for a later change.

For this FR-02 change, the warning-as-error build and Clang static analysis passed for
all 11 project translation units with zero findings. Remote CI remains unverified until
the new feature branch is pushed.
