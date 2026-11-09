#pragma once

#include "traffic/types.hpp"

#include <cstddef>
#include <deque>
#include <optional>
#include <vector>

namespace traffic {

class LaneQueue {
public:
    explicit LaneQueue(std::size_t capacity);
    // Reject zero IDs, IDs already in this queue, and arrivals beyond capacity.
    bool enqueue(VehicleId vehicleId);
    std::vector<VehicleId> snapshot() const;
    std::optional<VehicleId> front() const noexcept;
    std::optional<VehicleId> pop() noexcept;
    void clear() noexcept;
    bool empty() const noexcept;
    bool hasSpace() const noexcept;
    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;

private:
    std::size_t capacity_;
    std::deque<VehicleId> vehicles_;
};

}  // namespace traffic
