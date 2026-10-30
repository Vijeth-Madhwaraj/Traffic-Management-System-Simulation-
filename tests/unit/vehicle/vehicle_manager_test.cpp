#include "traffic/vehicle/vehicle_manager.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

namespace traffic {
namespace {
class VehicleInputTest : public ::testing::Test {
protected:
    VehicleManager manager{42};
    void SetUp() override {
        manager.configureTopology({"A", "B", "C"},
            {{"AB", "A", "B", 2, 1}, {"BC", "B", "C", 2, 1}});
    }
    static Vehicle vehicle() {
        Vehicle v;
        v.source = "A";
        v.destination = "C";
        v.route = {"AB", "BC"};
        return v;
    }
};
TEST_F(VehicleInputTest, UT_A01_AssignsUniqueIdsAndPreservesAttributes) {
    auto v = vehicle();
    v.vehicleClass = VehicleClass::Emergency;
    const auto first = manager.addVehicle(v);
    const auto second = manager.addVehicle(v);
    EXPECT_NE(first, second);
    ASSERT_NE(manager.find(first), nullptr);
    EXPECT_EQ(manager.find(first)->route, v.route);
    EXPECT_EQ(manager.find(first)->vehicleClass, VehicleClass::Emergency);
    EXPECT_EQ(manager.find(999), nullptr);
}
TEST_F(VehicleInputTest, UT_A03_RejectsUnknownOrEqualEndpointsWithoutAllocatingId) {
    auto v = vehicle();
    v.source = "missing";
    EXPECT_THROW(manager.addVehicle(v), std::invalid_argument);
    v = vehicle(); v.destination = "A";
    EXPECT_THROW(manager.addVehicle(v), std::invalid_argument);
    EXPECT_EQ(manager.size(), 0U);
    EXPECT_EQ(manager.addVehicle(vehicle()), 1U);
}
TEST_F(VehicleInputTest, RejectsInvalidClassStateAndDisconnectedRoutes) {
    auto v = vehicle(); v.vehicleClass = static_cast<VehicleClass>(99);
    EXPECT_THROW(manager.addVehicle(v), std::invalid_argument);
    v = vehicle(); v.route = {"BC", "AB"};
    EXPECT_THROW(manager.addVehicle(v), std::invalid_argument);
    v.route = {"AB"};
    EXPECT_THROW(manager.addVehicle(v), std::invalid_argument);
    v.route = {"missing"};
    EXPECT_THROW(manager.addVehicle(v), std::invalid_argument);
    v = vehicle(); v.state = VehicleState::Completed;
    EXPECT_THROW(manager.addVehicle(v), std::invalid_argument);
}
TEST_F(VehicleInputTest, InvalidTopologyReplacementPreservesOriginal) {
    EXPECT_THROW(manager.configureTopology({"A"}, {{"bad", "A", "B", 1, 1}}), std::invalid_argument);
    EXPECT_NO_THROW(manager.addVehicle(vehicle()));
}
TEST_F(VehicleInputTest, UT_A08_GeneratesExactlyAtScheduledTickAndOnlyOnce) {
    manager.setSchedule({{2, "entry", vehicle()}, {0, "entry", vehicle()}, {2, "entry", vehicle()}});
    const auto first = manager.generate(0);
    ASSERT_EQ(first.size(), 1U);
    EXPECT_EQ(first[0].id, 1U);
    EXPECT_TRUE(manager.generate(0).empty());
    EXPECT_TRUE(manager.generate(1).empty());
    const auto later = manager.generate(2);
    ASSERT_EQ(later.size(), 2U);
    EXPECT_EQ(later[0].id, 2U);
    EXPECT_EQ(later[1].id, 3U);
    EXPECT_EQ(later[0].lane, "entry");
    EXPECT_THROW(manager.generate(1), std::invalid_argument);
    EXPECT_THROW(manager.generate(4), std::invalid_argument);
    EXPECT_THROW(manager.setSchedule({}), std::logic_error);
}
TEST_F(VehicleInputTest, RejectsInvalidScheduleWithoutReplacingValidOne) {
    manager.setSchedule({{0, "entry", vehicle()}});
    EXPECT_THROW(manager.setSchedule({{0, "", vehicle()}}), std::invalid_argument);
    EXPECT_EQ(manager.generate(0).size(), 1U);
}
TEST_F(VehicleInputTest, UT_A02_A10_SeededStreamsRepeatAfterReset) {
    manager.setStreams({{"random", "entry", 0, 100, 0.5, vehicle()}});
    const auto run = [&]() {
        std::vector<Tick> arrivals;
        for (Tick tick = 0; tick < 100; ++tick) {
            for (const auto& event : manager.generate(tick)) {
                arrivals.push_back(tick);
                EXPECT_EQ(event.id, arrivals.size());
            }
        }
        return arrivals;
    };
    const auto first = run();
    EXPECT_GT(first.size(), 0U);
    EXPECT_LT(first.size(), 100U);
    manager.reset();
    EXPECT_EQ(manager.size(), 0U);
    EXPECT_EQ(manager.seed(), 42U);
    EXPECT_EQ(run(), first);
}
TEST_F(VehicleInputTest, ValidatesProbabilityAndHonorsStreamBounds) {
    EXPECT_THROW(manager.setStreams({{"x", "entry", 0, 2, -0.1, vehicle()}}), std::invalid_argument);
    EXPECT_THROW(manager.setStreams({{"x", "entry", 0, 2, 1.1, vehicle()}}), std::invalid_argument);
    EXPECT_THROW(manager.setStreams({{"x", "entry", 2, 2, 1, vehicle()}}), std::invalid_argument);
    manager.setStreams({{"never", "entry", 0, 4, 0, vehicle()}, {"always", "entry", 1, 3, 1, vehicle()}});
    EXPECT_TRUE(manager.generate(0).empty());
    EXPECT_EQ(manager.generate(1).size(), 1U);
    EXPECT_EQ(manager.generate(2).size(), 1U);
    EXPECT_TRUE(manager.generate(3).empty());
}
TEST_F(VehicleInputTest, StreamOrderIsStableRegardlessOfConfigurationOrder) {
    const auto a = ArrivalStream{"a", "a-lane", 0, 1, 1, vehicle()};
    const auto z = ArrivalStream{"z", "z-lane", 0, 1, 1, vehicle()};
    manager.setStreams({z, a});
    const auto arrivals = manager.generate(0);
    ASSERT_EQ(arrivals.size(), 2U);
    EXPECT_EQ(arrivals[0].lane, "a-lane");
    EXPECT_EQ(arrivals[1].lane, "z-lane");
}
TEST_F(VehicleInputTest, UT_A04_A05_A06_RetriesPendingBeforeNewArrivalsWithoutChangingIds) {
    manager.configureLanes({{"entry", "AB", 1}});
    manager.setSchedule({{0, "entry", vehicle()}, {0, "entry", vehicle()}, {1, "entry", vehicle()}});
    auto batch = manager.advanceArrivals(0, {});
    EXPECT_EQ(batch.generated, (std::vector<VehicleId>{1, 2}));
    EXPECT_EQ(batch.admitted, (std::vector<VehicleId>{1}));
    EXPECT_EQ(manager.pendingCount(), 1U);
    EXPECT_EQ(manager.lane("entry")->pop(), 1U);
    batch = manager.advanceArrivals(1, {});
    EXPECT_EQ(batch.admitted, (std::vector<VehicleId>{2}));
    EXPECT_EQ(manager.pendingCount(), 1U);
    EXPECT_EQ(manager.lane("entry")->pop(), 2U);
    batch = manager.advanceArrivals(2, {});
    EXPECT_EQ(batch.admitted, (std::vector<VehicleId>{3}));
    EXPECT_EQ(manager.pendingCount(), 0U);
    EXPECT_TRUE(manager.advanceArrivals(2, {}).generated.empty());
    manager.reset();
    EXPECT_EQ(manager.queuedCount(), 0U);
    EXPECT_EQ(manager.pendingCount(), 0U);
    EXPECT_EQ(manager.advanceArrivals(0, {}).generated, (std::vector<VehicleId>{1, 2}));
}
TEST_F(VehicleInputTest, UT_A09_RequestsCorrectRouteAndHandlesUnavailableRoutes) {
    manager.configureLanes({{"entry", "AB", 1}});
    auto v = vehicle(); v.route.clear();
    manager.setSchedule({{0, "entry", v}, {1, "entry", v}});
    auto batch = manager.advanceArrivals(0, [](const auto& source, const auto& destination) {
        EXPECT_EQ(source, "A"); EXPECT_EQ(destination, "C");
        return std::vector<RoadId>{"AB", "BC"};
    });
    EXPECT_EQ(batch.admitted.size(), 1U);
    batch = manager.advanceArrivals(1, [](const auto&, const auto&) { return std::vector<RoadId>{}; });
    EXPECT_EQ(batch.unroutable, (std::vector<VehicleId>{2}));
    EXPECT_EQ(manager.find(2)->state, VehicleState::Unroutable);
}
TEST_F(VehicleInputTest, FailedRouteProviderDoesNotConsumeArrivals) {
    manager.configureLanes({{"entry", "AB", 1}});
    auto v = vehicle(); v.route.clear();
    manager.setSchedule({{0, "entry", v}});
    EXPECT_THROW(manager.advanceArrivals(0, [](const auto&, const auto&) -> std::vector<RoadId> {
        throw std::runtime_error("provider unavailable");
    }), std::runtime_error);
    EXPECT_EQ(manager.size(), 0U);
    EXPECT_EQ(manager.advanceArrivals(0, {}).unroutable, (std::vector<VehicleId>{1}));
}
TEST_F(VehicleInputTest, RejectsLaneMismatchAndUnknownLanesBeforeGeneration) {
    manager.configureLanes({{"entry", "AB", 1}});
    EXPECT_THROW(manager.setSchedule({{0, "missing", vehicle()}}), std::invalid_argument);
    auto v = vehicle(); v.source = "B"; v.route = {"BC"};
    EXPECT_THROW(manager.setSchedule({{0, "entry", v}}), std::invalid_argument);
    EXPECT_EQ(manager.size(), 0U);
}
TEST_F(VehicleInputTest, FR04_PerLaneSnapshotsTrackIndependentFifoRetriesAndReset) {
    manager.configureLanes({{"z-entry", "AB", 1}, {"a-entry", "AB", 1}});
    manager.setSchedule({{0, "z-entry", vehicle()}, {0, "z-entry", vehicle()},
                         {0, "a-entry", vehicle()}, {0, "a-entry", vehicle()}});
    auto snapshots = manager.laneSnapshots();
    ASSERT_EQ(snapshots.size(), 2U);
    EXPECT_TRUE(snapshots[0].queued.empty());
    EXPECT_TRUE(snapshots[0].pending.empty());
    manager.advanceArrivals(0, {});
    snapshots = manager.laneSnapshots();
    EXPECT_EQ(snapshots[0].id, "a-entry");
    EXPECT_EQ(snapshots[0].road, "AB");
    EXPECT_EQ(snapshots[0].capacity, 1U);
    EXPECT_EQ(snapshots[0].queued, (std::vector<VehicleId>{3}));
    EXPECT_EQ(snapshots[0].pending, (std::vector<VehicleId>{4}));
    EXPECT_EQ(snapshots[1].id, "z-entry");
    EXPECT_EQ(snapshots[1].queued, (std::vector<VehicleId>{1}));
    EXPECT_EQ(snapshots[1].pending, (std::vector<VehicleId>{2}));
    snapshots[0].queued.clear();
    snapshots[0].pending.clear();
    EXPECT_EQ(manager.queuedCount(), 2U);
    EXPECT_EQ(manager.pendingCount(), 2U);
    EXPECT_EQ(manager.lane("a-entry")->pop(), 3U);
    manager.advanceArrivals(1, {});
    snapshots = manager.laneSnapshots();
    EXPECT_EQ(snapshots[0].queued, (std::vector<VehicleId>{4}));
    EXPECT_TRUE(snapshots[0].pending.empty());
    EXPECT_EQ(snapshots[1].pending, (std::vector<VehicleId>{2}));
    manager.reset();
    for (const auto& lane : manager.laneSnapshots()) {
        EXPECT_TRUE(lane.queued.empty());
        EXPECT_TRUE(lane.pending.empty());
    }
}
} // namespace
} // namespace traffic
