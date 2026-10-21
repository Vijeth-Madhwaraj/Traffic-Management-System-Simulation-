#include "traffic/management/incident_manager.hpp"

namespace traffic {

void IncidentManager::schedule(Incident incident) { incidents_.push_back(std::move(incident)); }

void IncidentManager::advance(Tick now, RoadNetwork& network) {
    // TODO: activate and clear scheduled restrictions, then request rerouting.
    (void)now;
    (void)network;
}

}  // namespace traffic
