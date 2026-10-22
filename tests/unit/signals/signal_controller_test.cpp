#include "traffic/signals/signal_controller.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <utility>
#include <vector>

namespace traffic {
namespace {

SignalPhase makePhase(std::vector<MovementId> movements,
                      Tick green = 3,
                      Tick yellow = 2,
                      Tick allRed = 1) {
    return {std::move(movements), green, yellow, allRed};
}

const std::vector<MovementConflict> kConflicts{{"north-south", "east-west"}};

TEST(SignalControllerTest, UT_VI01_StartsEachIntersectionInItsConfiguredInitialPhase) {
    SignalController signals;
    signals.configure("junction-a",
                      {makePhase({"north-south"}), makePhase({"east-west"})},
                      kConflicts);
    signals.configure("junction-b", {makePhase({"left-turn"})}, {});

    const auto first = signals.status("junction-a", 0);
    const auto second = signals.status("junction-b", 0);

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(first->phaseIndex, 0U);
    EXPECT_EQ(first->color, SignalColor::Green);
    EXPECT_EQ(first->permittedMovements, std::vector<MovementId>{"north-south"});
    EXPECT_TRUE(signals.permits("junction-a", "north-south"));
    EXPECT_FALSE(signals.permits("junction-a", "east-west"));
    EXPECT_EQ(second->phaseIndex, 0U);
    EXPECT_TRUE(signals.permits("junction-b", "left-turn"));
    EXPECT_FALSE(signals.status("unknown-junction", 0).has_value());
}

TEST(SignalControllerTest, UT_VI02_ChangesFromGreenAtTheConfiguredBoundary) {
    SignalController signals;
    signals.configure("junction", {makePhase({"north-south"}), makePhase({"east-west"})}, kConflicts);

    signals.advance(2);
    const auto beforeBoundary = signals.status("junction", 2);
    ASSERT_TRUE(beforeBoundary.has_value());
    EXPECT_EQ(beforeBoundary->color, SignalColor::Green);
    EXPECT_TRUE(signals.permits("junction", "north-south"));

    signals.advance(3);
    const auto atBoundary = signals.status("junction", 3);
    ASSERT_TRUE(atBoundary.has_value());
    EXPECT_EQ(atBoundary->color, SignalColor::Yellow);
    EXPECT_FALSE(signals.permits("junction", "north-south"));
    EXPECT_FALSE(signals.permits("junction", "east-west"));
}

TEST(SignalControllerTest, UT_VI03_UsesYellowAndAllRedClearanceBeforeNextGreen) {
    SignalController signals;
    signals.configure("junction", {makePhase({"north-south"}), makePhase({"east-west"})}, kConflicts);

    signals.advance(3);
    auto state = signals.status("junction", 3);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->color, SignalColor::Yellow);
    EXPECT_TRUE(state->permittedMovements.empty());

    signals.advance(5);
    state = signals.status("junction", 5);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->color, SignalColor::Red);
    EXPECT_EQ(state->phaseIndex, 0U);
    EXPECT_TRUE(state->permittedMovements.empty());

    signals.advance(6);
    state = signals.status("junction", 6);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->phaseIndex, 1U);
    EXPECT_EQ(state->color, SignalColor::Green);
    EXPECT_TRUE(signals.permits("junction", "east-west"));
    EXPECT_FALSE(signals.permits("junction", "north-south"));
}

TEST(SignalControllerTest, UT_VI04_NeverPermitsConflictingMovementsAtTheSameTime) {
    SignalController signals;
    signals.configure("junction",
                      {makePhase({"north-south"}), makePhase({"east-west"})},
                      kConflicts);

    for (Tick tick = 0; tick <= 18; ++tick) {
        signals.advance(tick);
        EXPECT_FALSE(signals.permits("junction", "north-south") &&
                     signals.permits("junction", "east-west"))
            << "Both conflicting movements were permitted at tick " << tick;
    }

    SignalController invalidPlan;
    EXPECT_THROW(invalidPlan.configure("junction",
                                       {makePhase({"north-south", "east-west"})},
                                       kConflicts),
                 std::invalid_argument);
}

TEST(SignalControllerTest, UT_VI08_RejectsEmptyOrInvalidSignalPlans) {
    SignalController signals;

    EXPECT_THROW(signals.configure("junction", {}, {}), std::invalid_argument);
    EXPECT_THROW(signals.configure("", {makePhase({"north-south"})}, {}), std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", {makePhase({})}, {}), std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", {makePhase({""})}, {}), std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", {makePhase({"north-south", "north-south"})}, {}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", {makePhase({"north-south"}, 0)}, {}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", {makePhase({"north-south"}, 3, 0)}, {}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", {makePhase({"north-south"}, 3, 2, 0)}, {}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", {makePhase({"north-south"})}, {{"north-south", ""}}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", {makePhase({"north-south"})}, {{"north-south", "missing"}}),
                 std::invalid_argument);
}

TEST(SignalControllerTest, UT_VI08_RejectsBackwardTicksWithoutChangingSignalState) {
    SignalController signals;
    signals.configure("junction", {makePhase({"north-south"}, 5)}, {});
    signals.advance(4);

    EXPECT_THROW(signals.advance(3), std::invalid_argument);
    const auto state = signals.status("junction", 4);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->color, SignalColor::Green);
    EXPECT_EQ(state->colorElapsed, 4U);
}

TEST(SignalControllerTest, UT_VI08_InvalidReconfigurationLeavesTheValidPlanIntact) {
    SignalController signals;
    signals.configure("junction", {makePhase({"north-south"})}, {});

    EXPECT_THROW(signals.configure("junction", {makePhase({"east-west"}, 0)}, {}),
                 std::invalid_argument);
    EXPECT_TRUE(signals.permits("junction", "north-south"));
    EXPECT_FALSE(signals.permits("junction", "east-west"));
}

TEST(SignalControllerTest, UT_VI08_AdvancesAcrossSkippedTicksDeterministically) {
    SignalController signals;
    signals.configure("junction", {makePhase({"north-south"}), makePhase({"east-west"})}, kConflicts);

    signals.advance(12);
    const auto state = signals.status("junction", 12);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->phaseIndex, 0U);
    EXPECT_EQ(state->color, SignalColor::Green);
    EXPECT_EQ(state->colorElapsed, 0U);
}

}  // namespace
}  // namespace traffic
