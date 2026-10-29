#include "traffic/input/scenario.hpp"
#include "traffic/simulation/simulation.hpp"
#include <gtest/gtest.h>
using namespace traffic;

TEST(IncidentMetricsIntegrationTest, RunsScenarioAndCollectsGenerationAndQueueMetrics) {
    const auto scenario = ScenarioLoader::loadFile(std::string(TEST_SOURCE_DIR) + "/config/demo_scenario.txt");
    Simulation simulation(scenario.config);
    simulation.configureScenario(scenario);
    simulation.run();
    const auto summary = simulation.inputSummary();
    const auto metrics = simulation.metricsSnapshot();
    EXPECT_EQ(metrics.generated, summary.generated);
    EXPECT_GE(metrics.maximumQueueLength, 0U);
    EXPECT_LE(metrics.completed, metrics.generated);
}
