# Vaibhav routing and movement test cases

These cases use the `UT-VA01` to `UT-VA10` identifiers already reserved in
[`traceability.md`](traceability.md). They are planned cases, not evidence that
routing or movement is implemented. Status stays `Blocked` until the behavior
and assertions exist and have actually run.

## Road network and routing

| Requirement | Test ID | Preconditions and action | Expected result | Test source | Status |
|---|---|---|---|---|---|
| FR-09 | UT-VA01 | Add a road with non-empty ID/endpoints and positive capacity/travel time; look it up by ID. | Addition succeeds and lookup returns the configured road and attributes. | `tests/unit/network/road_network_test.cpp` (planned) | Blocked: tests not added |
| FR-09 | UT-VA02 | Try roads with an empty ID/endpoint, zero capacity/time, and a duplicate ID. | Invalid and duplicate roads are rejected; the existing road remains unchanged. | `tests/unit/network/road_network_test.cpp` (planned) | Blocked: tests not added |
| FR-12 | UT-VA03 | Configure two directed paths between the same endpoints with different total travel times; request a route. | The connected route with the lowest total travel time is returned in traversal order. | `tests/unit/network/road_network_test.cpp` (planned) | Blocked: `shortestRoute` is a stub |
| FR-12 | UT-VA04 | Request a route to an unreachable destination; repeat with the only connecting road disabled. | An empty route is returned in both cases; disabled roads are not used. | `tests/unit/network/road_network_test.cpp` (planned) | Blocked: `shortestRoute` is a stub |

## Vehicle movement

| Requirement | Test ID | Preconditions and action | Expected result | Test source | Status |
|---|---|---|---|---|---|
| FR-10 | UT-VA05 | Put a vehicle at the front of an entry lane with a connected route, configure its movement as Green, and advance movement. | The vehicle leaves the entry queue and begins the first road only when its movement is permitted. | `tests/unit/network/movement_engine_test.cpp` (planned) | Blocked: movement is a stub |
| FR-10, FR-11 | UT-VA06 | Repeat the eligible vehicle setup while its movement is Yellow, Red, or absent from the signal plan. | It does not cross; queue position and road occupancy remain unchanged. | `tests/unit/network/movement_engine_test.cpp` (planned) | Blocked: movement is a stub |
| FR-10, FR-12 | UT-VA07 | Put a vehicle on a road with travel time greater than one tick; advance before and at the travel-time boundary. | It remains in transit before the boundary and reaches the next intersection only after the configured travel time. | `tests/unit/network/movement_engine_test.cpp` (planned) | Blocked: movement is a stub |
| FR-11 | UT-VA08 | Fill a downstream road to capacity and attempt a signal-permitted crossing into it. | The crossing is deferred; occupancy never exceeds capacity and the vehicle remains available for a later retry. | `tests/unit/network/movement_engine_test.cpp` (planned) | Blocked: movement is a stub |
| FR-10, FR-12 | UT-VA09 | Move a vehicle through a multi-road route with every crossing permitted and downstream capacity available. | Route position advances in order, road occupancy is updated, and the vehicle becomes `Completed` at its destination. | `tests/unit/network/movement_engine_test.cpp` (planned) | Blocked: movement is a stub |
| FR-10, FR-11 | UT-VA10 | Run the same fixed scenario tick-by-tick twice from reset with identical phases and capacities. | Both runs produce identical per-tick vehicle states, route positions, and occupancies; no vehicle moves twice in one tick. | `tests/integration/network_movement_test.cpp` (planned) | Blocked: integration is not implemented |

## Execution notes

- Requirement ownership and module mapping follow current project traceability:
  FR-09/FR-12 map to `RoadNetwork`; FR-10/FR-11 map to `MovementEngine`.
- Movement tests should use IDs matching the configured signal phase exactly
  and advance signals before movement for each tick.
- Do not mark a case `Pass` until its assertions run successfully. The current
  `shortestRoute` and `MovementEngine::advance` implementations are TODO stubs.
