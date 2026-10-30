#include "traffic/network/movement_engine.hpp"

#include <gtest/gtest.h>

#include <tuple>
#include <utility>
#include <vector>

namespace traffic {
namespace {

class MovementEngineTest : public ::testing::Test {
protected:
    RoadNetwork network;
    SignalController signals;
    VehicleManager vehicles{42};
    MovementEngine engine;

    void SetUp() override {
        ASSERT_TRUE(network.addIntersection("A"));
        ASSERT_TRUE(network.addIntersection("B"));
        ASSERT_TRUE(network.addIntersection("C"));
        ASSERT_TRUE(network.addRoad({"AB", "A", "B", 2, 1, 0, true}));
        ASSERT_TRUE(network.addRoad({"BC", "B", "C", 2, 1, 0, true}));
        vehicles.configureTopology({"A", "B", "C"},
                                   {{"AB", "A", "B", 2, 1}, {"BC", "B", "C", 2, 1}});
        vehicles.configureLanes({{"entry", "AB", 2}});
        configureSignal("AB-to-BC", 10);
    }

    void configureSignal(const MovementId& movement, Tick greenDuration) {
        signals.configure("B", {{{movement}, greenDuration, 2, 1}}, {});
    }

    VehicleId enqueueRoute(std::vector<RoadId> route = {"AB", "BC"},
                          const IntersectionId& destination = "C") {
        Vehicle vehicle;
        vehicle.source = "A";
        vehicle.destination = destination;
        vehicle.route = std::move(route);
        const auto id = vehicles.addVehicle(std::move(vehicle));
        EXPECT_TRUE(vehicles.lane("entry")->enqueue(id));
        return id;
    }

    void advance(Tick now) {
        signals.advance(now);
        engine.advance(now, network, signals, vehicles);
    }
};

TEST_F(MovementEngineTest, UT_VA05_EntersRoadAndCrossesOnlyOnPermittedGreen) {
    const auto id = enqueueRoute();

    advance(0);
    ASSERT_NE(vehicles.find(id), nullptr);
    EXPECT_EQ(vehicles.find(id)->state, VehicleState::InTransit);
    EXPECT_EQ(vehicles.find(id)->routeIndex, 0U);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 1U);
    EXPECT_EQ(vehicles.lane("entry")->size(), 0U);

    advance(1);
    EXPECT_EQ(vehicles.find(id)->routeIndex, 1U);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 0U);
    EXPECT_EQ(network.findRoad("BC")->occupancy, 1U);
}

TEST_F(MovementEngineTest, UT_VA06_YellowOrUnlistedMovementKeepsVehicleOnItsRouteRoad) {
    const auto id = enqueueRoute();
    configureSignal("different-movement", 1);

    advance(0);
    advance(1);  // Green expires; Yellow cannot permit a crossing.
    advance(2);  // The vehicle continues waiting during Yellow.
    advance(3);  // Red also denies the crossing.

    configureSignal("different-movement", 10);
    advance(4);  // A Green phase without this movement still denies it.

    ASSERT_NE(vehicles.find(id), nullptr);
    EXPECT_EQ(vehicles.find(id)->state, VehicleState::InTransit);
    EXPECT_EQ(vehicles.find(id)->routeIndex, 0U);
    EXPECT_EQ(vehicles.find(id)->waitingTicks, 4U);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 1U);
    EXPECT_EQ(network.findRoad("BC")->occupancy, 0U);
}

TEST_F(MovementEngineTest, UT_VA07_RespectsConfiguredRoadTravelTime) {
    network.findRoad("AB")->travelTime = 2;
    const auto id = enqueueRoute();

    advance(0);
    advance(1);
    EXPECT_EQ(vehicles.find(id)->routeIndex, 0U);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 1U);
    EXPECT_EQ(network.findRoad("BC")->occupancy, 0U);

    advance(2);
    EXPECT_EQ(vehicles.find(id)->routeIndex, 1U);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 0U);
    EXPECT_EQ(network.findRoad("BC")->occupancy, 1U);
}

TEST_F(MovementEngineTest, UT_VA08_DoesNotCrossWhenDownstreamRoadIsFull) {
    network.findRoad("BC")->occupancy = network.findRoad("BC")->capacity;
    const auto id = enqueueRoute();

    advance(0);
    advance(1);

    EXPECT_EQ(vehicles.find(id)->routeIndex, 0U);
    EXPECT_EQ(vehicles.find(id)->waitingTicks, 1U);
    EXPECT_EQ(network.findRoad("AB")->occupancy, 1U);
    EXPECT_EQ(network.findRoad("BC")->occupancy, network.findRoad("BC")->capacity);
}

TEST_F(MovementEngineTest, UT_VA09_CompletesVehicleAfterTraversingItsPlannedRoute) {
    const auto id = enqueueRoute();

    advance(0);
    advance(1);
    advance(2);

    ASSERT_NE(vehicles.find(id), nullptr);
    EXPECT_EQ(vehicles.find(id)->state, VehicleState::Completed);
    EXPECT_EQ(vehicles.find(id)->routeIndex, vehicles.find(id)->route.size());
    EXPECT_EQ(network.findRoad("AB")->occupancy, 0U);
    EXPECT_EQ(network.findRoad("BC")->occupancy, 0U);
}

TEST_F(MovementEngineTest, UT_VA10_NFR02_IsDeterministicAndCallableWithoutSimulation) {
    const auto runDirectly = [] {
        RoadNetwork network;
        EXPECT_TRUE(network.addIntersection("A"));
        EXPECT_TRUE(network.addIntersection("B"));
        EXPECT_TRUE(network.addIntersection("C"));
        EXPECT_TRUE(network.addRoad({"AB", "A", "B", 2, 1, 0, true}));
        EXPECT_TRUE(network.addRoad({"BC", "B", "C", 2, 1, 0, true}));
        SignalController signals;
        signals.configure("B", {{{"AB-to-BC"}, 10, 2, 1}}, {});
        VehicleManager vehicles{42};
        vehicles.configureTopology({"A", "B", "C"},
                                   {{"AB", "A", "B", 2, 1}, {"BC", "B", "C", 2, 1}});
        vehicles.configureLanes({{"entry", "AB", 2}});
        Vehicle vehicle;
        vehicle.source = "A";
        vehicle.destination = "C";
        vehicle.route = {"AB", "BC"};
        const auto id = vehicles.addVehicle(std::move(vehicle));
        EXPECT_TRUE(vehicles.lane("entry")->enqueue(id));
        MovementEngine engine;
        std::vector<std::tuple<VehicleState, std::size_t, std::size_t, std::size_t>> states;
        for (Tick tick = 0; tick < 3; ++tick) {
            signals.advance(tick);
            engine.advance(tick, network, signals, vehicles);
            if (tick == 0) engine.advance(tick, network, signals, vehicles);
            states.emplace_back(vehicles.find(id)->state, vehicles.find(id)->routeIndex,
                                network.findRoad("AB")->occupancy, network.findRoad("BC")->occupancy);
        }
        return states;
    };

    EXPECT_EQ(runDirectly(), runDirectly());
}

}  // namespace
}  // namespace traffic
