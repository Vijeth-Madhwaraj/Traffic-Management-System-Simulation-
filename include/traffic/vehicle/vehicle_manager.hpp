#pragma once

#include "traffic/types.hpp"

#include <optional>
#include <unordered_set>
#include <unordered_map>

namespace traffic {

class VehicleManager {
public:
    explicit VehicleManager(std::uint64_t seed);
    void configureTopology(std::vector<IntersectionId> intersections, std::vector<Road> roads);
    void validateVehicle(const Vehicle& vehicle) const;
    VehicleId addVehicle(Vehicle vehicle);
    const Vehicle* find(VehicleId id) const noexcept;
    Vehicle* find(VehicleId id) noexcept;
    std::size_t size() const noexcept;

private:
    std::unordered_set<IntersectionId> intersections_;
    std::unordered_map<RoadId, Road> roads_;
    std::uint64_t seed_;
    VehicleId nextId_{1};
    std::unordered_map<VehicleId, Vehicle> vehicles_;
};

}  // namespace traffic
