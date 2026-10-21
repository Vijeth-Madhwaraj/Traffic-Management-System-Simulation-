#include "traffic/vehicle/lane_queue.hpp"

namespace traffic {

LaneQueue::LaneQueue(std::size_t capacity) : capacity_(capacity) {}

bool LaneQueue::enqueue(VehicleId vehicleId) {
    if (!hasSpace()) return false;
    vehicles_.push_back(vehicleId);
    return true;
}

bool LaneQueue::empty() const noexcept { return vehicles_.empty(); }
bool LaneQueue::hasSpace() const noexcept { return vehicles_.size() < capacity_; }
std::size_t LaneQueue::size() const noexcept { return vehicles_.size(); }
std::size_t LaneQueue::capacity() const noexcept { return capacity_; }

}  // namespace traffic
