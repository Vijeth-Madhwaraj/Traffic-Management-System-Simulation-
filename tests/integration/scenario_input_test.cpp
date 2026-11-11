
#include "traffic/input/scenario.hpp"
#include "traffic/simulation/simulation.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <vector>

namespace traffic {
namespace {

Scenario fixture() {
    std::istringstream file(
        "scenario 1\n"
        "seed 42\n"
        "duration 6\n"
        "intersection A\n"
        "intersection B\n"
        "intersection C\n"
        "road AB A B 2 1\n"
        "road BC B C 2 1\n"
        "lane entry AB 1\n"
        "phase B 2 1 1 AB-to-BC\n"
        "arrival 0 entry C standard AB BC\n"
        "arrival 0 entry C emergency AB BC\n"
        "arrival 1 entry C standard AB BC\n");

    return ScenarioLoader::load(file);
}

TEST(
    ScenarioInputIntegrationTest,
    TC_01_TC_04_LoadsGeneratesAndMovesExplicitRoutes
) {
    Simulation simulation({});
    simulation.configureScenario(fixture());

    EXPECT_EQ(
        simulation.step().generated,
        (std::vector<VehicleId>{1, 2}));

    simulation.run();

    const auto summary = simulation.inputSummary();

    EXPECT_EQ(summary.generated, 3U);
    EXPECT_EQ(summary.admitted, 3U);
    EXPECT_EQ(summary.queued, 0U);
    EXPECT_EQ(summary.pending, 0U);
    EXPECT_EQ(summary.unroutable, 0U);
    EXPECT_EQ(simulation.currentTick(), 6U);

    for (VehicleId id = 1; id <= 3; ++id) {
        ASSERT_NE(simulation.vehicles().find(id), nullptr);
        EXPECT_EQ(
            simulation.vehicles().find(id)->state,
            VehicleState::Completed);
    }

    simulation.run();

    EXPECT_EQ(simulation.inputSummary().generated, 3U);
}

TEST(
    ScenarioInputIntegrationTest,
    InvalidReplacementPreservesRunningSimulation
) {
    Simulation simulation({});
    simulation.configureScenario(fixture());

    simulation.step();

    auto invalid = fixture();
    invalid.lanes[0].road = "unknown";

    EXPECT_THROW(
        simulation.configureScenario(invalid),
        std::invalid_argument);

    EXPECT_EQ(simulation.currentTick(), 1U);
    EXPECT_EQ(simulation.inputSummary().generated, 2U);

    simulation.run();

    EXPECT_EQ(simulation.inputSummary().generated, 3U);
}

TEST(
    ScenarioInputIntegrationTest,
    TC_18_ResetRepeatsActualSeededArrivalEvents
) {
    auto scenario = fixture();

    scenario.config.duration = 100;
    scenario.arrivals.clear();

    auto vehicle = fixture().arrivals[0].vehicle;

    scenario.streams = {
        {"traffic", "entry", 0, 100, 0.5, vehicle}};

    Simulation simulation({});
    simulation.configureScenario(scenario);

    const auto run = [&]() {
        std::vector<Tick> ticks;

        while (
            simulation.currentTick() <
            scenario.config.duration) {

            const auto tick = simulation.currentTick();

            for (const auto id : simulation.step().generated) {
                ticks.push_back(tick);
                EXPECT_EQ(id, ticks.size());
            }
        }

        return ticks;
    };

    const auto first = run();

    EXPECT_FALSE(first.empty());

    simulation.reset();

    EXPECT_EQ(simulation.currentTick(), 0U);
    EXPECT_EQ(simulation.inputSummary().generated, 0U);

    EXPECT_EQ(run(), first);
}

TEST(
    ScenarioInputIntegrationTest,
    UsesInjectedRoutesAndProductionRouting
) {
    auto scenario = fixture();

    scenario.arrivals.resize(1);
    scenario.arrivals[0].vehicle.route.clear();

    Simulation simulation({});
    simulation.configureScenario(scenario);

    // FR-15: Production routing should find a valid route automatically.
    simulation.run();

    EXPECT_EQ(simulation.inputSummary().unroutable, 0U);
    EXPECT_EQ(simulation.inputSummary().admitted, 1U);

    ASSERT_NE(simulation.vehicles().find(1), nullptr);
    EXPECT_EQ(simulation.vehicles().find(1)->route,
              (std::vector<RoadId>{"AB", "BC"}));
    EXPECT_EQ(simulation.vehicles().find(1)->state, VehicleState::Completed);

    simulation.reset();

    // A custom route provider can also supply the route.
    simulation.setRouteProvider(
        [](const auto& source, const auto& destination) {
            EXPECT_EQ(source, "A");
            EXPECT_EQ(destination, "C");

            return std::vector<RoadId>{"AB", "BC"};
        });

    simulation.run();

    EXPECT_EQ(simulation.inputSummary().unroutable, 0U);
    EXPECT_EQ(simulation.inputSummary().admitted, 1U);

    EXPECT_THROW(
        simulation.setRouteProvider({}),
        std::logic_error);
}

TEST(
    ScenarioInputIntegrationTest,
    RequiresValidatedScenarioBeforeRunning
) {
    Simulation simulation({});

    EXPECT_THROW(simulation.step(), std::logic_error);
    EXPECT_THROW(simulation.run(), std::logic_error);
    EXPECT_THROW(simulation.reset(), std::logic_error);
}

// Master integration test: invalid routing for one vehicle
// must not prevent other vehicles from completing their journeys.
TEST(
    ScenarioInputIntegrationTest,
    FR03_RunContinuesAfterInvalidRouteAndMovesOtherVehicles
) {
    auto scenario = fixture();

    scenario.arrivals[0].vehicle.route.clear();

    Simulation simulation({});
    simulation.configureScenario(scenario);

    simulation.setRouteProvider(
        [](const auto&, const auto&) {
            return std::vector<RoadId>{"unknown"};
        });

    EXPECT_NO_THROW(simulation.run());

    EXPECT_EQ(simulation.inputSummary().generated, 3U);
    EXPECT_EQ(simulation.inputSummary().unroutable, 1U);
    EXPECT_EQ(simulation.inputSummary().admitted, 2U);

    ASSERT_NE(simulation.vehicles().find(1), nullptr);
    ASSERT_NE(simulation.vehicles().find(2), nullptr);
    ASSERT_NE(simulation.vehicles().find(3), nullptr);

    EXPECT_EQ(
        simulation.vehicles().find(1)->routeFailure,
        RouteFailure::InvalidRoute);

    EXPECT_EQ(
        simulation.vehicles().find(2)->state,
        VehicleState::Completed);

    EXPECT_EQ(
        simulation.vehicles().find(3)->state,
        VehicleState::Completed);

    EXPECT_EQ(
        simulation.currentTick(),
        scenario.config.duration);
}

// FR-14: Scenario incident records must be parsed and validated.
TEST(
    ScenarioInputIntegrationTest,
    ParsesAndValidatesScheduledIncidents
) {
    std::istringstream input(
        "scenario 1\n"
        "seed 42\n"
        "duration 5\n"
        "intersection A\n"
        "intersection B\n"
        "road AB A B 5 1\n"
        "lane entry AB 2\n"
        "incident AB 1 4\n");

    const auto scenario = ScenarioLoader::load(input);

    ASSERT_EQ(scenario.incidents.size(), 1U);

    EXPECT_EQ(scenario.incidents[0].roadId, "AB");
    EXPECT_EQ(scenario.incidents[0].startsAt, 1U);
    EXPECT_EQ(scenario.incidents[0].endsAt, 4U);
}

TEST(
    ScenarioInputIntegrationTest,
    RejectsIncidentWithUnknownRoad
) {
    std::istringstream input(
        "scenario 1\n"
        "seed 42\n"
        "duration 5\n"
        "intersection A\n"
        "intersection B\n"
        "road AB A B 5 1\n"
        "lane entry AB 2\n"
        "incident UNKNOWN 1 4\n");

    EXPECT_THROW(
        ScenarioLoader::load(input),
        std::invalid_argument);
}

// FR-14 and FR-15: An active incident blocks a road.
// Production routing should select an available alternative.
TEST(
    ScenarioInputIntegrationTest,
    IncidentAwareRoutingUsesAlternativeRoad
) {
    std::istringstream input(
        "scenario 1\n"
        "seed 42\n"
        "duration 4\n"
        "intersection A\n"
        "intersection B\n"
        "intersection C\n"
        "intersection D\n"
        "road AB A B 5 1\n"
        "road BC B C 5 1\n"
        "road BD B D 5 2\n"
        "road DC D C 5 2\n"
        "lane entry AB 2\n"
        "incident BC 0 2\n"
        "arrival 0 entry C standard -\n");

    const auto scenario = ScenarioLoader::load(input);

    Simulation simulation({});
    simulation.configureScenario(scenario);

    const auto batch = simulation.step();

    ASSERT_EQ(batch.generated.size(), 1U);
    ASSERT_EQ(batch.admitted.size(), 1U);

    const auto* vehicle =
        simulation.vehicles().find(batch.generated.front());

    ASSERT_NE(vehicle, nullptr);

    EXPECT_EQ(
        vehicle->route,
        (std::vector<RoadId>{"AB", "BD", "DC"}));
}


// Merge regression: FR-11/12 must coexist with adaptive signals, incidents and reports.
TEST(ScenarioInputIntegrationTest, LaneCapacityPreservesSignalsIncidentsReportsAndReset) {
    std::istringstream input(
        "scenario 1\nseed 42\nduration 12\n"
        "intersection A\nintersection B\nintersection C\n"
        "road AB A B 2 1\nroad BC B C 2 2\n"
        "lane entry AB 2\nlane outbound BC 1\n"
        "phase B 4 1 1 AB-to-BC\nadaptive B 2 6 1 1\n"
        "incident BC 2 4\n"
        "arrival 0 entry C standard -\narrival 0 entry C standard -\n");
    Simulation simulation({});
    simulation.configureScenario(ScenarioLoader::load(input));
    const auto firstBatch = simulation.step();
    ASSERT_EQ(firstBatch.admitted.size(), 2U);
    for (const auto id : firstBatch.admitted) {
        ASSERT_NE(simulation.vehicles().find(id), nullptr);
        EXPECT_EQ(simulation.vehicles().find(id)->route,
                  (std::vector<RoadId>{"AB", "BC"}));
    }
    simulation.step();
    EXPECT_EQ(simulation.vehicles().find(1)->routeIndex, 1U);
    EXPECT_EQ(simulation.vehicles().find(2)->routeIndex, 0U);
    EXPECT_EQ(simulation.vehicles().find(2)->waitingTicks, 1U);
    simulation.step();
    EXPECT_EQ(simulation.createReport().activeIncidents, 1U);
    EXPECT_EQ(simulation.vehicles().find(2)->routeIndex, 0U);
    simulation.run();
    const auto report = simulation.createReport();
    EXPECT_EQ(report.generated, 2U);
    EXPECT_EQ(report.metrics.completed, 2U);
    EXPECT_EQ(report.metrics.unfinished, 0U);
    EXPECT_EQ(report.activeIncidents, 0U);
    std::ostringstream firstCsv;
    ReportWriter::writeCsv(firstCsv, report);

    // Reset while the downstream lane is occupied; stale occupancy must not survive.
    simulation.reset();
    simulation.step();
    simulation.step();
    ASSERT_EQ(simulation.vehicles().find(1)->routeIndex, 1U);
    simulation.reset();
    simulation.run();
    std::ostringstream secondCsv;
    ReportWriter::writeCsv(secondCsv, simulation.createReport());
    EXPECT_EQ(secondCsv.str(), firstCsv.str());
}

TEST(ScenarioInputIntegrationTest, RoutesThroughPrescribedEntryDespiteCheaperOtherRoad) {
    std::istringstream input(
        "scenario 1\nseed 42\nduration 8\n"
        "intersection A\nintersection B\nintersection C\n"
        "road AB A B 2 2\nroad BC B C 2 2\nroad AC A C 2 1\n"
        "lane entry AB 1\nphase B 10 1 1 AB-to-BC\n"
        "arrival 0 entry C standard -\n");
    Simulation simulation({});
    simulation.configureScenario(ScenarioLoader::load(input));
    simulation.run();
    ASSERT_NE(simulation.vehicles().find(1), nullptr);
    EXPECT_EQ(simulation.vehicles().find(1)->route, (std::vector<RoadId>{"AB", "BC"}));
    EXPECT_EQ(simulation.metricsSnapshot().completed, 1U);
    EXPECT_EQ(simulation.inputSummary().unroutable, 0U);
}

TEST(ScenarioInputIntegrationTest, EmergencyVehicleRequestsSafePhaseWithoutReorderingEntryQueue) {
    std::istringstream input(
        "scenario 1\nseed 42\nduration 9\nintersection A\nintersection B\n"
        "intersection C\nintersection D\nintersection E\n"
        "road AB A B 2 1\nroad BC B C 2 1\nroad BD B D 2 1\nroad BE B E 2 1\n"
        "lane entry AB 2\nphase B 2 1 1 AB-to-BC\nphase B 2 1 1 AB-to-BD\n"
        "phase B 2 1 1 AB-to-BE\nconflict B AB-to-BC AB-to-BE\n"
        "arrival 0 entry E emergency AB BE\n");
    Simulation simulation({});
    simulation.configureScenario(ScenarioLoader::load(input));
    for (int i = 0; i < 5; ++i) simulation.step();
    EXPECT_EQ(simulation.vehicles().find(1)->routeIndex, 1U);
    EXPECT_TRUE(simulation.signalSnapshots(4).front().second.permittedMovements ==
                std::vector<MovementId>{"AB-to-BE"});
    simulation.run();
    EXPECT_EQ(simulation.metricsSnapshot().completed, 1U);
}

TEST(ScenarioInputIntegrationTest, TC_15_ReroutesExistingVehicleAfterLateIncidentAndReplays) {
    std::istringstream input(
        "scenario 1\nseed 42\nduration 8\nintersection A\nintersection B\n"
        "intersection C\nintersection D\nroad AB A B 2 1\nroad BC B C 2 1\n"
        "road BD B D 2 1\nroad DC D C 2 1\nlane entry AB 2\n"
        "phase B 10 1 1 AB-to-BC AB-to-BD\nphase D 10 1 1 BD-to-DC\n"
        "incident BC 1 7\narrival 0 entry C standard -\n");
    Simulation simulation({});
    simulation.configureScenario(ScenarioLoader::load(input));
    simulation.step();
    EXPECT_EQ(simulation.vehicles().find(1)->route, (std::vector<RoadId>{"AB", "BC"}));
    simulation.step();
    EXPECT_EQ(simulation.vehicles().find(1)->route, (std::vector<RoadId>{"AB", "BD", "DC"}));
    simulation.run();
    EXPECT_EQ(simulation.metricsSnapshot().completed, 1U);
    simulation.reset();
    simulation.run();
    EXPECT_EQ(simulation.metricsSnapshot().completed, 1U);
}

TEST(ScenarioInputIntegrationTest, TC_15_NoAlternativeWaitsSafelyUntilRoadRestores) {
    auto scenario = fixture();
    scenario.arrivals.resize(1);
    scenario.config.duration = 10;
    scenario.incidents = {{"BC", 1, 4}};
    Simulation simulation({});
    simulation.configureScenario(scenario);
    for (int i = 0; i < 4; ++i) simulation.step();
    EXPECT_EQ(simulation.vehicles().find(1)->routeIndex, 0U);
    simulation.run();
    EXPECT_EQ(simulation.metricsSnapshot().completed, 1U);
}

TEST(ScenarioInputIntegrationTest, TC_15_CongestionTriggersAlternativeWithoutIncident) {
    std::istringstream input(
        "scenario 1\nseed 42\nduration 8\nintersection A\nintersection B\n"
        "intersection C\nintersection D\nroad AB A B 2 1\nroad BC B C 1 4\n"
        "road BD B D 2 2\nroad DC D C 2 3\nlane entry AB 2\nlane direct BC 1\n"
        "phase B 10 1 1 AB-to-BC AB-to-BD\nphase D 10 1 1 BD-to-DC\n"
        "arrival 0 direct C standard BC\narrival 1 entry C standard -\n");
    Simulation simulation({});
    simulation.configureScenario(ScenarioLoader::load(input));
    simulation.step();
    EXPECT_EQ(simulation.vehicles().find(1)->state, VehicleState::InTransit);
    simulation.step();
    EXPECT_EQ(simulation.vehicles().find(2)->route, (std::vector<RoadId>{"AB", "BD", "DC"}));
    simulation.run();
    EXPECT_EQ(simulation.metricsSnapshot().completed, 2U);
}

}  // namespace
}  // namespace traffic
