#include "traffic/network/road_network.hpp"

namespace traffic {

bool RoadNetwork::addIntersection(const IntersectionId& id) {
    return !id.empty() && adjacency_.emplace(id, std::vector<RoadId>{}).second;
}

bool RoadNetwork::hasIntersection(const IntersectionId& id) const noexcept {
    return adjacency_.count(id) != 0;
}

bool RoadNetwork::addRoad(Road road) {
    if (road.id.empty() || !hasIntersection(road.from) || !hasIntersection(road.to) ||
        road.capacity == 0 || road.travelTime == 0 || road.occupancy > road.capacity) return false;
    const auto id = road.id;
    const auto from = road.from;
    const auto inserted = roads_.emplace(id, std::move(road));
    if (!inserted.second) return false;
    try {
        adjacency_.at(from).push_back(id);
    } catch (...) {
        roads_.erase(inserted.first);
        throw;
    }
    return true;
}

const Road* RoadNetwork::findRoad(const RoadId& id) const noexcept {
    const auto it = roads_.find(id);
    return it == roads_.end() ? nullptr : &it->second;
}

Road* RoadNetwork::findRoad(const RoadId& id) noexcept {
    const auto it = roads_.find(id);
    return it == roads_.end() ? nullptr : &it->second;
}

std::vector<RoadId> RoadNetwork::neighbors(const IntersectionId& intersection) const {
    const auto it = adjacency_.find(intersection);
    return it == adjacency_.end() ? std::vector<RoadId>{} : it->second;
}

std::vector<RoadId> RoadNetwork::shortestRoute(const IntersectionId&, const IntersectionId&) const {
    // TODO: implement weighted shortest path over enabled roads; return empty
    // when the destination is unreachable.
    return {};
}

}  // namespace traffic
