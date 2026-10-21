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
    std::vector<RoadId> shortestRoute(const IntersectionId& from, const IntersectionId& to) const;

private:
    std::unordered_map<RoadId, Road> roads_;
};

}  // namespace traffic
