
#include "traffic/management/metrics_collector.hpp"

#include <gtest/gtest.h>

#include <sstream>

namespace traffic {

TEST(MetricsCollectorTest, TracksGeneratedVehicles) {
    MetricsCollector metrics;

    metrics.recordGenerated();
    metrics.recordGenerated();

    const auto result = metrics.snapshot(2);

    EXPECT_EQ(result.generated, 2);
    EXPECT_EQ(result.completed, 0);
    EXPECT_EQ(result.unfinished, 2);
}

TEST(MetricsCollectorTest, TracksCompletedVehiclesAndWaitingTime) {
    MetricsCollector metrics;

    metrics.recordGenerated();
    metrics.recordGenerated();

    metrics.recordCompleted(5);
    metrics.recordCompleted(9);

    const auto result = metrics.snapshot(0);

    EXPECT_EQ(result.generated, 2);
    EXPECT_EQ(result.completed, 2);
    EXPECT_DOUBLE_EQ(result.averageWaitingTicks, 7.0);
}

TEST(MetricsCollectorTest, TracksMaximumQueueLength) {
    MetricsCollector metrics;

    metrics.observeQueue(3);
    metrics.observeQueue(7);
    metrics.observeQueue(4);

    const auto result = metrics.snapshot(0);

    EXPECT_EQ(result.maximumQueueLength, 7);
}

TEST(MetricsCollectorTest, AverageWaitingTimeIsZeroWithoutCompletions) {
    MetricsCollector metrics;

    const auto result = metrics.snapshot(0);

    EXPECT_DOUBLE_EQ(result.averageWaitingTicks, 0.0);
}

TEST(MetricsCollectorTest, WritesCsvOutput) {
    MetricsCollector metrics;

    metrics.recordGenerated();
    metrics.observeQueue(3);

    std::ostringstream output;
    metrics.writeCsv(output, 1);

    EXPECT_NE(
        output.str().find("generated,completed,unfinished"),
        std::string::npos
    );
    EXPECT_NE(
        output.str().find("1,0,1"),
        std::string::npos
    );
}

}  // namespace traffic
