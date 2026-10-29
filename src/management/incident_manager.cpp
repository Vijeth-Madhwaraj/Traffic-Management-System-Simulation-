#include "traffic/management/incident_manager.hpp"

#include <algorithm>
#include <stdexcept>

namespace traffic {

void IncidentManager::schedule(Incident incident) {
    if (incident.roadId.empty()) {
        throw std::invalid_argument("Incident road ID cannot be empty");
    }
    if (incident.startsAt >= incident.endsAt) {
        throw std::invalid_argument("Incident startsAt must be strictly less than endsAt");
    }
    incidents_.push_back(std::move(incident));
}

void IncidentManager::advance(Tick now, RoadNetwork& network) {
    std::unordered_set<RoadId> desiredBlocked;

    for (const auto& incident : incidents_) {
        if (incident.startsAt <= now && now < incident.endsAt) {
            desiredBlocked.insert(incident.roadId);
        }
    }

    // 1. Clear expired restrictions
    for (const auto& roadId : currentlyBlockedRoads_) {
        if (desiredBlocked.find(roadId) == desiredBlocked.end()) {
            if (auto* road = network.findRoad(roadId)) {
                const auto origIt = originalRoadState_.find(roadId);
                if (origIt != originalRoadState_.end()) {
                    road->enabled = origIt->second;
                }
            }
            originalRoadState_.erase(roadId);
        }
    }

    // 2. Activate new restrictions and preserve pre-disabled state
    for (const auto& roadId : desiredBlocked) {
        if (auto* road = network.findRoad(roadId)) {
            if (currentlyBlockedRoads_.find(roadId) == currentlyBlockedRoads_.end()) {
                originalRoadState_.emplace(roadId, road->enabled);
                road->enabled = false;
            }
        }
        // Unknown roads are safely skipped without null dereference
    }

    currentlyBlockedRoads_ = std::move(desiredBlocked);
}

void IncidentManager::advance(Tick now, RoadNetwork& network, VehicleManager& vehicles) {
    advance(now, network);
    rerouteAffectedVehicles(network, vehicles);
}

bool IncidentManager::isRoadBlocked(const RoadId& roadId) const noexcept {
    return currentlyBlockedRoads_.find(roadId) != currentlyBlockedRoads_.end();
}

std::vector<RoadId> IncidentManager::activeBlockedRoads() const {
    std::vector<RoadId> result(currentlyBlockedRoads_.begin(), currentlyBlockedRoads_.end());
    std::sort(result.begin(), result.end());
    return result;
}

std::size_t IncidentManager::activeIncidentCount(Tick now) const noexcept {
    std::size_t count = 0;
    for (const auto& incident : incidents_) {
        if (incident.startsAt <= now && now < incident.endsAt) {
            ++count;
        }
    }
    return count;
}

void IncidentManager::clear() noexcept {
    incidents_.clear();
    originalRoadState_.clear();
    currentlyBlockedRoads_.clear();
}

std::size_t IncidentManager::rerouteAffectedVehicles(RoadNetwork& network, VehicleManager& vehicles) {
    std::size_t reroutedCount = 0;
    const std::size_t total = vehicles.size();

    for (VehicleId id = 1; id <= total; ++id) {
        auto* vehicle = vehicles.find(id);
        if (!vehicle) continue;

        if (vehicle->state == VehicleState::Completed || vehicle->state == VehicleState::Unroutable) {
            continue;
        }

        // A vehicle already travelling on a road must finish that road before
        // taking a detour. Only inspect roads after its current road in that case.
        const bool inTransit = vehicle->state == VehicleState::InTransit;
        const std::size_t firstUpcoming = vehicle->routeIndex + (inTransit ? 1U : 0U);
        bool needsReroute = false;
        for (std::size_t idx = firstUpcoming; idx < vehicle->route.size(); ++idx) {
            const auto* road = network.findRoad(vehicle->route[idx]);
            if (!road || !road->enabled) {
                needsReroute = true;
                break;
            }
        }
        if (!needsReroute) continue;

        IntersectionId currentIntersection;
        std::size_t prefixCount = vehicle->routeIndex;
        if (inTransit && vehicle->routeIndex < vehicle->route.size()) {
            const auto* currentRoad = network.findRoad(vehicle->route[vehicle->routeIndex]);
            currentIntersection = currentRoad ? currentRoad->to : vehicle->source;
            prefixCount = vehicle->routeIndex + 1;
        } else if (vehicle->routeIndex == 0) {
            currentIntersection = vehicle->source;
        } else {
            const auto* prevRoad = network.findRoad(vehicle->route[vehicle->routeIndex - 1]);
            currentIntersection = prevRoad ? prevRoad->to : vehicle->source;
        }

        auto newPath = network.shortestRoute(currentIntersection, vehicle->destination);
        if (newPath.empty()) {
            // Keep an in-transit vehicle on its current road; it can become
            // unroutable at the next intersection if no continuation exists.
            if (!inTransit) vehicle->state = VehicleState::Unroutable;
        } else {
            std::vector<RoadId> splicedRoute;
            splicedRoute.reserve(prefixCount + newPath.size());
            for (std::size_t i = 0; i < prefixCount && i < vehicle->route.size(); ++i) {
                splicedRoute.push_back(vehicle->route[i]);
            }
            for (auto&& roadId : newPath) splicedRoute.push_back(std::move(roadId));
            vehicle->route = std::move(splicedRoute);
            ++reroutedCount;
        }
    }

    return reroutedCount;
}

}  // namespace traffic
