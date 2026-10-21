# Requirement traceability starter

The source requirements are FR-01 through FR-18 and NFR-01 through NFR-10 in the project SRS. This table is a planning aid; a source file or test ID does not imply the requirement is implemented.

| Requirement | Planned module/file | Planned test IDs |
|---|---|---|
| FR-01, FR-18 | scenario input, `SimulationConfig` | TC-01, TC-18 |
| FR-02 to FR-04 | `VehicleManager`, `LaneQueue` | UT-A01 to UT-A10 |
| FR-05 to FR-08 | `SignalController` | UT-VI01 to UT-VI10 |
| FR-09, FR-12 | `RoadNetwork` | UT-VA01 to UT-VA04 |
| FR-10, FR-11 | `MovementEngine` | UT-VA05 to UT-VA10 |
| FR-13 to FR-15 | `IncidentManager` and congestion policy | UT-VN01 to UT-VN06 |
| FR-16, FR-17 | `MetricsCollector` and report output | UT-VN07 to UT-VN10 |

Add exact source references, integration/system test IDs, and actual results as work lands. Keep NFR coverage visible in the test and validation plans.
