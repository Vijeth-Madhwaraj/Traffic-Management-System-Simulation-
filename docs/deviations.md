# Design deviations and final policies

11 November 2026. Review these changes with the team before final acceptance.

| Initial document proposal | Implemented contract and rationale |
|---|---|
| Pure abstract module interfaces | Concrete public C++ headers and value types; modules are tested directly and routes can be injected. No shared-library ABI is promised. |
| Simulation-owned std::mt19937 | VehicleManager owns std::mt19937_64; explicit integer thresholds make event streams repeatable. |
| Interactive load/run/status/incident/priority/export menu | Versioned text-file CLI with validate, preview, trace, lane status and CSV flags; incidents are scheduled and emergency requests use vehicle class or the in-process API. |
| Typed Result objects everywhere and numeric error codes | Setup/operational exceptions at the CLI boundary; bool/optional/RouteFailure for normal domain outcomes; exit codes 0, 1 and 2. |
| Immediate-looking emergency sequence | Serve a configured target phase only after full current green, yellow and all-red; FIFO/deduplicated requests preserve safety. No bounded normal-traffic delay is claimed. |
| Route selection independent of entry lane | Prescribe the first entry road, then minimize enabled suffix travel time with road-ID tie breaking. |
| Response shown conceptually in incident sequence | Preserve current road/index and replace only the suffix for incident/congestion triggers; safely wait when no enabled alternative exists. |
| Lane queues interpreted as storage everywhere | Source FIFO waiting capacity and downstream in-transit lane occupancy are separate; road occupancy is an aggregate limit. |
| Undefined waiting and throughput | Waiting includes deferred/queued tick-end waits and ready transit blocking. Average covers completed trips; throughput is completed per processed tick. Queue peak excludes deferred backlog, which has its own count. |
| Sanitizers implied in CI | Warnings, pinned formatting and Clang analysis run; sanitizer execution is not claimed. |

The SRS use-case view updates the original actor responsibilities; SAD component/two
sequence diagrams are retained as conceptual views; the updated contracts and traceability specify actual call names/order.
The implementation keeps C++17, CMake, deterministic execution and the agreed FIFO deferral.
