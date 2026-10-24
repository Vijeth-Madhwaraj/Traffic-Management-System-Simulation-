#pragma once

#include "traffic/input/scenario.hpp"
#include "traffic/management/incident_manager.hpp"
#include "traffic/management/metrics_collector.hpp"
#include "traffic/network/movement_engine.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/types.hpp"

namespace traffic {

struct InputSummary {
    std::size_t generated{};
    std::size_t admitted{}; // cumulative admissions, not completed trips
    std::size_t queued{};
    std::size_t pending{};
    std::size_t unroutable{};
};

class Simulation {
public:
    explicit Simulation(SimulationConfig config);
    void configureScenario(const Scenario& scenario);
    void setRouteProvider(RouteProvider provider);
    AdmissionBatch step();
    void reset();
    InputSummary inputSummary() const noexcept;
    const VehicleManager& vehicles() const noexcept;
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
    std::optional<Scenario> scenario_;
    RouteProvider routeProvider_;
    std::size_t admitted_{};
    std::size_t unroutable_{};
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
