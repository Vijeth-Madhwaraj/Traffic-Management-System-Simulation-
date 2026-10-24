#include "traffic/vehicle/lane_queue.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
namespace traffic {
TEST(LaneQueueTest, UT_A04_A05_A06_EnforcesCapacityAndFifo) {
    LaneQueue queue(2);
    EXPECT_TRUE(queue.enqueue(7));
    EXPECT_TRUE(queue.enqueue(3));
    EXPECT_FALSE(queue.enqueue(9));
    EXPECT_EQ(queue.front(), 7U);
    EXPECT_EQ(queue.pop(), 7U);
    EXPECT_TRUE(queue.enqueue(9));
    EXPECT_EQ(queue.pop(), 3U);
    EXPECT_EQ(queue.pop(), 9U);
    EXPECT_FALSE(queue.pop().has_value());
    EXPECT_FALSE(queue.front().has_value());
    EXPECT_TRUE(queue.empty());
}
TEST(LaneQueueTest, UT_A07_A10_RejectsZeroCapacityAndClearsState) {
    EXPECT_THROW(LaneQueue(0), std::invalid_argument);
    LaneQueue queue(1);
    queue.enqueue(4);
    queue.clear();
    EXPECT_TRUE(queue.hasSpace());
    EXPECT_EQ(queue.capacity(), 1U);
    EXPECT_FALSE(queue.pop().has_value());
}
} // namespace traffic
