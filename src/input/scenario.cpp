#include "traffic/input/scenario.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <limits>
#include <locale>
#include <map>
#include <sstream>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace traffic {
namespace {
constexpr std::size_t maxBytes = 1024 * 1024;
constexpr std::size_t maxLine = 4096;
constexpr std::uint64_t maxDuration = 100000;
constexpr std::uint64_t maxValue = 1000000;
constexpr std::size_t maxVehicles = 100000;

std::uint64_t number(const std::string& text, std::uint64_t maximum, bool positive = false) {
    std::uint64_t value{};
    const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
        value > maximum || (positive && value == 0))
        throw std::invalid_argument("Invalid or out-of-range unsigned number: " + text);
    return value;
}

double probability(const std::string& text) {
    std::istringstream input(text);
    input.imbue(std::locale::classic());
    double value{};
    if (!(input >> value) || input.peek() != std::char_traits<char>::eof() ||
        !std::isfinite(value) || value < 0 || value > 1)
        throw std::invalid_argument("Probability must be between 0 and 1");
    return value;
}

VehicleClass vehicleClass(const std::string& text) {
    if (text == "standard") return VehicleClass::Standard;
    if (text == "emergency") return VehicleClass::Emergency;
    throw std::invalid_argument("Class must be standard or emergency");
}

void fields(const std::vector<std::string>& words, std::size_t count, bool atLeast = false) {
    if (atLeast ? words.size() < count : words.size() != count)
        throw std::invalid_argument("Wrong number of fields for " + words.front());
}

struct Record {
    std::vector<std::string> words;
    std::size_t line{};
};

std::invalid_argument lineError(std::size_t line, const std::string& message) {
    return std::invalid_argument("Scenario line " + std::to_string(line) + ": " + message);
}
}  // namespace

Scenario ScenarioLoader::load(std::istream& input) {
    Scenario scenario;
    bool header = false;
    bool duration = false;
    bool seed = false;
    std::size_t bytes = 0;
    std::size_t lineNumber = 0;
    std::vector<Record> arrivals;
    std::map<std::string, std::pair<std::size_t, std::size_t>> batches;
    std::map<IntersectionId, ScenarioSignals> signals;
    // Read with explicit bounds instead of letting getline allocate an unbounded line.
    std::string line;
    while (true) {
        line.clear();
        char character{};
        while (input.get(character)) {
            if (++bytes > maxBytes) throw std::invalid_argument("Scenario exceeds 1 MiB");
            if (character == '\n') break;
            if (line.size() == maxLine) throw std::invalid_argument("Scenario line exceeds 4096 bytes");
            line.push_back(character);
        }
        if (input.bad() || (input.fail() && !input.eof())) throw std::runtime_error("Cannot read scenario");
        if (line.empty() && input.eof()) break;
        ++lineNumber;
        const auto comment = line.find('#');
        if (comment != std::string::npos) line.erase(comment);
        std::istringstream tokens(line);
        tokens.imbue(std::locale::classic());
        std::vector<std::string> words;
        for (std::string word; tokens >> word;) words.push_back(word);
        if (words.empty()) continue;
        try {
            const auto& kind = words.front();
            if (!header) {
                fields(words, 2);
                if (kind != "scenario" || words[1] != "1")
                    throw std::invalid_argument("First record must be scenario 1");
                header = true;
            } else if (kind == "duration") {
                fields(words, 2);
                if (duration) throw std::invalid_argument("Duplicate duration");
                scenario.config.duration = number(words[1], maxDuration, true);
                duration = true;
            } else if (kind == "seed") {
                fields(words, 2);
                if (seed) throw std::invalid_argument("Duplicate seed");
                scenario.config.seed = number(words[1], std::numeric_limits<std::uint64_t>::max());
                seed = true;
            } else if (kind == "intersection") {
                fields(words, 2);
                scenario.intersections.push_back(words[1]);
            } else if (kind == "road") {
                fields(words, 6);
                scenario.roads.push_back({words[1], words[2], words[3],
                    static_cast<std::size_t>(number(words[4], maxValue, true)), number(words[5], maxValue, true)});
            } else if (kind == "lane") {
                fields(words, 4);
                scenario.lanes.push_back({words[1], words[2],
                    static_cast<std::size_t>(number(words[3], maxValue, true))});
            } else if (kind == "phase") {
                fields(words, 6, true);
                auto& plan = signals[words[1]];
                plan.intersection = words[1];
                plan.phases.push_back({{words.begin() + 5, words.end()},
                    number(words[2], maxValue, true), number(words[3], maxValue, true), number(words[4], maxValue, true)});
            } else if (kind == "conflict") {
                fields(words, 4);
                auto& plan = signals[words[1]];
                plan.intersection = words[1];
                plan.conflicts.emplace_back(words[2], words[3]);
            } else if (kind == "batch") {
                fields(words, 3);
                const auto count = static_cast<std::size_t>(
                    number(words[2], ArrivalStream::maxVehiclesPerArrival, true));
                if (!batches.emplace(words[1], std::make_pair(count, lineNumber)).second)
                    throw std::invalid_argument("Duplicate batch setting for stream: " + words[1]);
            } else if (kind == "arrival" || kind == "stream") {
                fields(words, kind == "arrival" ? 6 : 9, true);
                arrivals.push_back({std::move(words), lineNumber});
            } else {
                throw std::invalid_argument("Unknown record: " + kind);
            }
        } catch (const std::invalid_argument& error) {
            throw lineError(lineNumber, error.what());
        }
    }
    if (!header || !duration || !seed) throw std::invalid_argument("Scenario requires version, duration, and seed");
    for (auto& entry : signals) scenario.signals.push_back(std::move(entry.second));
    for (const auto& record : arrivals) {
        try {
            const auto& words = record.words;
            const bool stream = words[0] == "stream";
            const auto& laneId = words[2];
            const auto lane = std::find_if(scenario.lanes.begin(), scenario.lanes.end(),
                [&](const auto& value) { return value.id == laneId; });
            if (lane == scenario.lanes.end()) throw std::invalid_argument("Unknown entry lane: " + laneId);
            const auto road = std::find_if(scenario.roads.begin(), scenario.roads.end(),
                [&](const auto& value) { return value.id == lane->road; });
            if (road == scenario.roads.end()) throw std::invalid_argument("Entry lane references unknown road");
            Vehicle vehicle;
            vehicle.source = road->from;
            vehicle.destination = words[3];
            vehicle.vehicleClass = vehicleClass(words[4]);
            const std::size_t routeStart = stream ? 8 : 5;
            if (words[routeStart] == "-") {
                fields(words, routeStart + 1);
            } else {
                vehicle.route.assign(words.begin() + static_cast<std::ptrdiff_t>(routeStart), words.end());
            }
            if (stream) {
                scenario.streams.push_back({words[1], laneId, number(words[5], maxDuration),
                    number(words[6], maxDuration), probability(words[7]), std::move(vehicle)});
            } else {
                scenario.arrivals.push_back({number(words[1], maxDuration), laneId, std::move(vehicle)});
            }
        } catch (const std::invalid_argument& error) {
            throw lineError(record.line, error.what());
        }
    }
    for (const auto& batch : batches) {
        const auto stream = std::find_if(scenario.streams.begin(), scenario.streams.end(),
            [&](const auto& value) { return value.id == batch.first; });
        if (stream == scenario.streams.end())
            throw lineError(batch.second.second, "Batch references unknown stream: " + batch.first);
        stream->vehiclesPerArrival = batch.second.first;
    }
    validate(scenario);
    return scenario;
}

Scenario ScenarioLoader::loadFile(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open scenario file");
    return load(input);
}

void ScenarioLoader::validate(const Scenario& scenario) {
    if (scenario.config.duration == 0 || scenario.config.duration > maxDuration ||
        scenario.intersections.size() < 2 || scenario.intersections.size() > 1024 ||
        scenario.roads.empty() || scenario.roads.size() > 4096 ||
        scenario.lanes.empty() || scenario.lanes.size() > 4096 || scenario.streams.size() > 128 ||
        scenario.arrivals.size() > maxVehicles)
        throw std::invalid_argument("Scenario size or duration is outside supported bounds");
    for (const auto& road : scenario.roads) {
        if (road.capacity > maxValue || road.travelTime > maxValue || road.occupancy != 0)
            throw std::invalid_argument("Road must have bounded capacity/travel time and start empty");
    }
    for (const auto& lane : scenario.lanes) {
        if (lane.capacity > maxValue) throw std::invalid_argument("Lane capacity exceeds supported bound");
    }
    std::size_t potentialVehicles = scenario.arrivals.size();
    for (const auto& arrival : scenario.arrivals) {
        if (arrival.tick >= scenario.config.duration) throw std::invalid_argument("Arrival tick is outside run duration");
    }
    for (const auto& stream : scenario.streams) {
        if (stream.endTick > scenario.config.duration || stream.firstTick >= stream.endTick)
            throw std::invalid_argument("Stream interval is outside run duration");
        if (stream.vehiclesPerArrival == 0 || stream.vehiclesPerArrival > ArrivalStream::maxVehiclesPerArrival)
            throw std::invalid_argument("Stream batch size must be between 1 and 1000");
        const auto ticks = stream.endTick - stream.firstTick;
        // Divide the remaining budget before multiplying, so extreme values cannot overflow.
        if (ticks > (maxVehicles - potentialVehicles) / stream.vehiclesPerArrival)
            throw std::invalid_argument("Scenario could exceed 100000 vehicles");
        potentialVehicles += static_cast<std::size_t>(ticks) * stream.vehiclesPerArrival;
    }
    VehicleManager validator(scenario.config.seed);
    validator.configureTopology(scenario.intersections, scenario.roads);
    validator.configureLanes(scenario.lanes);
    validator.setSchedule(scenario.arrivals);
    validator.setStreams(scenario.streams);
    const std::unordered_set<IntersectionId> nodes(scenario.intersections.begin(), scenario.intersections.end());
    std::unordered_set<IntersectionId> controlled;
    SignalController signals;
    for (const auto& plan : scenario.signals) {
        if (!nodes.count(plan.intersection) || !controlled.insert(plan.intersection).second)
            throw std::invalid_argument("Unknown or duplicate controlled intersection");
        for (const auto& phase : plan.phases) {
            if (phase.greenDuration > maxValue || phase.yellowDuration > maxValue || phase.allRedDuration > maxValue)
                throw std::invalid_argument("Signal timing exceeds supported bound");
        }
        signals.configure(plan.intersection, plan.phases, plan.conflicts);
    }
}

}  // namespace traffic
