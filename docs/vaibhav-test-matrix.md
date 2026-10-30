# Vaibhav routing and movement test cases

These cases use the project test IDs and distinguish implemented behavior from
planned work. Status is recorded only after the corresponding assertions run.

## Road network and routing

| Requirement | Test ID | Preconditions and action | Expected result | Test source | Status |
|---|---|---|---|---|---|
| FR-09 | UT-VA01 | Register intersections, add connected roads, retrieve their properties, and query outgoing roads. | Road and neighbor data are available through `RoadNetwork`. | `tests/unit/network/road_network_test.cpp` | Pass (1 test) |
| FR-09 | UT-VA11 | Reject empty/duplicate intersections, unknown endpoints, empty/duplicate road IDs, zero capacity/travel time, and occupancy above capacity. | Invalid roads are rejected without changing stored roads or adjacency. | `tests/unit/network/road_network_test.cpp` | Pass (1 test) |
| FR-12 | UT-VA02 | Configure two connected paths with different travel costs and request a route. | The lowest-cost route is returned in traversal order. | `tests/unit/network/road_network_test.cpp` (planned) | Blocked: `shortestRoute` is a stub |
| FR-12 | UT-VA03, UT-VA04 | Request routes to unreachable destinations and verify disabled roads are not selected. | No route is returned when no enabled path exists. | `tests/unit/network/road_network_test.cpp` (planned) | Blocked: `shortestRoute` is a stub |

## Vehicle movement

| Requirement | Test ID | Preconditions and action | Expected result | Test source | Status |
|---|---|---|---|---|---|
| FR-10 | UT-VA05 | Admit a vehicle onto its entry road, then present its connected next road under a matching Green movement. | It leaves the lane, follows its route, and crosses only while the matching movement is Green. | `tests/unit/network/movement_engine_test.cpp` | Pass (1 test) |
| FR-10 | UT-VA06 | Present the vehicle next movement during Yellow/Red or when it is absent from the active Green phase. | The vehicle remains on its current road and does not advance its route index. | `tests/unit/network/movement_engine_test.cpp` | Pass (1 test) |
| FR-10 | UT-VA07 | Set the current road travel time above one tick and advance before and at the travel-time boundary. | The vehicle stays in transit before the boundary and becomes eligible at the boundary. | `tests/unit/network/movement_engine_test.cpp` | Pass (1 test) |
| FR-11 | UT-VA08 | Fill a downstream road and attempt a signal-permitted crossing into it. | The vehicle waits; downstream occupancy never exceeds capacity. | `tests/unit/network/movement_engine_test.cpp` | Pass (1 test) |
| FR-10 | UT-VA09 | Move a vehicle over a multi-road route with each movement permitted. | Road occupancy transfers correctly and the vehicle becomes `Completed` at its destination. | `tests/unit/network/movement_engine_test.cpp` | Pass (1 test) |
| FR-10, NFR-02 | UT-VA10 | Invoke `MovementEngine` directly, including a duplicate call at the same tick, twice with identical inputs. | Both runs yield identical state and occupancy snapshots; one tick cannot move a vehicle twice. | `tests/unit/network/movement_engine_test.cpp` | Pass (1 test) |

## Execution notes

- Requirement ownership and module mapping follow current project traceability:
  FR-09/FR-12 map to `RoadNetwork`; FR-10/FR-11 map to `MovementEngine`.
- Movement tests should use IDs matching the configured signal phase exactly
  and advance signals before movement for each tick.
- FR-10 uses caller-provided or explicit planned routes; automatic route selection
  remains separate FR-12 work. NFR-02 is covered with direct, deterministic
  `MovementEngine` unit tests without constructing `Simulation`.
- Windows and Linux build/test jobs and Clang static analysis passed on PR #68.
- `shortestRoute` remains a TODO until FR-12 is implemented.
