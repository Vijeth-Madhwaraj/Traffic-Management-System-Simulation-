
#pragma once

#include "traffic/types.hpp"

#include <optional>
#include <unordered_map>
#include <vector>

namespace traffic {

class RoadNetwork {
public:
    bool addRoad(Road road);

    const Road* findRoad(const RoadId& id) const noexcept;
    Road* findRoad(const RoadId& id) noexcept;

    std::vector<RoadId> shortestRoute(
        const IntersectionId& from,
        const IntersectionId& to) const;

    // Read-only access to all roads for congestion detection.
    const std::unordered_map<RoadId, Road>& roads() const noexcept {
        return roads_;
    }

private:
    std::unordered_map<RoadId, Road> roads_;
};

}  // namespace traffic
