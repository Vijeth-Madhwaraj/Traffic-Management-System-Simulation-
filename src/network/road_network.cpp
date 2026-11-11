
#include "traffic/network/road_network.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

namespace traffic {

bool RoadNetwork::addIntersection(const IntersectionId &id) {
    return !id.empty() && adjacency_.emplace(id, std::vector<RoadId>{}).second;
}

bool RoadNetwork::hasIntersection(const IntersectionId &id) const noexcept {
    return adjacency_.count(id) != 0;
}

bool RoadNetwork::addRoad(Road road) {
    if (road.id.empty() || !hasIntersection(road.from) || !hasIntersection(road.to) ||
        road.capacity == 0 || road.travelTime == 0 || road.occupancy > road.capacity) {
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

const Road *RoadNetwork::findRoad(const RoadId &id) const noexcept {
    const auto it = roads_.find(id);
    return it == roads_.end() ? nullptr : &it->second;
}

Road *RoadNetwork::findRoad(const RoadId &id) noexcept {
    const auto it = roads_.find(id);
    return it == roads_.end() ? nullptr : &it->second;
}

std::vector<RoadId> RoadNetwork::neighbors(const IntersectionId &intersection) const {

    const auto it = adjacency_.find(intersection);

    return it == adjacency_.end() ? std::vector<RoadId>{} : it->second;
}

std::vector<RoadId> RoadNetwork::shortestRoute(const IntersectionId &from, const IntersectionId &to,
                                               const std::vector<RoadId> &excluded) const {
    if (!hasIntersection(from) || !hasIntersection(to) || from == to)
        return {};

    struct RouteLabel {
        Tick cost{};
        std::vector<RoadId> roads;
    };
    struct QueueEntry {
        IntersectionId intersection;
        RouteLabel label;
    };
    const auto comesAfter = [](const QueueEntry &left, const QueueEntry &right) {
        if (left.label.cost != right.label.cost)
            return left.label.cost > right.label.cost;
        if (left.label.roads != right.label.roads)
            return left.label.roads > right.label.roads;
        return left.intersection > right.intersection;
    };

    std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(comesAfter)> pending(
        comesAfter);
    std::unordered_map<IntersectionId, RouteLabel> best;
    best.emplace(from, RouteLabel{});
    pending.push({from, {}});

    while (!pending.empty()) {
        auto current = pending.top();
        pending.pop();
        const auto known = best.find(current.intersection);
        if (known == best.end() || known->second.cost != current.label.cost ||
            known->second.roads != current.label.roads)
            continue;
        if (current.intersection == to)
            return current.label.roads;

        const auto outgoing = adjacency_.find(current.intersection);
        if (outgoing == adjacency_.end())
            continue;
        for (const auto &roadId : outgoing->second) {
            const auto roadIt = roads_.find(roadId);
            if (std::find(excluded.begin(), excluded.end(), roadId) != excluded.end() ||
                roadIt == roads_.end() || !roadIt->second.enabled ||
                roadIt->second.travelTime == 0 ||
                current.label.cost > std::numeric_limits<Tick>::max() - roadIt->second.travelTime)
                continue;

            RouteLabel candidate = current.label;
            candidate.cost += roadIt->second.travelTime;
            candidate.roads.push_back(roadId);
            const auto previous = best.find(roadIt->second.to);
            const bool improves = previous == best.end() ||
                                  candidate.cost < previous->second.cost ||
                                  (candidate.cost == previous->second.cost &&
                                   candidate.roads < previous->second.roads);
            if (!improves)
                continue;
            best[roadIt->second.to] = candidate;
            pending.push({roadIt->second.to, std::move(candidate)});
        }
    }

    return {};
}

} // namespace traffic
