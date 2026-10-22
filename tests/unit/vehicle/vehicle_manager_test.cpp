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
} // namespace
} // namespace traffic
