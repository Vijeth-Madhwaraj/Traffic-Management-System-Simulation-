#pragma once

#include "traffic/management/incident_manager.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/vehicle/vehicle_manager.hpp"

#include <istream>
#include <string>
#include <vector>

namespace traffic {

struct ScenarioSignals {
    IntersectionId intersection;
    std::vector<SignalPhase> phases;
    std::vector<MovementConflict> conflicts;
    std::optional<AdaptiveSignalPolicy> adaptivePolicy;
};

struct Scenario {
    SimulationConfig config;

    std::vector<IntersectionId> intersections;
    std::vector<Road> roads;
    std::vector<EntryLane> lanes;

    std::vector<ScheduledArrival> arrivals;
    std::vector<ArrivalStream> streams;

    std::vector<ScenarioSignals> signals;

    // FR-14: Scheduled road incidents.
    std::vector<Incident> incidents;
};

class ScenarioLoader {
public:
    static Scenario load(std::istream& input);
    static Scenario loadFile(const std::string& path);
    static void validate(const Scenario& scenario);
};

}  // namespace traffic