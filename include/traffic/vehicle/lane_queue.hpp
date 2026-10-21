#pragma once

#include "traffic/types.hpp"

#include <cstddef>
#include <deque>

namespace traffic {

class LaneQueue {
public:
    explicit LaneQueue(std::size_t capacity);
    bool enqueue(VehicleId vehicleId);
    bool empty() const noexcept;
    bool hasSpace() const noexcept;
    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;

private:
    std::size_t capacity_;
    std::deque<VehicleId> vehicles_;
};

}  // namespace traffic
