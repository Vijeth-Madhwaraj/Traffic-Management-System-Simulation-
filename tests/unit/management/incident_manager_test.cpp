
#include "traffic/management/incident_manager.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

using namespace traffic;

TEST(IncidentManagerTest, UT_VN04_BlocksAndRestoresRoadAtHalfOpenInterval) {
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

TEST(IncidentManagerTest, UT_VN05_PreservesInitiallyDisabledRoad) {
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

TEST(IncidentManagerTest, UT_VN06_RejectsInvalidInterval) {
    IncidentManager manager;

    EXPECT_THROW(
        manager.schedule({"r1", 3, 3}),
        std::invalid_argument
    );
}

TEST(IncidentManagerTest, UT_VN12_OverlappingIncidentsKeepRoadBlockedUntilBothEnd) {
    RoadNetwork network;
    ASSERT_TRUE(network.addIntersection("A"));
    ASSERT_TRUE(network.addIntersection("B"));
    ASSERT_TRUE(network.addIntersection("C"));
    ASSERT_TRUE(network.addRoad({"r1", "A", "B", 10, 1, 0, true}));
    ASSERT_TRUE(network.addRoad({"r2", "B", "C", 10, 1, 0, true}));

    IncidentManager manager;
    manager.schedule({"r1", 1, 3});
    manager.schedule({"r1", 2, 4});

    manager.advance(1, network);
    EXPECT_FALSE(network.findRoad("r1")->enabled);
    EXPECT_TRUE(network.findRoad("r2")->enabled);
    manager.advance(3, network);
    EXPECT_FALSE(network.findRoad("r1")->enabled);
    manager.advance(4, network);
    EXPECT_TRUE(network.findRoad("r1")->enabled);
}

TEST(IncidentManagerTest, UT_VN13_RejectsActiveUnknownRoadWithoutPartialChanges) {
    RoadNetwork network;
    ASSERT_TRUE(network.addIntersection("A"));
    ASSERT_TRUE(network.addIntersection("B"));
    ASSERT_TRUE(network.addRoad({"r1", "A", "B", 10, 1, 0, true}));

    IncidentManager manager;
    manager.schedule({"r1", 1, 3});
    manager.schedule({"missing", 1, 3});

    EXPECT_THROW(manager.advance(1, network), std::invalid_argument);
    EXPECT_TRUE(network.findRoad("r1")->enabled);
    EXPECT_FALSE(manager.isRoadBlocked("r1"));
}

TEST(IncidentManagerTest, UT_VN14_ClearRestoresRoadsBlockedByActiveIncidents) {
    RoadNetwork network;
    ASSERT_TRUE(network.addIntersection("A"));
    ASSERT_TRUE(network.addIntersection("B"));
    ASSERT_TRUE(network.addRoad({"r1", "A", "B", 10, 1, 0, true}));

    IncidentManager manager;
    manager.schedule({"r1", 1, 5});
    manager.advance(1, network);
    ASSERT_FALSE(network.findRoad("r1")->enabled);

    manager.clear(network);
    EXPECT_TRUE(network.findRoad("r1")->enabled);
    EXPECT_FALSE(manager.isRoadBlocked("r1"));
    EXPECT_TRUE(manager.incidents().empty());
}
