#include "traffic/input/scenario.hpp"

#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>

namespace traffic {
namespace {
const std::string base =
    "scenario 1\nseed 42\nduration 5\n"
    "intersection A\nintersection B\nintersection C\n"
    "road AB A B 2 1\nroad BC B C 2 1\nlane entry AB 1\n";

Scenario parse(const std::string& text) {
    std::istringstream input(text);
    return ScenarioLoader::load(input);
}

TEST(ScenarioTest, TC_01_LoadsForwardReferencesClassesRoutesAndSignals) {
    const auto scenario = parse("# comment\nscenario 1\narrival 0 entry C emergency AB BC\n"
        "stream random entry C standard 1 5 0.5 -\n"
        "phase B 3 1 1 straight\n"
        "seed 42\nduration 5\nintersection A\nintersection B\nintersection C\n"
        "road AB A B 2 1\nroad BC B C 2 1\nlane entry AB 1\n");
    EXPECT_EQ(scenario.config.seed, 42U);
    ASSERT_EQ(scenario.arrivals.size(), 1U);
    EXPECT_EQ(scenario.arrivals[0].vehicle.source, "A");
    EXPECT_EQ(scenario.arrivals[0].vehicle.vehicleClass, VehicleClass::Emergency);
    EXPECT_EQ(scenario.arrivals[0].vehicle.route, (std::vector<RoadId>{"AB", "BC"}));
    ASSERT_EQ(scenario.streams.size(), 1U);
    EXPECT_DOUBLE_EQ(scenario.streams[0].probability, 0.5);
    EXPECT_TRUE(scenario.streams[0].vehicle.route.empty());
    ASSERT_EQ(scenario.signals.size(), 1U);
    EXPECT_EQ(scenario.signals[0].intersection, "B");
}

TEST(ScenarioTest, UT_A07_RejectsNegativeOverflowZeroAndMalformedNumbers) {
    for (const auto& value : {"-1", "0", "18446744073709551616", "2x", "1.5"}) {
        const auto text = "scenario 1\nseed 1\nduration " + std::string(value) +
            "\nintersection A\nintersection B\nroad AB A B 1 1\nlane entry AB 1\n";
        EXPECT_THROW(parse(text), std::invalid_argument) << value;
    }
    EXPECT_THROW(parse(base + "road bad A B -1 1\n"), std::invalid_argument);
    EXPECT_THROW(parse(base + "road bad A B 1 0\n"), std::invalid_argument);
    EXPECT_THROW(parse(base + "lane bad AB 0\n"), std::invalid_argument);
}

TEST(ScenarioTest, RejectsBadRecordsUnknownReferencesAndDuplicates) {
    for (const auto& tail : {
        "unknown value\n", "seed 2\n", "duration 2\n", "intersection A\n",
        "road AB A B 2 1\n", "road bad A missing 2 1\n", "lane entry AB 1\n",
        "lane bad missing 1\n", "lane too-large AB 3\n", "arrival 0 missing C standard -\n",
        "arrival 0 entry missing standard -\n", "arrival 0 entry C flying -\n",
        "arrival 0 entry C standard BC\n", "arrival 5 entry C standard -\n",
        "arrival 0 entry C standard - AB\n", "stream x entry C standard 0 6 0.5 -\n",
        "stream x entry C standard 0 2 nan -\n", "stream x entry C standard 0 2 1.1 -\n",
        "phase missing 3 1 1 go\n", "phase B 0 1 1 go\n",
        "phase B 3 1 1 one two\nconflict B one two\n"}) {
        EXPECT_THROW(parse(base + tail), std::invalid_argument) << tail;
    }
    EXPECT_THROW(parse("scenario 2\n"), std::invalid_argument);
    EXPECT_THROW(parse("scenario 1\n"), std::invalid_argument);
    EXPECT_THROW(parse(base + "seed\n"), std::invalid_argument);
}

TEST(ScenarioTest, BoundsInputAndPotentialVehicleCounts) {
    EXPECT_THROW(parse(base + std::string(4097, 'x')), std::invalid_argument);
    EXPECT_THROW(parse(base + std::string(1024 * 1024, '\n')), std::invalid_argument);
    auto scenario = parse(base);
    scenario.config.duration = 100000;
    Vehicle v; v.source = "A"; v.destination = "C";
    scenario.streams = {{"one", "entry", 0, 100000, 1, v}, {"two", "entry", 0, 1, 1, v}};
    EXPECT_THROW(ScenarioLoader::validate(scenario), std::invalid_argument);
}

TEST(ScenarioTest, FailedLoadLeavesPreviouslyLoadedValueUnchanged) {
    auto scenario = parse(base);
    try {
        scenario = parse(base + "lane bad missing 1\n");
        FAIL() << "Invalid scenario was accepted";
    } catch (const std::invalid_argument&) {
        EXPECT_EQ(scenario.config.seed, 42U);
        ASSERT_EQ(scenario.lanes.size(), 1U);
        EXPECT_EQ(scenario.lanes[0].id, "entry");
    }
    EXPECT_THROW(ScenarioLoader::loadFile("/nonexistent/traffic-scenario.txt"), std::runtime_error);
    std::istringstream broken(base);
    broken.setstate(std::ios::badbit);
    EXPECT_THROW(ScenarioLoader::load(broken), std::runtime_error);
}
}  // namespace
}  // namespace traffic
