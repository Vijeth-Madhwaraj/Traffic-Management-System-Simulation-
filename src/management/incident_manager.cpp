
#include "traffic/management/incident_manager.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace traffic {

void IncidentManager::schedule(Incident incident) {
    if (incident.roadId.empty()) {
        throw std::invalid_argument("Incident road ID cannot be empty");
    }

    if (incident.startsAt >= incident.endsAt) {
        throw std::invalid_argument(
            "Incident startsAt must be strictly less than endsAt");
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

    // Restore roads whose incidents have expired.
    for (const auto& roadId : currentlyBlockedRoads_) {
        if (desiredBlocked.find(roadId) == desiredBlocked.end()) {
            if (auto* road = network.findRoad(roadId)) {
                const auto original = originalRoadState_.find(roadId);
                if (original != originalRoadState_.end()) {
                    road->enabled = original->second;
                }
            }
            originalRoadState_.erase(roadId);
        }
    }

    // Block roads for newly active incidents.
    for (const auto& roadId : desiredBlocked) {
        if (auto* road = network.findRoad(roadId)) {
            if (currentlyBlockedRoads_.find(roadId) ==
                currentlyBlockedRoads_.end()) {
                originalRoadState_.emplace(roadId, road->enabled);
                road->enabled = false;
            }
        }
    }

    currentlyBlockedRoads_ = std::move(desiredBlocked);
}

bool IncidentManager::isRoadBlocked(const RoadId& roadId) const noexcept {
    return currentlyBlockedRoads_.find(roadId) != currentlyBlockedRoads_.end();
}

std::vector<RoadId> IncidentManager::activeBlockedRoads() const {
    std::vector<RoadId> result(
        currentlyBlockedRoads_.begin(), currentlyBlockedRoads_.end());
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

}  // namespace traffic
