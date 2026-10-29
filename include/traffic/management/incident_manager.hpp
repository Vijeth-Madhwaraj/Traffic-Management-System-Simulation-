#pragma once

#include "traffic/network/road_network.hpp"
#include "traffic/types.hpp"
#include "traffic/vehicle/vehicle_manager.hpp"

#include <cstddef>
#include <string>
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
    void schedule(Incident incident);
    void advance(Tick now, RoadNetwork& network);
    void advance(Tick now, RoadNetwork& network, VehicleManager& vehicles);

    bool isRoadBlocked(const RoadId& roadId) const noexcept;
    std::vector<RoadId> activeBlockedRoads() const;
    std::size_t activeIncidentCount(Tick now) const noexcept;
    const std::vector<Incident>& incidents() const noexcept { return incidents_; }
    void clear() noexcept;

    std::size_t rerouteAffectedVehicles(RoadNetwork& network, VehicleManager& vehicles);

private:
    std::vector<Incident> incidents_;
    std::unordered_map<RoadId, bool> originalRoadState_;
    std::unordered_set<RoadId> currentlyBlockedRoads_;
};

}  // namespace traffic
