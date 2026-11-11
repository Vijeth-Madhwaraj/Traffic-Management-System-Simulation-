
#pragma once

#include "traffic/input/scenario.hpp"
#include "traffic/management/congestion_detector.hpp"
#include "traffic/management/incident_manager.hpp"
#include "traffic/management/metrics_collector.hpp"
#include "traffic/management/report_writer.hpp"
#include "traffic/network/movement_engine.hpp"
#include "traffic/network/road_network.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/types.hpp"
#include "traffic/vehicle/vehicle_manager.hpp"

#include <cstddef>
#include <optional>
#include <unordered_set>
#include <utility>
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

    bool requestPriority(const IntersectionId& intersection, const MovementId& movement);
    AdmissionBatch step();
    void run();
    void reset();

    InputSummary inputSummary() const noexcept;

    const VehicleManager& vehicles() const noexcept;

    Tick currentTick() const noexcept;

    std::vector<std::pair<IntersectionId, SignalStatus>>
    signalSnapshots(Tick tick) const;

    void configureSignals(
        const IntersectionId& intersection,
        std::vector<SignalPhase> phases,
        const std::vector<MovementConflict>& conflicts,
        std::optional<AdaptiveSignalPolicy> adaptivePolicy = std::nullopt
    );

    // FR-16: Performance metrics.
    MetricsSnapshot metricsSnapshot() const noexcept;

    // FR-17: Report generation.
    SimulationReport createReport() const noexcept;

private:
    void respondToTraffic();
    SimulationConfig config_;

    // Scenario and simulation state.
    std::optional<Scenario> scenario_;
    Tick currentTick_{};

    // Core simulation components.
    RoadNetwork network_;
    VehicleManager vehicles_;
    SignalController signals_;
    MovementEngine movement_;

    // Routing.
    std::optional<RouteProvider> routeProvider_;

    // Admission statistics.
    std::size_t admitted_{};
    std::size_t unroutable_{};

    // FR-13: Congestion detection.
    CongestionDetector congestionDetector_;
    CongestionReport lastCongestionReport_;

    // FR-14: Incident scheduling and road blockages.
    IncidentManager incidents_;

    // FR-16: Performance metrics.
    MetricsCollector metrics_;

    // Track vehicles for completion metrics.
    std::vector<VehicleId> trackedVehicles_;
    std::unordered_set<VehicleId> recordedCompletions_;
};

}  // namespace traffic