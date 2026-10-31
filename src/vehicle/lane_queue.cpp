#include "traffic/vehicle/lane_queue.hpp"

#include <algorithm>
#include <stdexcept>

namespace traffic {

LaneQueue::LaneQueue(std::size_t capacity) : capacity_(capacity) {
    if (capacity == 0) throw std::invalid_argument("Lane capacity must be positive");
}

bool LaneQueue::enqueue(VehicleId vehicleId) {
    if (vehicleId == 0 || !hasSpace() ||
        std::find(vehicles_.begin(), vehicles_.end(), vehicleId) != vehicles_.end()) return false;
    vehicles_.push_back(vehicleId);
    return true;
}

std::vector<VehicleId> LaneQueue::snapshot() const {
    return {vehicles_.begin(), vehicles_.end()};
}

std::optional<VehicleId> LaneQueue::front() const noexcept {
    if (vehicles_.empty()) return std::nullopt;
    return vehicles_.front();
}
std::optional<VehicleId> LaneQueue::pop() noexcept {
    const auto id = front();
    if (id) vehicles_.pop_front();
    return id;
}
void LaneQueue::clear() noexcept { vehicles_.clear(); }

bool LaneQueue::empty() const noexcept { return vehicles_.empty(); }
bool LaneQueue::hasSpace() const noexcept { return vehicles_.size() < capacity_; }
std::size_t LaneQueue::size() const noexcept { return vehicles_.size(); }
std::size_t LaneQueue::capacity() const noexcept { return capacity_; }

}  // namespace traffic
