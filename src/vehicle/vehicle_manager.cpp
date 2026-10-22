#include "traffic/vehicle/vehicle_manager.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace traffic {

VehicleManager::VehicleManager(std::uint64_t seed) : seed_(seed) {}

void VehicleManager::configureTopology(std::vector<IntersectionId> intersections, std::vector<Road> roads) {
    if (!vehicles_.empty()) throw std::logic_error("Cannot replace topology after vehicles are created");
    std::unordered_set<IntersectionId> nodes;
    std::unordered_map<RoadId, Road> edges;
    for (const auto& id : intersections) {
        if (id.empty() || !nodes.insert(id).second) throw std::invalid_argument("Invalid or duplicate intersection");
    }
    if (nodes.empty()) throw std::invalid_argument("Topology requires intersections");
    for (const auto& road : roads) {
        if (road.id.empty() || !nodes.count(road.from) || !nodes.count(road.to) ||
            road.capacity == 0 || road.travelTime == 0 || road.occupancy > road.capacity ||
            !edges.emplace(road.id, road).second) throw std::invalid_argument("Invalid or duplicate road");
    }
    intersections_ = std::move(nodes);
    roads_ = std::move(edges);
}

void VehicleManager::validateVehicle(const Vehicle& vehicle) const {
    if (!intersections_.count(vehicle.source) || !intersections_.count(vehicle.destination) ||
        vehicle.source == vehicle.destination) throw std::invalid_argument("Vehicle requires distinct known endpoints");
    if (vehicle.vehicleClass != VehicleClass::Standard && vehicle.vehicleClass != VehicleClass::Emergency)
        throw std::invalid_argument("Unknown vehicle class");
    if (vehicle.state != VehicleState::Waiting || vehicle.routeIndex != 0 || vehicle.waitingTicks != 0)
        throw std::invalid_argument("New vehicle must have initial state");
    auto node = vehicle.source;
    for (const auto& id : vehicle.route) {
        const auto road = roads_.find(id);
        if (road == roads_.end() || !road->second.enabled || road->second.from != node)
            throw std::invalid_argument("Vehicle route contains an invalid road connection");
        node = road->second.to;
    }
    if (!vehicle.route.empty() && node != vehicle.destination)
        throw std::invalid_argument("Vehicle route does not reach destination");
}

void VehicleManager::setSchedule(std::vector<ScheduledArrival> arrivals) {
    if (lastGenerated_) throw std::logic_error("Cannot change arrivals after generation starts");
    for (const auto& arrival : arrivals) {
        if (arrival.lane.empty()) throw std::invalid_argument("Arrival requires an entry lane");
        validateVehicle(arrival.vehicle);
    }
    std::stable_sort(arrivals.begin(), arrivals.end(), [](const auto& a, const auto& b) { return a.tick < b.tick; });
    schedule_ = std::move(arrivals);
    nextArrival_ = 0;
}

std::vector<GeneratedArrival> VehicleManager::generate(Tick now) {
    if (lastGenerated_ && now == *lastGenerated_) return {};
    if ((!lastGenerated_ && now != 0) ||
        (lastGenerated_ && (*lastGenerated_ == std::numeric_limits<Tick>::max() || now != *lastGenerated_ + 1)))
        throw std::invalid_argument("Generate requires consecutive ticks starting at zero");
    std::vector<GeneratedArrival> result;
    while (nextArrival_ < schedule_.size() && schedule_[nextArrival_].tick == now) {
        const auto& arrival = schedule_[nextArrival_];
        result.push_back({arrival.lane, addVehicle(arrival.vehicle)});
        ++nextArrival_;
    }
    lastGenerated_ = now;
    return result;
}

VehicleId VehicleManager::addVehicle(Vehicle vehicle) {
    validateVehicle(vehicle);
    if (nextId_ == std::numeric_limits<VehicleId>::max()) throw std::overflow_error("Vehicle IDs exhausted");
    vehicle.id = nextId_++;
    const auto id = vehicle.id;
    vehicles_.emplace(id, std::move(vehicle));
    return id;
}

const Vehicle* VehicleManager::find(VehicleId id) const noexcept {
    const auto it = vehicles_.find(id);
    return it == vehicles_.end() ? nullptr : &it->second;
}

Vehicle* VehicleManager::find(VehicleId id) noexcept {
    const auto it = vehicles_.find(id);
    return it == vehicles_.end() ? nullptr : &it->second;
}

std::size_t VehicleManager::size() const noexcept { return vehicles_.size(); }

}  // namespace traffic
