#pragma once

#include "traffic/management/incident_manager.hpp"
#include "traffic/management/metrics_collector.hpp"
#include "traffic/network/movement_engine.hpp"
#include "traffic/types.hpp"

namespace traffic {

class Simulation {
public:
    explicit Simulation(SimulationConfig config);
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
