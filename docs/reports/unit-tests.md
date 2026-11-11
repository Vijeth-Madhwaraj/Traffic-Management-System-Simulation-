# Unit and automated regression report

11 November 2026. Linux GCC 15.2, C++17 Debug, GoogleTest v1.15.2.

**115/115 CTest cases passed; zero failures.**

This includes unit, integration and CLI regressions. It does not claim every human acceptance activity has passed.

| Suite | Cases | Result |
|---|---:|---|
| CongestionDetectorTest.UT_VN01_DetectsRoadAtConfiguredOccupancyThreshold | 1 | PASS |
| CongestionDetectorTest.UT_VN02_DetectsQueueAndHonorsAbsoluteThreshold | 1 | PASS |
| CongestionDetectorTest.UT_VN03_RejectsInvalidThresholdUpdate | 1 | PASS |
| CongestionDetectorTest.UT_VN11_RejectsInvalidConstructorThresholds | 1 | PASS |
| IncidentManagerTest.UT_VN04_BlocksAndRestoresRoadAtHalfOpenInterval | 1 | PASS |
| IncidentManagerTest.UT_VN05_PreservesInitiallyDisabledRoad | 1 | PASS |
| IncidentManagerTest.UT_VN06_RejectsInvalidInterval | 1 | PASS |
| IncidentManagerTest.UT_VN12_OverlappingIncidentsKeepRoadBlockedUntilBothEnd | 1 | PASS |
| IncidentManagerTest.UT_VN13_RejectsActiveUnknownRoadWithoutPartialChanges | 1 | PASS |
| IncidentManagerTest.UT_VN14_ClearRestoresRoadsBlockedByActiveIncidents | 1 | PASS |
| LaneQueueTest.FR04_RejectsZeroAndDuplicateIdsWithoutUsingCapacity | 1 | PASS |
| LaneQueueTest.FR04_SnapshotPreservesOrderWithoutExposingMutableStorage | 1 | PASS |
| LaneQueueTest.UT_A04_A05_A06_EnforcesCapacityAndFifo | 1 | PASS |
| LaneQueueTest.UT_A07_A10_RejectsZeroCapacityAndClearsState | 1 | PASS |
| MetricsCollectorTest.AverageWaitingTimeIsZeroWithoutCompletions | 1 | PASS |
| MetricsCollectorTest.TracksCompletedVehiclesAndWaitingTime | 1 | PASS |
| MetricsCollectorTest.TracksGeneratedVehicles | 1 | PASS |
| MetricsCollectorTest.TracksMaximumQueueLength | 1 | PASS |
| MetricsCollectorTest.WritesCsvOutput | 1 | PASS |
| MovementEngineTest.TC_07_CountsOnlyVehiclesReadyToCrossByMovement | 1 | PASS |
| MovementEngineTest.TC_07_ReadyQueueExtendsTheNextGreenWithoutSkippingClearance | 1 | PASS |
| MovementEngineTest.UT_VA05_EntersRoadAndCrossesOnlyOnPermittedGreen | 1 | PASS |
| MovementEngineTest.UT_VA06_YellowOrUnlistedMovementKeepsVehicleOnItsRouteRoad | 1 | PASS |
| MovementEngineTest.UT_VA07_RespectsConfiguredRoadTravelTime | 1 | PASS |
| MovementEngineTest.UT_VA08_DoesNotCrossWhenDownstreamRoadIsFull | 1 | PASS |
| MovementEngineTest.UT_VA09_CompletesVehicleAfterTraversingItsPlannedRoute | 1 | PASS |
| MovementEngineTest.UT_VA10_NFR02_IsDeterministicAndCallableWithoutSimulation | 1 | PASS |
| MovementEngineTest.UT_VA13_FR11_NFR02_DoesNotEnterFullDownstreamLane | 1 | PASS |
| ReportWriterTest.WritesCsvHeader | 1 | PASS |
| ReportWriterTest.WritesPerformanceAndIncidentMetrics | 1 | PASS |
| ReportWriterTest.WritesSimulationCounts | 1 | PASS |
| RoadNetworkTest.UT_VA01_RegistersConnectedIntersectionsAndExposesRoadProperties | 1 | PASS |
| RoadNetworkTest.UT_VA02_ReturnsConnectedRouteWithMinimumTravelTime | 1 | PASS |
| RoadNetworkTest.UT_VA03_ReturnsEmptyForUnknownOrUnreachableDestinations | 1 | PASS |
| RoadNetworkTest.UT_VA04_ExcludesDisabledRoadsAndKeepsDeterministicTies | 1 | PASS |
| RoadNetworkTest.UT_VA11_RejectsInvalidOrUnconnectedRoadsWithoutChangingNetwork | 1 | PASS |
| ScenarioInputIntegrationTest.EmergencyVehicleRequestsSafePhaseWithoutReorderingEntryQueue | 1 | PASS |
| ScenarioInputIntegrationTest.FR03_RunContinuesAfterInvalidRouteAndMovesOtherVehicles | 1 | PASS |
| ScenarioInputIntegrationTest.IncidentAwareRoutingUsesAlternativeRoad | 1 | PASS |
| ScenarioInputIntegrationTest.InvalidReplacementPreservesRunningSimulation | 1 | PASS |
| ScenarioInputIntegrationTest.LaneCapacityPreservesSignalsIncidentsReportsAndReset | 1 | PASS |
| ScenarioInputIntegrationTest.ParsesAndValidatesScheduledIncidents | 1 | PASS |
| ScenarioInputIntegrationTest.RejectsIncidentWithUnknownRoad | 1 | PASS |
| ScenarioInputIntegrationTest.RequiresValidatedScenarioBeforeRunning | 1 | PASS |
| ScenarioInputIntegrationTest.RoutesThroughPrescribedEntryDespiteCheaperOtherRoad | 1 | PASS |
| ScenarioInputIntegrationTest.TC_01_TC_04_LoadsGeneratesAndMovesExplicitRoutes | 1 | PASS |
| ScenarioInputIntegrationTest.TC_15_CongestionTriggersAlternativeWithoutIncident | 1 | PASS |
| ScenarioInputIntegrationTest.TC_15_NoAlternativeWaitsSafelyUntilRoadRestores | 1 | PASS |
| ScenarioInputIntegrationTest.TC_15_ReroutesExistingVehicleAfterLateIncidentAndReplays | 1 | PASS |
| ScenarioInputIntegrationTest.TC_16_CountsDeferredWaitingAndPeakQueueBeforeMovement | 1 | PASS |
| ScenarioInputIntegrationTest.TC_17_FinalCongestionMatchesFinalStateAndStoresSeed | 1 | PASS |
| ScenarioInputIntegrationTest.TC_18_ResetRepeatsActualSeededArrivalEvents | 1 | PASS |
| ScenarioInputIntegrationTest.UsesInjectedRoutesAndProductionRouting | 1 | PASS |
| ScenarioTest.BoundsInputAndPotentialVehicleCounts | 1 | PASS |
| ScenarioTest.FR02_CountsBatchVehiclesAgainstScenarioLimitBeforeMultiplying | 1 | PASS |
| ScenarioTest.FR02_LoadsBatchSettingsWithForwardReferencesAndDefaultCompatibility | 1 | PASS |
| ScenarioTest.FR02_RejectsInvalidDuplicateAndUnknownBatchSettings | 1 | PASS |
| ScenarioTest.FailedLoadLeavesPreviouslyLoadedValueUnchanged | 1 | PASS |
| ScenarioTest.RejectsBadRecordsUnknownReferencesAndDuplicates | 1 | PASS |
| ScenarioTest.RejectsNonexistentOrWrongIntersectionMovements | 1 | PASS |
| ScenarioTest.TC_01_LoadsForwardReferencesClassesRoutesAndSignals | 1 | PASS |
| ScenarioTest.TC_07_LoadsAdaptiveSignalPolicy | 1 | PASS |
| ScenarioTest.TC_07_RejectsInvalidOrDuplicateAdaptiveSignalPolicies | 1 | PASS |
| ScenarioTest.UT_A07_RejectsNegativeOverflowZeroAndMalformedNumbers | 1 | PASS |
| SignalControllerTest.TC_07_AdaptiveTimingPreservesConflictAndClearanceSafety | 1 | PASS |
| SignalControllerTest.TC_07_ExtendsGreenForLongQueuesUpToMaximum | 1 | PASS |
| SignalControllerTest.TC_07_RejectsInvalidAdaptiveLimits | 1 | PASS |
| SignalControllerTest.TC_07_ShortensEmptyGreenToMinimumAndCanExtendAgain | 1 | PASS |
| SignalControllerTest.TC_08_PrioritySkipsToRequestedPhaseAfterFullClearance | 1 | PASS |
| SignalControllerTest.TC_08_RequestsDuringClearanceRemainSafeAndFifo | 1 | PASS |
| SignalControllerTest.UT_VI01_StartsEachIntersectionInItsConfiguredInitialPhase | 1 | PASS |
| SignalControllerTest.UT_VI02_ChangesFromGreenAtTheConfiguredBoundary | 1 | PASS |
| SignalControllerTest.UT_VI03_UsesYellowAndAllRedClearanceBeforeNextGreen | 1 | PASS |
| SignalControllerTest.UT_VI04_NeverPermitsConflictingMovementsAtTheSameTime | 1 | PASS |
| SignalControllerTest.UT_VI08_AdvancesAcrossSkippedTicksDeterministically | 1 | PASS |
| SignalControllerTest.UT_VI08_InvalidReconfigurationLeavesTheValidPlanIntact | 1 | PASS |
| SignalControllerTest.UT_VI08_RejectsBackwardTicksWithoutChangingSignalState | 1 | PASS |
| SignalControllerTest.UT_VI08_RejectsEmptyOrInvalidSignalPlans | 1 | PASS |
| VehicleInputTest.FR02_BatchArrivalsDeferThroughExistingCapacityAndFifoRules | 1 | PASS |
| VehicleInputTest.FR02_BatchArrivalsHaveUniqueIdsAndRespectTickOrdering | 1 | PASS |
| VehicleInputTest.FR02_BatchSizeDoesNotChangeOtherStreamsRandomDecisions | 1 | PASS |
| VehicleInputTest.FR02_IndependentSeededBatchRunsAreReproducible | 1 | PASS |
| VehicleInputTest.FR02_RejectsInvalidBatchSizeWithoutReplacingValidStreams | 1 | PASS |
| VehicleInputTest.FR03_DistinguishesUnavailableRouteAndResetsFailureState | 1 | PASS |
| VehicleInputTest.FR03_InvalidProviderRouteDoesNotBlockValidVehiclesAtSameTick | 1 | PASS |
| VehicleInputTest.FR03_NewVehicleCannotCarryAnOldRoutingFailure | 1 | PASS |
| VehicleInputTest.FR03_ProviderExceptionsRollBackEvenAfterARejectedVehicle | 1 | PASS |
| VehicleInputTest.FR03_RejectsDisconnectedIncompleteDisabledAndWrongEntryProviderRoutes | 1 | PASS |
| VehicleInputTest.FR04_PerLaneSnapshotsTrackIndependentFifoRetriesAndReset | 1 | PASS |
| VehicleInputTest.FailedRouteProviderDoesNotConsumeArrivals | 1 | PASS |
| VehicleInputTest.InvalidTopologyReplacementPreservesOriginal | 1 | PASS |
| VehicleInputTest.RejectsInvalidClassStateAndDisconnectedRoutes | 1 | PASS |
| VehicleInputTest.RejectsInvalidScheduleWithoutReplacingValidOne | 1 | PASS |
| VehicleInputTest.RejectsLaneMismatchAndUnknownLanesBeforeGeneration | 1 | PASS |
| VehicleInputTest.StreamOrderIsStableRegardlessOfConfigurationOrder | 1 | PASS |
| VehicleInputTest.UT_A01_AssignsUniqueIdsAndPreservesAttributes | 1 | PASS |
| VehicleInputTest.UT_A02_A10_SeededStreamsRepeatAfterReset | 1 | PASS |
| VehicleInputTest.UT_A03_RejectsUnknownOrEqualEndpointsWithoutAllocatingId | 1 | PASS |
| VehicleInputTest.UT_A04_A05_A06_RetriesPendingBeforeNewArrivalsWithoutChangingIds | 1 | PASS |
| VehicleInputTest.UT_A08_GeneratesExactlyAtScheduledTickAndOnlyOnce | 1 | PASS |
| VehicleInputTest.UT_A09_RequestsCorrectRouteAndHandlesUnavailableRoutes | 1 | PASS |
| VehicleInputTest.ValidatesProbabilityAndHonorsStreamBounds | 1 | PASS |
| cli_adaptive_signal_demo | 1 | PASS |
| cli_bad_arguments | 1 | PASS |
| cli_batch_arrivals | 1 | PASS |
| cli_help | 1 | PASS |
| cli_lane_status | 1 | PASS |
| cli_missing_file | 1 | PASS |
| cli_report_open_failure | 1 | PASS |
| cli_report_write_failure | 1 | PASS |
| cli_scheduled_demo | 1 | PASS |
| cli_seeded_demo | 1 | PASS |
| cli_validate_invalid | 1 | PASS |
| cli_validate_missing | 1 | PASS |
| cli_validate_only | 1 | PASS |

Raw executed cases: [CTest JUnit](coverage/tests.xml). Source/test mapping: [RTM](../traceability.md).
New regressions cover emergency clearance, late incident and congestion rerouting, no-alternative restoration,
entry-aware routing, signal references, deferred waiting, throughput, final congestion and report write failures.
The separate GCC coverage build passed the same suite; clang-format and all 12 static-analysis units passed.
No Docker/Jenkins execution or human usability approval is inferred from these results.
