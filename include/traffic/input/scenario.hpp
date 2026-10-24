#pragma once

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
};

struct Scenario {
    SimulationConfig config;
    std::vector<IntersectionId> intersections;
    std::vector<Road> roads;
    std::vector<EntryLane> lanes;
    std::vector<ScheduledArrival> arrivals;
    std::vector<ArrivalStream> streams;
    std::vector<ScenarioSignals> signals;
};

class ScenarioLoader {
public:
    // Return a complete validated value; failures never modify a running simulation.
    static Scenario load(std::istream& input);
    static Scenario loadFile(const std::string& path);
    static void validate(const Scenario& scenario);
};

}  // namespace traffic
