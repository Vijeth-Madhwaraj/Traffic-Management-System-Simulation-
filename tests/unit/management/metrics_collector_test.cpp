#include "traffic/management/metrics_collector.hpp"
#include <gtest/gtest.h>
using namespace traffic;

TEST(MetricsCollectorTest, ComputesSnapshotAndAverageWaiting) {
    MetricsCollector metrics;
    metrics.recordGenerated(); metrics.recordGenerated(); metrics.recordGenerated();
    EXPECT_TRUE(metrics.recordCompletedVehicle(1, 4));
    EXPECT_FALSE(metrics.recordCompletedVehicle(1, 99));
    EXPECT_TRUE(metrics.recordCompletedVehicle(2, 6));
    metrics.observeQueue(3); metrics.observeQueue(1); metrics.recordCongestionEvent();
    const auto snapshot = metrics.snapshot();
    EXPECT_EQ(snapshot.generated, 3U);
    EXPECT_EQ(snapshot.completed, 2U);
    EXPECT_EQ(snapshot.unfinished, 1U);
    EXPECT_DOUBLE_EQ(snapshot.averageWaitingTicks, 5.0);
    EXPECT_EQ(snapshot.maximumQueueLength, 3U);
    EXPECT_EQ(snapshot.congestionEvents, 1U);
}

TEST(MetricsCollectorTest, EmptySnapshotDoesNotDivideByZero) {
    MetricsCollector metrics;
    EXPECT_DOUBLE_EQ(metrics.snapshot().averageWaitingTicks, 0.0);
}
