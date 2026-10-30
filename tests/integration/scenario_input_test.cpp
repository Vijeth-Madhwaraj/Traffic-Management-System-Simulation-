#include "traffic/input/scenario.hpp"
#include "traffic/simulation/simulation.hpp"

#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>

namespace traffic {
namespace {
Scenario fixture() {
    std::istringstream file(
        "scenario 1\nseed 42\nduration 6\nintersection A\nintersection B\nintersection C\n"
        "road AB A B 2 1\nroad BC B C 2 1\nlane entry AB 1\n"
        "phase B 2 1 1 AB-to-BC\n"
        "arrival 0 entry C standard AB BC\narrival 0 entry C emergency AB BC\n"
        "arrival 1 entry C standard AB BC\n");
    return ScenarioLoader::load(file);
}

TEST(ScenarioInputIntegrationTest, TC_01_TC_04_LoadsGeneratesAndMovesExplicitRoutes) {
    Simulation simulation({});
    simulation.configureScenario(fixture());
    EXPECT_EQ(simulation.step().generated, (std::vector<VehicleId>{1, 2}));
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
        EXPECT_EQ(simulation.vehicles().find(id)->state, VehicleState::Completed);
    }
    simulation.run();
    EXPECT_EQ(simulation.inputSummary().generated, 3U);
}

TEST(ScenarioInputIntegrationTest, InvalidReplacementPreservesRunningSimulation) {
    Simulation simulation({});
    simulation.configureScenario(fixture());
    simulation.step();
    auto invalid = fixture();
    invalid.lanes[0].road = "unknown";
    EXPECT_THROW(simulation.configureScenario(invalid), std::invalid_argument);
    EXPECT_EQ(simulation.currentTick(), 1U);
    EXPECT_EQ(simulation.inputSummary().generated, 2U);
    simulation.run();
    EXPECT_EQ(simulation.inputSummary().generated, 3U);
}

TEST(ScenarioInputIntegrationTest, TC_18_ResetRepeatsActualSeededArrivalEvents) {
    auto scenario = fixture();
    scenario.config.duration = 100;
    scenario.arrivals.clear();
    auto vehicle = fixture().arrivals[0].vehicle;
    scenario.streams = {{"traffic", "entry", 0, 100, 0.5, vehicle}};
    Simulation simulation({});
    simulation.configureScenario(scenario);
    const auto run = [&]() {
        std::vector<Tick> ticks;
        while (simulation.currentTick() < scenario.config.duration) {
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

TEST(ScenarioInputIntegrationTest, UsesInjectedRoutesAndReportsProductionRoutingStubAsUnroutable) {
    auto scenario = fixture();
    scenario.arrivals.resize(1);
    scenario.arrivals[0].vehicle.route.clear();
    Simulation simulation({});
    simulation.configureScenario(scenario);
    simulation.run();
    EXPECT_EQ(simulation.inputSummary().unroutable, 1U);
    EXPECT_EQ(simulation.inputSummary().admitted, 0U);
    simulation.reset();
    simulation.setRouteProvider([](const auto& source, const auto& destination) {
        EXPECT_EQ(source, "A"); EXPECT_EQ(destination, "C");
        return std::vector<RoadId>{"AB", "BC"};
    });
    simulation.run();
    EXPECT_EQ(simulation.inputSummary().unroutable, 0U);
    EXPECT_EQ(simulation.inputSummary().admitted, 1U);
    EXPECT_THROW(simulation.setRouteProvider({}), std::logic_error);
}

TEST(ScenarioInputIntegrationTest, RequiresValidatedScenarioBeforeRunning) {
    Simulation simulation({});
    EXPECT_THROW(simulation.step(), std::logic_error);
    EXPECT_THROW(simulation.run(), std::logic_error);
    EXPECT_THROW(simulation.reset(), std::logic_error);
}
}  // namespace
}  // namespace traffic
