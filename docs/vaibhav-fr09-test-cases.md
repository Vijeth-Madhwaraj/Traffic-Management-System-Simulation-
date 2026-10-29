# Vaibhav FR-09 test coverage

FR-09 requires the road network to represent connected intersections and roads
with valid, accessible capacity and travel-time data. The cases below exercise
the network module directly; they do not claim routing or vehicle movement.

| Requirement | Test ID | Test behavior | Source | Status |
|---|---|---|---|---|
| FR-09 | UT-VA01 | Register intersections, add a road between known endpoints, and retrieve its endpoints, capacity, travel time, occupancy, and enabled state. | `tests/unit/network/road_network_test.cpp` | Pending CI |
| FR-09 | UT-VA02 | Reject empty/duplicate intersections, unknown endpoints, empty or duplicate road IDs, zero capacity/travel time, and occupancy above capacity; preserve the existing road. | `tests/unit/network/road_network_test.cpp` | Pending CI |

## Execution notes

- Tests are registered as `traffic_network_tests` and discovered by CTest.
- Local build tools are unavailable in this Windows environment; the configured
  Windows and Linux GitHub Actions jobs will provide build and test results.
- FR-12 route selection and FR-10/FR-11 movement are outside this FR-09 change.
- Status reflects only the focused road-network assertions, not full system acceptance.
