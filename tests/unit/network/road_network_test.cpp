#include "traffic/network/road_network.hpp"

#include <gtest/gtest.h>

namespace traffic {
namespace {

TEST(RoadNetworkTest, UT_VA01_RegistersConnectedIntersectionsAndExposesRoadProperties) {
    RoadNetwork network;
    ASSERT_TRUE(network.addIntersection("north"));
    ASSERT_TRUE(network.addIntersection("south"));

    EXPECT_TRUE(network.hasIntersection("north"));
    EXPECT_TRUE(network.hasIntersection("south"));
    EXPECT_FALSE(network.hasIntersection("unknown"));
    ASSERT_TRUE(network.addRoad({"north-south", "north", "south", 8, 5, 0, true}));

    const auto* road = network.findRoad("north-south");
    ASSERT_NE(road, nullptr);
    EXPECT_EQ(road->from, "north");
    EXPECT_EQ(road->to, "south");
    EXPECT_EQ(road->capacity, 8U);
    EXPECT_EQ(road->travelTime, 5U);
    EXPECT_EQ(road->occupancy, 0U);
    EXPECT_TRUE(road->enabled);
}

TEST(RoadNetworkTest, UT_VA02_RejectsInvalidOrUnconnectedRoadsWithoutChangingNetwork) {
    RoadNetwork network;
    ASSERT_TRUE(network.addIntersection("north"));
    ASSERT_TRUE(network.addIntersection("south"));
    ASSERT_TRUE(network.addRoad({"north-south", "north", "south", 8, 5, 2, true}));

    EXPECT_FALSE(network.addIntersection(""));
    EXPECT_FALSE(network.addIntersection("north"));
    EXPECT_FALSE(network.addRoad({"unknown-endpoint", "north", "west", 1, 1, 0, true}));
    EXPECT_FALSE(network.addRoad({"", "north", "south", 1, 1, 0, true}));
    EXPECT_FALSE(network.addRoad({"zero-capacity", "north", "south", 0, 1, 0, true}));
    EXPECT_FALSE(network.addRoad({"zero-travel-time", "north", "south", 1, 0, 0, true}));
    EXPECT_FALSE(network.addRoad({"over-capacity", "north", "south", 1, 1, 2, true}));
    EXPECT_FALSE(network.addRoad({"north-south", "north", "south", 3, 4, 0, false}));

    const auto* road = network.findRoad("north-south");
    ASSERT_NE(road, nullptr);
    EXPECT_EQ(road->capacity, 8U);
    EXPECT_EQ(road->travelTime, 5U);
    EXPECT_EQ(road->occupancy, 2U);
    EXPECT_EQ(network.findRoad("unknown-endpoint"), nullptr);
}

}  // namespace
}  // namespace traffic
