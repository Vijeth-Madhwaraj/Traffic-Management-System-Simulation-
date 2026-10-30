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
TEST(LaneQueueTest, FR04_RejectsZeroAndDuplicateIdsWithoutUsingCapacity) {
    LaneQueue queue(3);
    EXPECT_FALSE(queue.enqueue(0));
    EXPECT_TRUE(queue.enqueue(7));
    EXPECT_FALSE(queue.enqueue(7));
    EXPECT_EQ(queue.size(), 1U);
    EXPECT_TRUE(queue.enqueue(9));
    EXPECT_EQ(queue.snapshot(), (std::vector<VehicleId>{7, 9}));
    EXPECT_EQ(queue.pop(), 7U);
    EXPECT_TRUE(queue.enqueue(7)); // An ID may re-enter only after leaving the queue.
    EXPECT_EQ(queue.snapshot(), (std::vector<VehicleId>{9, 7}));
}
TEST(LaneQueueTest, FR04_SnapshotPreservesOrderWithoutExposingMutableStorage) {
    LaneQueue queue(2);
    queue.enqueue(4);
    queue.enqueue(2);
    auto snapshot = queue.snapshot();
    snapshot.clear();
    EXPECT_EQ(queue.snapshot(), (std::vector<VehicleId>{4, 2}));
    EXPECT_EQ(queue.front(), 4U);
    queue.clear();
    EXPECT_TRUE(queue.snapshot().empty());
}
} // namespace traffic
