#include "traffic/network/road_network.hpp"

#include <algorithm>
#include <limits>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <utility>

namespace traffic {

bool RoadNetwork::addRoad(Road road) {
    if (road.id.empty() || road.from.empty() || road.to.empty() || road.capacity == 0 || road.travelTime == 0) {
        return false;
    }
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

std::vector<RoadId> RoadNetwork::allRoadIds() const {
    std::vector<RoadId> ids;
    ids.reserve(roads_.size());
    for (const auto& [id, _] : roads_) {
        ids.push_back(id);
    }
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::vector<RoadId> RoadNetwork::shortestRoute(const IntersectionId& from, const IntersectionId& to) const {
    if (from.empty() || to.empty() || from == to) {
        return {};
    }

    // Build outgoing adjacency list of enabled roads
    // from_node -> list of (to_node, travelTime, roadId)
    std::unordered_map<IntersectionId, std::vector<const Road*>> adj;
    bool foundSource = false;
    bool foundDest = false;

    for (const auto& [_, road] : roads_) {
        if (road.from == from) foundSource = true;
        if (road.to == to || road.from == to) foundDest = true;
        if (road.enabled) {
            adj[road.from].push_back(&road);
        }
    }

    if (!foundSource || !foundDest) {
        return {};
    }

    // Sort outgoing edges by road ID for deterministic tie-breaking
    for (auto& [_, edgeList] : adj) {
        std::sort(edgeList.begin(), edgeList.end(), [](const Road* a, const Road* b) {
            return a->id < b->id;
        });
    }

    // Min-heap for Dijkstra: (cost, intersectionId)
    using Element = std::pair<Tick, IntersectionId>;
    std::priority_queue<Element, std::vector<Element>, std::greater<Element>> pq;

    std::unordered_map<IntersectionId, Tick> dist;
    // Map node -> (predecessor_node, roadId used)
    std::unordered_map<IntersectionId, std::pair<IntersectionId, RoadId>> parent;

    dist[from] = 0;
    pq.emplace(0, from);

    while (!pq.empty()) {
        const auto [currentDist, u] = pq.top();
        pq.pop();

        if (currentDist > dist[u]) {
            continue;
        }

        if (u == to) {
            break;
        }

        const auto it = adj.find(u);
        if (it == adj.end()) {
            continue;
        }

        for (const auto* edge : it->second) {
            const auto& v = edge->to;
            const Tick edgeWeight = edge->travelTime > 0 ? edge->travelTime : 1;
            if (currentDist > std::numeric_limits<Tick>::max() - edgeWeight) {
                continue; // Ignore paths whose cost would overflow Tick.
            }
            const Tick newDist = currentDist + edgeWeight;

            const auto distIt = dist.find(v);
            if (distIt == dist.end() || newDist < distIt->second) {
                dist[v] = newDist;
                parent[v] = {u, edge->id};
                pq.emplace(newDist, v);
            }
        }
    }

    if (dist.find(to) == dist.end()) {
        // Unreachable destination
        return {};
    }

    // Reconstruct path
    std::vector<RoadId> path;
    IntersectionId current = to;

    while (current != from) {
        const auto parentIt = parent.find(current);
        if (parentIt == parent.end()) {
            // Disconnected or invalid predecessor state
            return {};
        }
        path.push_back(parentIt->second.second);
        current = parentIt->second.first;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

}  // namespace traffic
