#include "traffic/management/congestion_detector.hpp"
#include <gtest/gtest.h>

using namespace traffic;

TEST(CongestionDetectorTest, DetectsRoadAtConfiguredOccupancyThreshold) {
    CongestionDetector detector({0.8, 0.8, 0});
    Road road{"r1", "A", "B", 10, 1, 8, true};
    EXPECT_TRUE(detector.isRoadCongested(road));
    road.occupancy = 7;
    EXPECT_FALSE(detector.isRoadCongested(road));
}

TEST(CongestionDetectorTest, DetectsQueueAndHonorsAbsoluteThreshold) {
    CongestionDetector detector({0.8, 0.9, 2});
    LaneQueue queue(5);
    EXPECT_FALSE(detector.isQueueCongested(queue));
    ASSERT_TRUE(queue.enqueue(1));
    ASSERT_TRUE(queue.enqueue(2));
    EXPECT_TRUE(detector.isQueueCongested(queue));
}

TEST(CongestionDetectorTest, RejectsInvalidThresholdUpdate) {
    CongestionDetector detector;
    EXPECT_THROW(detector.setThresholds({1.1, 0.8, 0}), std::invalid_argument);
}
