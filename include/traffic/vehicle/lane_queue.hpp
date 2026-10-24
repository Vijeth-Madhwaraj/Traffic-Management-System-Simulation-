#pragma once

#include "traffic/types.hpp"

#include <cstddef>
#include <deque>
#include <optional>

namespace traffic {

class LaneQueue {
public:
    explicit LaneQueue(std::size_t capacity);
    bool enqueue(VehicleId vehicleId);
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
