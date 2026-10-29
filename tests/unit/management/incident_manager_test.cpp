#include "traffic/management/incident_manager.hpp"
#include <gtest/gtest.h>
using namespace traffic;

TEST(IncidentManagerTest, BlocksAndRestoresRoadAtHalfOpenInterval) {
    RoadNetwork network;
    ASSERT_TRUE(network.addRoad({"r1", "A", "B", 10, 1, 0, true}));
    IncidentManager manager;
    manager.schedule({"r1", 2, 4});
    manager.advance(1, network);
    ASSERT_TRUE(network.findRoad("r1")->enabled);
    manager.advance(2, network);
    EXPECT_FALSE(network.findRoad("r1")->enabled);
    manager.advance(4, network);
    EXPECT_TRUE(network.findRoad("r1")->enabled);
}

TEST(IncidentManagerTest, PreservesInitiallyDisabledRoad) {
    RoadNetwork network;
    ASSERT_TRUE(network.addRoad({"r1", "A", "B", 10, 1, 0, false}));
    IncidentManager manager;
    manager.schedule({"r1", 1, 2});
    manager.advance(1, network);
    manager.advance(2, network);
    EXPECT_FALSE(network.findRoad("r1")->enabled);
}

TEST(IncidentManagerTest, RejectsInvalidInterval) {
    IncidentManager manager;
    EXPECT_THROW(manager.schedule({"r1", 3, 3}), std::invalid_argument);
}


TEST(IncidentManagerTest, ReroutesVehicleAroundBlockedRoad) {
    RoadNetwork network;
    ASSERT_TRUE(network.addRoad({"ab", "A", "B", 10, 1, 0, true}));
    ASSERT_TRUE(network.addRoad({"bd", "B", "D", 10, 1, 0, true}));
    ASSERT_TRUE(network.addRoad({"ac", "A", "C", 10, 2, 0, true}));
    ASSERT_TRUE(network.addRoad({"cd", "C", "D", 10, 2, 0, true}));
    VehicleManager vehicles(1);
    vehicles.configureTopology({"A", "B", "C", "D"}, {
        {"ab", "A", "B", 10, 1, 0, true}, {"bd", "B", "D", 10, 1, 0, true},
        {"ac", "A", "C", 10, 2, 0, true}, {"cd", "C", "D", 10, 2, 0, true}});
    Vehicle v;
    v.source = "A"; v.destination = "D"; v.route = {"ab", "bd"};
    const auto id = vehicles.addVehicle(v);
    IncidentManager manager;
    manager.schedule({"ab", 1, 3});
    manager.advance(1, network, vehicles);
    const auto* changed = vehicles.find(id);
    ASSERT_NE(changed, nullptr);
    EXPECT_EQ(changed->route, (std::vector<RoadId>{"ac", "cd"}));
    EXPECT_EQ(changed->state, VehicleState::Waiting);
}
