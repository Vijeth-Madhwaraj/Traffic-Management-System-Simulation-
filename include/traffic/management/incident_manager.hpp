
#pragma once

#include "traffic/network/road_network.hpp"
#include "traffic/types.hpp"

#include <cstddef>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace traffic {

struct Incident {
    RoadId roadId;
    Tick startsAt{};
    Tick endsAt{};
};

class IncidentManager {
public:
    // Schedule an incident that blocks a road during a time interval.
    void schedule(Incident incident);

    // Activate or clear road blockages for the current simulation tick.
    void advance(Tick now, RoadNetwork& network);

    // Query currently active road blockages.
    bool isRoadBlocked(const RoadId& roadId) const noexcept;
    std::vector<RoadId> activeBlockedRoads() const;

    // Count incidents active at the specified tick.
    std::size_t activeIncidentCount(Tick now) const noexcept;

    // Read scheduled incidents.
    const std::vector<Incident>& incidents() const noexcept {
        return incidents_;
    }

    // Restore affected roads and clear all scheduled and active incidents.
    void clear(RoadNetwork& network);

private:
    std::vector<Incident> incidents_;

    // Preserve each road's original enabled state for restoration.
    std::unordered_map<RoadId, bool> originalRoadState_;

    // Roads currently blocked by active incidents.
    std::unordered_set<RoadId> currentlyBlockedRoads_;
};

}  // namespace traffic
