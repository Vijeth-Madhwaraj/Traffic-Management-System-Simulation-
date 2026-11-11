#include "traffic/signals/signal_controller.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <utility>
#include <vector>

namespace traffic {
namespace {

SignalPhase makePhase(std::vector<MovementId> movements, Tick green = 3, Tick yellow = 2,
                      Tick allRed = 1) {
    return {std::move(movements), green, yellow, allRed};
}

const std::vector<MovementConflict> kConflicts{{"north-south", "east-west"}};

TEST(SignalControllerTest, UT_VI01_StartsEachIntersectionInItsConfiguredInitialPhase) {
    SignalController signals;
    signals.configure("junction-a", {makePhase({"north-south"}), makePhase({"east-west"})},
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
    signals.configure("junction", {makePhase({"north-south"}), makePhase({"east-west"})},
                      kConflicts);

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
    signals.configure("junction", {makePhase({"north-south"}), makePhase({"east-west"})},
                      kConflicts);

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
    signals.configure("junction", {makePhase({"north-south"}), makePhase({"east-west"})},
                      kConflicts);

    for (Tick tick = 0; tick <= 18; ++tick) {
        signals.advance(tick);
        EXPECT_FALSE(signals.permits("junction", "north-south") &&
                     signals.permits("junction", "east-west"))
            << "Both conflicting movements were permitted at tick " << tick;
    }

    SignalController invalidPlan;
    EXPECT_THROW(
        invalidPlan.configure("junction", {makePhase({"north-south", "east-west"})}, kConflicts),
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
    EXPECT_THROW(
        signals.configure("junction", {makePhase({"north-south"})}, {{"north-south", "missing"}}),
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
    signals.configure("junction", {makePhase({"north-south"}), makePhase({"east-west"})},
                      kConflicts);

    signals.advance(12);
    const auto state = signals.status("junction", 12);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->phaseIndex, 0U);
    EXPECT_EQ(state->color, SignalColor::Green);
    EXPECT_EQ(state->colorElapsed, 0U);
}

TEST(SignalControllerTest, TC_07_ExtendsGreenForLongQueuesUpToMaximum) {
    SignalController signals;
    const AdaptiveSignalPolicy policy{1, 3, 2, 1};
    signals.configure("junction", {makePhase({"north-south"}, 2, 1, 1)}, {}, policy);

    MovementQueueCounts queues{{"junction", {{"north-south", 2}}}};
    signals.advance(0, queues);
    auto state = signals.status("junction", 0);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->color, SignalColor::Green);
    EXPECT_EQ(state->greenDuration, 3U);
    EXPECT_TRUE(signals.permits("junction", "north-south"));

    signals.advance(5, queues); // The next green starts after yellow and all-red.
    state = signals.status("junction", 5);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->color, SignalColor::Green);
    EXPECT_EQ(state->greenDuration, policy.maximumGreenDuration);
}

TEST(SignalControllerTest, TC_07_ShortensEmptyGreenToMinimumAndCanExtendAgain) {
    SignalController signals;
    const AdaptiveSignalPolicy policy{1, 4, 2, 1};
    signals.configure("junction", {makePhase({"north-south"}, 3, 1, 1)}, {}, policy);

    signals.advance(0, {});
    auto state = signals.status("junction", 0);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->greenDuration, 2U);
    signals.advance(2, {});
    ASSERT_EQ(signals.status("junction", 2)->color, SignalColor::Yellow);
    signals.advance(4, {});
    state = signals.status("junction", 4);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->color, SignalColor::Green);
    EXPECT_EQ(state->greenDuration, policy.minimumGreenDuration);

    MovementQueueCounts queues{{"junction", {{"north-south", 2}}}};
    signals.advance(7, queues);
    state = signals.status("junction", 7);
    ASSERT_TRUE(state.has_value());
    EXPECT_EQ(state->color, SignalColor::Green);
    EXPECT_EQ(state->greenDuration, 2U);
}

TEST(SignalControllerTest, TC_07_AdaptiveTimingPreservesConflictAndClearanceSafety) {
    SignalController signals;
    const AdaptiveSignalPolicy policy{1, 3, 1, 1};
    signals.configure("junction",
                      {makePhase({"north-south"}, 1, 1, 1), makePhase({"east-west"}, 1, 1, 1)},
                      kConflicts, policy);
    MovementQueueCounts queues{{"junction", {{"north-south", 4}, {"east-west", 4}}}};

    for (Tick tick = 0; tick <= 24; ++tick) {
        signals.advance(tick, queues);
        const auto state = signals.status("junction", tick);
        ASSERT_TRUE(state.has_value());
        EXPECT_GE(state->greenDuration, policy.minimumGreenDuration);
        EXPECT_LE(state->greenDuration, policy.maximumGreenDuration);
        EXPECT_FALSE(signals.permits("junction", "north-south") &&
                     signals.permits("junction", "east-west"));
        if (state->color != SignalColor::Green) {
            EXPECT_FALSE(signals.permits("junction", "north-south"));
            EXPECT_FALSE(signals.permits("junction", "east-west"));
        }
    }
}

TEST(SignalControllerTest, TC_07_RejectsInvalidAdaptiveLimits) {
    SignalController signals;
    const auto phases = std::vector<SignalPhase>{makePhase({"north-south"}, 2, 1, 1)};
    EXPECT_THROW(signals.configure("junction", phases, {}, AdaptiveSignalPolicy{0, 4, 2, 1}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", phases, {}, AdaptiveSignalPolicy{3, 2, 2, 1}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", phases, {}, AdaptiveSignalPolicy{1, 4, 0, 1}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", phases, {}, AdaptiveSignalPolicy{1, 4, 2, 0}),
                 std::invalid_argument);
    EXPECT_THROW(signals.configure("junction", phases, {}, AdaptiveSignalPolicy{3, 4, 2, 1}),
                 std::invalid_argument);
}

TEST(SignalControllerTest, TC_08_PrioritySkipsToRequestedPhaseAfterFullClearance) {
    SignalController signals;
    signals.configure("B",
                      {makePhase({"AB-to-BC"}, 2, 2, 1), makePhase({"AB-to-BD"}, 2, 2, 1),
                       makePhase({"AB-to-BE"}, 2, 2, 1)},
                      {});
    EXPECT_FALSE(signals.requestPriority("missing", "AB-to-BE"));
    EXPECT_FALSE(signals.requestPriority("B", "missing"));
    EXPECT_TRUE(signals.requestPriority("B", "AB-to-BE"));
    EXPECT_TRUE(signals.requestPriority("B", "AB-to-BE"));
    signals.advance(1);
    EXPECT_TRUE(signals.permits("B", "AB-to-BC"));
    signals.advance(2);
    EXPECT_EQ(signals.status("B", 2)->color, SignalColor::Yellow);
    EXPECT_FALSE(signals.permits("B", "AB-to-BE"));
    signals.advance(4);
    EXPECT_EQ(signals.status("B", 4)->color, SignalColor::Red);
    signals.advance(5);
    EXPECT_TRUE(signals.permits("B", "AB-to-BE"));
    EXPECT_FALSE(signals.permits("B", "AB-to-BC"));
    signals.advance(10);
    EXPECT_TRUE(signals.permits("B", "AB-to-BC"));
}

TEST(SignalControllerTest, TC_08_RequestsDuringClearanceRemainSafeAndFifo) {
    SignalController signals;
    signals.configure(
        "J", {makePhase({"a"}, 1, 1, 1), makePhase({"b"}, 1, 1, 1), makePhase({"c"}, 1, 1, 1)},
        {{"a", "b"}, {"a", "c"}, {"b", "c"}});
    signals.advance(1);
    EXPECT_TRUE(signals.requestPriority("J", "c"));
    EXPECT_TRUE(signals.requestPriority("J", "b"));
    signals.advance(3);
    EXPECT_TRUE(signals.permits("J", "c"));
    signals.advance(6);
    EXPECT_TRUE(signals.permits("J", "b"));
    signals.advance(9);
    EXPECT_TRUE(signals.permits("J", "c")); // Normal cycle resumes.
}

} // namespace
} // namespace traffic
