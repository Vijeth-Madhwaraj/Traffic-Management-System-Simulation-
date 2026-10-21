#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace traffic {

using Tick = std::uint64_t;
using VehicleId = std::uint64_t;
using RoadId = std::string;
using IntersectionId = std::string;

enum class VehicleClass { Standard, Emergency };
enum class VehicleState { Waiting, InTransit, Completed, Unroutable };
enum class SignalColor { Red, Yellow, Green };

struct Vehicle {
    VehicleId id{};
    IntersectionId source;
    IntersectionId destination;
    VehicleClass vehicleClass{VehicleClass::Standard};
    VehicleState state{VehicleState::Waiting};
    std::vector<RoadId> route;
    std::size_t routeIndex{};
    Tick waitingTicks{};
};

struct Road {
    RoadId id;
    IntersectionId from;
    IntersectionId to;
    std::size_t capacity{};
    Tick travelTime{1};
    std::size_t occupancy{};
    bool enabled{true};
};

struct SimulationConfig {
    Tick duration{100};
    std::uint64_t seed{1};
};

}  // namespace traffic
