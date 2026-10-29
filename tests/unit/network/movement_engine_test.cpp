#include "traffic/network/movement_engine.hpp"

#include <gtest/gtest.h>

using namespace traffic;

TEST(MovementEngineTest, MovesAlongRouteAndCompletesAfterTravelTimes) {
    RoadNetwork network;
    ASSERT_TRUE(network.addRoad({"AB", "A", "B", 2, 1, 0, true}));
    ASSERT_TRUE(network.addRoad({"BC", "B", "C", 2, 1, 0, true}));

    VehicleManager vehicles(7);
    vehicles.configureTopology({"A", "B", "C"}, {
        {"AB", "A", "B", 2, 1, 0, true},
        {"BC", "B", "C", 2, 1, 0, true}
    });
    vehicles.configureLanes({{"entry", "AB", 2}});
    Vehicle vehicle;
    vehicle.source = "A";
    vehicle.destination = "C";
    vehicle.route = {"AB", "BC"};
    vehicles.setSchedule({{0, "entry", vehicle}});
    const auto arrivals = vehicles.advanceArrivals(0, [](const auto&, const auto&) {
        return std::vector<RoadId>{"AB", "BC"};
    });
    ASSERT_EQ(arrivals.admitted.size(), 1U);

    SignalController signals;
    signals.configure("B", {{ {"AB-to-BC"}, 10, 1, 1 }}, {});
    MovementEngine movement;

    signals.advance(0);
    movement.advance(0, network, signals, vehicles);
    auto* tracked = vehicles.find(arrivals.admitted.front());
    ASSERT_NE(tracked, nullptr);
    EXPECT_EQ(tracked->state, VehicleState::InTransit);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 1U);

    signals.advance(1);
    movement.advance(1, network, signals, vehicles);
    EXPECT_EQ(tracked->routeIndex, 1U);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 0U);
    EXPECT_EQ(network.findRoad("BC")->occupancy, 1U);

    signals.advance(2);
    movement.advance(2, network, signals, vehicles);
    EXPECT_EQ(tracked->state, VehicleState::Completed);
    EXPECT_EQ(network.findRoad("BC")->occupancy, 0U);
}

TEST(MovementEngineTest, DoesNotEnterDownstreamRoadWhenSignalIsNotGreen) {
    RoadNetwork network;
    ASSERT_TRUE(network.addRoad({"AB", "A", "B", 2, 1, 0, true}));
    ASSERT_TRUE(network.addRoad({"BC", "B", "C", 2, 1, 0, true}));
    VehicleManager vehicles(8);
    vehicles.configureTopology({"A", "B", "C"}, {
        {"AB", "A", "B", 2, 1, 0, true},
        {"BC", "B", "C", 2, 1, 0, true}
    });
    vehicles.configureLanes({{"entry", "AB", 2}});
    Vehicle vehicle;
    vehicle.source = "A";
    vehicle.destination = "C";
    vehicle.route = {"AB", "BC"};
    vehicles.setSchedule({{0, "entry", vehicle}});
    const auto arrivals = vehicles.advanceArrivals(0, [](const auto&, const auto&) {
        return std::vector<RoadId>{"AB", "BC"};
    });
    SignalController signals;
    signals.configure("B", {{ {"another-movement"}, 1, 1, 1 }}, {});
    MovementEngine movement;
    signals.advance(0);
    movement.advance(0, network, signals, vehicles);
    signals.advance(1);
    movement.advance(1, network, signals, vehicles);
    const auto* tracked = vehicles.find(arrivals.admitted.front());
    ASSERT_NE(tracked, nullptr);
    EXPECT_EQ(tracked->routeIndex, 0U);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 1U);
    EXPECT_EQ(network.findRoad("BC")->occupancy, 0U);
}
