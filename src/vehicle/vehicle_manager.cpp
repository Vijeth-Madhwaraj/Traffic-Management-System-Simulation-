#include "traffic/vehicle/vehicle_manager.hpp"

namespace traffic {

VehicleManager::VehicleManager(std::uint64_t seed) : seed_(seed) {}

VehicleId VehicleManager::addVehicle(Vehicle vehicle) {
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
