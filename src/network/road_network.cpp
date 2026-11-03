
#include "traffic/network/road_network.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

namespace traffic {

bool RoadNetwork::addIntersection(const IntersectionId& id) {
    return !id.empty() &&
           adjacency_.emplace(id, std::vector<RoadId>{}).second;
}

bool RoadNetwork::hasIntersection(const IntersectionId& id) const noexcept {
    return adjacency_.count(id) != 0;
}

bool RoadNetwork::addRoad(Road road) {
    if (road.id.empty() ||
        !hasIntersection(road.from) ||
        !hasIntersection(road.to) ||
        road.capacity == 0 ||
        road.travelTime == 0 ||
        road.occupancy > road.capacity) {
        return false;
    }

    const auto id = road.id;
    const auto from = road.from;

    const auto inserted = roads_.emplace(id, std::move(road));

    if (!inserted.second) {
        return false;
    }

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

std::vector<RoadId> RoadNetwork::neighbors(
    const IntersectionId& intersection) const {

    const auto it = adjacency_.find(intersection);

    return it == adjacency_.end()
        ? std::vector<RoadId>{}
        : it->second;
}

std::vector<RoadId> RoadNetwork::shortestRoute(
    const IntersectionId& from,
    const IntersectionId& to) const {

    // Check whether both intersections exist.
    if (!hasIntersection(from) || !hasIntersection(to)) {
        return {};
    }

    // No roads are required when source and destination are the same.
    if (from == to) {
        return {};
    }

    using QueueEntry = std::pair<Tick, IntersectionId>;

    // Min-priority queue: smallest travel time is processed first.
    std::priority_queue<
        QueueEntry,
        std::vector<QueueEntry>,
        std::greater<QueueEntry>
    > queue;

    std::unordered_map<IntersectionId, Tick> distances;
    std::unordered_map<IntersectionId, RoadId> previousRoad;

    // Initialize every intersection with infinite distance.
    for (const auto& intersection : adjacency_) {
        distances[intersection.first] =
            std::numeric_limits<Tick>::max();
    }

    // Starting intersection has distance zero.
    distances[from] = 0;
    queue.push({0, from});

    // Dijkstra's shortest-path algorithm.
    while (!queue.empty()) {
        const auto current = queue.top();
        queue.pop();

        const Tick currentDistance = current.first;
        const IntersectionId currentIntersection = current.second;

        // Ignore outdated queue entries.
        if (currentDistance != distances[currentIntersection]) {
            continue;
        }

        // Destination reached.
        if (currentIntersection == to) {
            break;
        }

        // Examine outgoing roads.
        for (const auto& roadId : neighbors(currentIntersection)) {
            const Road* road = findRoad(roadId);

            // Ignore missing or disabled roads.
            if (road == nullptr || !road->enabled) {
                continue;
            }

            // Prevent integer overflow.
            if (currentDistance >
                std::numeric_limits<Tick>::max() - road->travelTime) {
                continue;
            }

            const Tick newDistance =
                currentDistance + road->travelTime;

            // Update route if a shorter path is found.
            if (newDistance < distances[road->to]) {
                distances[road->to] = newDistance;
                previousRoad[road->to] = roadId;

                queue.push({newDistance, road->to});
            }
        }
    }

    // Destination cannot be reached.
    if (distances[to] == std::numeric_limits<Tick>::max()) {
        return {};
    }

    // Reconstruct the route from destination to source.
    std::vector<RoadId> route;
    IntersectionId current = to;

    while (current != from) {
        const auto previous = previousRoad.find(current);

        if (previous == previousRoad.end()) {
            return {};
        }

        const RoadId roadId = previous->second;
        route.push_back(roadId);

        const Road* road = findRoad(roadId);

        if (road == nullptr) {
            return {};
        }

        current = road->from;
    }

    // Reverse to get source-to-destination order.
    std::reverse(route.begin(), route.end());

    return route;
}

}  // namespace traffic
