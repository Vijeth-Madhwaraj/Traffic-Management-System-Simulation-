#pragma once

#include "traffic/types.hpp"

#include <optional>
#include <random>
#include <unordered_set>
#include <unordered_map>

namespace traffic {

struct ScheduledArrival {
    Tick tick{};
    std::string lane;
    Vehicle vehicle;
};

struct ArrivalStream {
    std::string id;
    std::string lane;
    Tick firstTick{};
    Tick endTick{}; // exclusive
    double probability{}; // at most one arrival per tick
    Vehicle vehicle;
};

struct GeneratedArrival {
    std::string lane;
    VehicleId id{};
};

class VehicleManager {
public:
    explicit VehicleManager(std::uint64_t seed);
    void configureTopology(std::vector<IntersectionId> intersections, std::vector<Road> roads);
    void validateVehicle(const Vehicle& vehicle) const;
    void setSchedule(std::vector<ScheduledArrival> arrivals);
    void setStreams(std::vector<ArrivalStream> streams);
    void reset();
    std::uint64_t seed() const noexcept;
    std::vector<GeneratedArrival> generate(Tick now);
    VehicleId addVehicle(Vehicle vehicle);
    const Vehicle* find(VehicleId id) const noexcept;
    Vehicle* find(VehicleId id) noexcept;
    std::size_t size() const noexcept;

private:
    std::unordered_set<IntersectionId> intersections_;
    std::unordered_map<RoadId, Road> roads_;
    std::vector<ArrivalStream> streams_;
    std::mt19937_64 rng_;
    std::vector<ScheduledArrival> schedule_;
    std::size_t nextArrival_{};
    std::optional<Tick> lastGenerated_;
    std::uint64_t seed_;
    VehicleId nextId_{1};
    std::unordered_map<VehicleId, Vehicle> vehicles_;
};

}  // namespace traffic
