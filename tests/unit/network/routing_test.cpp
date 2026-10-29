#include "traffic/network/road_network.hpp"
#include <gtest/gtest.h>
using namespace traffic;

TEST(RoadNetworkRoutingTest, FindsLeastTravelTimePathAndAvoidsDisabledRoads) {
    RoadNetwork network;
    ASSERT_TRUE(network.addRoad({"ab", "A", "B", 10, 1, 0, true}));
    ASSERT_TRUE(network.addRoad({"bd", "B", "D", 10, 1, 0, true}));
    ASSERT_TRUE(network.addRoad({"ac", "A", "C", 10, 2, 0, true}));
    ASSERT_TRUE(network.addRoad({"cd", "C", "D", 10, 2, 0, true}));
    EXPECT_EQ(network.shortestRoute("A", "D"), (std::vector<RoadId>{"ab", "bd"}));
    network.findRoad("ab")->enabled = false;
    EXPECT_EQ(network.shortestRoute("A", "D"), (std::vector<RoadId>{"ac", "cd"}));
}

TEST(RoadNetworkRoutingTest, ReturnsEmptyForUnreachableOrSameEndpoint) {
    RoadNetwork network;
    ASSERT_TRUE(network.addRoad({"ab", "A", "B", 10, 1, 0, true}));
    EXPECT_TRUE(network.shortestRoute("A", "Z").empty());
    EXPECT_TRUE(network.shortestRoute("A", "A").empty());
}
