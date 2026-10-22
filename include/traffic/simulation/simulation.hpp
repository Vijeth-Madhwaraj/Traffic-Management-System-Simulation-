#pragma once

#include "traffic/management/incident_manager.hpp"
#include "traffic/management/metrics_collector.hpp"
#include "traffic/network/movement_engine.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/types.hpp"

namespace traffic {

class Simulation {
public:
    explicit Simulation(SimulationConfig config);
    // Scenario/configuration owner uses this before run() to install the
    // intersection's validated phases and conflict table.
    void configureSignals(const IntersectionId& intersection,
                         std::vector<SignalPhase> phases,
                         const std::vector<MovementConflict>& conflicts);
    // Integration dependency (FR-06): configure signals before run(), then
    // advance signals once per monotonically increasing tick before movement.
    void run();
    Tick currentTick() const noexcept;

private:
    SimulationConfig config_;
    Tick currentTick_{};
    VehicleManager vehicles_;
    RoadNetwork network_;
    SignalController signals_;
    MovementEngine movement_;
    IncidentManager incidents_;
    MetricsCollector metrics_;
};

}  // namespace traffic
