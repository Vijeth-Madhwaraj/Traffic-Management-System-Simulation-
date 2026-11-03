#pragma once

#include "traffic/input/scenario.hpp"
#include "traffic/management/congestion_detector.hpp"
#include "traffic/management/incident_manager.hpp"
#include "traffic/management/metrics_collector.hpp"
#include "traffic/management/report_writer.hpp"
#include "traffic/network/movement_engine.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/types.hpp"

#include <optional>
#include <unordered_set>
#include <vector>

namespace traffic {

struct InputSummary {
    std::size_t generated{};
    std::size_t admitted{};
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

    void configureSignals(
        const IntersectionId& intersection,
        std::vector<SignalPhase> phases,
        const std::vector<MovementConflict>& conflicts
    );

    void run();
    Tick currentTick() const noexcept;

    // FR-16
    MetricsSnapshot metricsSnapshot() const noexcept;

    // FR-17
    SimulationReport createReport() const noexcept;

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

    // FR-13
    CongestionDetector congestionDetector_;
    CongestionReport lastCongestionReport_;

    // FR-14
    IncidentManager incidents_;

    // FR-16
    MetricsCollector metrics_;

    // Track vehicles for completion metrics.
    std::vector<VehicleId> trackedVehicles_;
    std::unordered_set<VehicleId> recordedCompletions_;
};

}  // namespace traffic