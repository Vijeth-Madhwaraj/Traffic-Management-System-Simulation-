
#include "traffic/management/incident_manager.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

using namespace traffic;

TEST(IncidentManagerTest, BlocksAndRestoresRoadAtHalfOpenInterval) {
    RoadNetwork network;
    ASSERT_TRUE(network.addIntersection("A"));
    ASSERT_TRUE(network.addIntersection("B"));
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
    ASSERT_TRUE(network.addIntersection("A"));
    ASSERT_TRUE(network.addIntersection("B"));
    ASSERT_TRUE(network.addRoad({"r1", "A", "B", 10, 1, 0, false}));

    IncidentManager manager;
    manager.schedule({"r1", 1, 2});

    manager.advance(1, network);
    manager.advance(2, network);

    EXPECT_FALSE(network.findRoad("r1")->enabled);
}

TEST(IncidentManagerTest, RejectsInvalidInterval) {
    IncidentManager manager;

    EXPECT_THROW(
        manager.schedule({"r1", 3, 3}),
        std::invalid_argument
    );
}
