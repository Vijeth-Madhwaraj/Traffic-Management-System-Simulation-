#include "traffic/management/congestion_detector.hpp"
#include <gtest/gtest.h>

#include <limits>
#include <stdexcept>

using namespace traffic;

TEST(CongestionDetectorTest, UT_VN01_DetectsRoadAtConfiguredOccupancyThreshold) {
    CongestionDetector detector({0.8, 0.8, 0});
    Road road{"r1", "A", "B", 10, 1, 8, true};
    EXPECT_TRUE(detector.isRoadCongested(road));
    road.occupancy = 7;
    EXPECT_FALSE(detector.isRoadCongested(road));
}

TEST(CongestionDetectorTest, UT_VN02_DetectsQueueAndHonorsAbsoluteThreshold) {
    CongestionDetector detector({0.8, 0.9, 2});
    LaneQueue queue(5);
    EXPECT_FALSE(detector.isQueueCongested(queue));
    ASSERT_TRUE(queue.enqueue(1));
    ASSERT_TRUE(queue.enqueue(2));
    EXPECT_TRUE(detector.isQueueCongested(queue));
}

TEST(CongestionDetectorTest, UT_VN03_RejectsInvalidThresholdUpdate) {
    CongestionDetector detector;
    EXPECT_THROW(detector.setThresholds({1.1, 0.8, 0}), std::invalid_argument);
}

TEST(CongestionDetectorTest, UT_VN11_RejectsInvalidConstructorThresholds) {
    EXPECT_THROW(
        CongestionDetector(CongestionThresholds{1.1, 0.8, 0}),
        std::invalid_argument);
    EXPECT_THROW(
        CongestionDetector(CongestionThresholds{0.8, -0.1, 0}),
        std::invalid_argument);
    EXPECT_THROW(
        CongestionDetector(CongestionThresholds{
            0.8, std::numeric_limits<double>::infinity(), 0}),
        std::invalid_argument);
}
