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
        "duration 4\n"
        "intersection A\n"
        "intersection B\n"
        "road AB A B 2 1\n"
        "lane entry AB 1\n"
        "phase A 2 1 1 AB\n"
        "arrival 0 entry B standard AB\n"
        "arrival 0 entry B emergency AB\n"
        "arrival 1 entry B standard AB\n"
    );

    return ScenarioLoader::load(file);
}

TEST(
    ScenarioInputIntegrationTest,
    TC_01_TC_04_LoadsGeneratesAndDefersWithoutClaimingMovement
) {
    Simulation simulation({});
    simulation.configureScenario(fixture());

    EXPECT_EQ(
        simulation.step().generated,
        (std::vector<VehicleId>{1, 2})
    );

    simulation.run();

    const auto summary = simulation.inputSummary();

    EXPECT_EQ(summary.generated, 3U);
    EXPECT_EQ(summary.admitted, 1U);
    EXPECT_EQ(summary.queued, 1U);
    EXPECT_EQ(summary.pending, 2U);
    EXPECT_EQ(summary.unroutable, 0U);
    EXPECT_EQ(simulation.currentTick(), 4U);

    simulation.run();

    EXPECT_EQ(
        simulation.inputSummary().generated,
        3U
    );
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
        std::invalid_argument
    );

    EXPECT_EQ(simulation.currentTick(), 1U);
    EXPECT_EQ(
        simulation.inputSummary().generated,
        2U
    );

    simulation.run();

    EXPECT_EQ(
        simulation.inputSummary().generated,
        3U
    );
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
        {"traffic", "entry", 0, 100, 0.5, vehicle}
    };

    Simulation simulation({});
    simulation.configureScenario(scenario);

    const auto run = [&]() {
        std::vector<Tick> ticks;

        while (
            simulation.currentTick() <
            scenario.config.duration
        ) {
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
    EXPECT_EQ(
        simulation.inputSummary().generated,
        0U
    );

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

    simulation.run();

    EXPECT_EQ(
        simulation.inputSummary().unroutable,
        0U
    );

    EXPECT_EQ(
        simulation.inputSummary().admitted,
        1U
    );

    simulation.reset();

    simulation.setRouteProvider(
        [](const auto& source, const auto& destination) {
            EXPECT_EQ(source, "A");
            EXPECT_EQ(destination, "B");

            return std::vector<RoadId>{"AB"};
        }
    );

    simulation.run();

    EXPECT_EQ(
        simulation.inputSummary().unroutable,
        0U
    );

    EXPECT_EQ(
        simulation.inputSummary().admitted,
        1U
    );

    EXPECT_THROW(
        simulation.setRouteProvider({}),
        std::logic_error
    );
}

TEST(
    ScenarioInputIntegrationTest,
    RequiresValidatedScenarioBeforeRunning
) {
    Simulation simulation({});

    EXPECT_THROW(
        simulation.step(),
        std::logic_error
    );

    EXPECT_THROW(
        simulation.run(),
        std::logic_error
    );

    EXPECT_THROW(
        simulation.reset(),
        std::logic_error
    );
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
        "incident AB 1 4\n"
    );

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
        "incident UNKNOWN 1 4\n"
    );

    EXPECT_THROW(
        ScenarioLoader::load(input),
        std::invalid_argument
    );
}

// FR-14 and FR-15: An active incident blocks a road.
// Production routing selects an available alternative.
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
        "arrival 0 entry C standard -\n"
    );

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
        (std::vector<RoadId>{"AB", "BD", "DC"})
    );
}

}  // namespace
}  // namespace traffic