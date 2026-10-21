#include "traffic/network/road_network.hpp"

namespace traffic {

bool RoadNetwork::addRoad(Road road) {
    if (road.id.empty() || road.from.empty() || road.to.empty() || road.capacity == 0 || road.travelTime == 0) return false;
    return roads_.emplace(road.id, std::move(road)).second;
}

const Road* RoadNetwork::findRoad(const RoadId& id) const noexcept {
    const auto it = roads_.find(id);
    return it == roads_.end() ? nullptr : &it->second;
}

Road* RoadNetwork::findRoad(const RoadId& id) noexcept {
    const auto it = roads_.find(id);
    return it == roads_.end() ? nullptr : &it->second;
}

std::vector<RoadId> RoadNetwork::shortestRoute(const IntersectionId&, const IntersectionId&) const {
    // TODO: implement weighted shortest path over enabled roads; return empty
    // when the destination is unreachable.
    return {};
}

}  // namespace traffic
