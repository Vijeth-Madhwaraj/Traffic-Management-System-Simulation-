
#include "traffic/simulation/simulation.hpp"

#include <stdexcept>
#include <utility>

namespace traffic {

Simulation::Simulation(SimulationConfig config)
    : config_(config), vehicles_(config.seed) {}

void Simulation::configureScenario(const Scenario& scenario) {
    ScenarioLoader::validate(scenario);

    Simulation replacement(scenario.config);

    replacement.vehicles_.configureTopology(
        scenario.intersections,
        scenario.roads
    );

    replacement.vehicles_.configureLanes(scenario.lanes);
    replacement.vehicles_.setSchedule(scenario.arrivals);
    replacement.vehicles_.setStreams(scenario.streams);

    for (const auto& intersection : scenario.intersections) {
        if (!replacement.network_.addIntersection(intersection)) {
            throw std::invalid_argument("Invalid scenario intersection");
        }
    }

    for (const auto& road : scenario.roads) {
        if (!replacement.network_.addRoad(road)) {
            throw std::invalid_argument("Invalid scenario road");
        }
    }

    for (const auto& plan : scenario.signals) {
        replacement.configureSignals(
            plan.intersection,
            plan.phases,
            plan.conflicts
        );
    }

    replacement.scenario_ = scenario;
    *this = std::move(replacement);
}

void Simulation::setRouteProvider(RouteProvider provider) {
    if (currentTick_ != 0) {
        throw std::logic_error(
            "Set route provider before starting simulation"
        );
    }

    routeProvider_ = std::move(provider);
}

void Simulation::configureSignals(
    const IntersectionId& intersection,
    std::vector<SignalPhase> phases,
    const std::vector<MovementConflict>& conflicts
) {
    if (currentTick_ != 0) {
        throw std::logic_error(
            "Configure signals before starting simulation"
        );
    }

    signals_.configure(
        intersection,
        std::move(phases),
        conflicts
    );
}

AdmissionBatch Simulation::step() {
    if (!scenario_) {
        throw std::logic_error(
            "Load a scenario before running simulation"
        );
    }

    if (currentTick_ >= config_.duration) {
        return {};
    }

    // FR-14: Apply scheduled incidents.
    incidents_.advance(currentTick_, network_);

    // FR-15: Use an injected route provider or production shortest-path routing.
    const RouteProvider routes = routeProvider_
        ? routeProvider_
        : RouteProvider{
              [this](
                  const IntersectionId& source,
                  const IntersectionId& destination
              ) {
                  return network_.shortestRoute(
                      source,
                      destination
                  );
              }
          };

    // Generate and admit vehicles.
    auto arrivals = vehicles_.advanceArrivals(
        currentTick_,
        routes
    );

    admitted_ += arrivals.admitted.size();
    unroutable_ += arrivals.unroutable.size();

    // FR-16: Record generated vehicles.
    for (std::size_t index = 0;
         index < arrivals.generated.size();
         ++index) {
        metrics_.recordGenerated();
    }

    // FR-13: Evaluate congestion.
    lastCongestionReport_ = congestionDetector_.evaluate(
        network_,
        vehicles_,
        scenario_->lanes
    );

    // Advance traffic signals and movement engine.
    signals_.advance(currentTick_);

    movement_.advance(
        currentTick_,
        network_,
        signals_,
        vehicles_
    );

    // FR-16: Record queue lengths after processing this tick.
    for (const auto& lane : scenario_->lanes) {
        const auto* queue = vehicles_.lane(lane.id);

        if (queue != nullptr) {
            metrics_.observeQueue(queue->size());
        }
    }

    ++currentTick_;

    return arrivals;
}

void Simulation::run() {
    if (!scenario_) {
        throw std::logic_error(
            "Load a scenario before running simulation"
        );
    }

    while (currentTick_ < config_.duration) {
        step();
    }
}

void Simulation::reset() {
    if (!scenario_) {
        throw std::logic_error(
            "Load a scenario before resetting simulation"
        );
    }

    const auto scenario = *scenario_;
    auto provider = routeProvider_;

    configureScenario(scenario);

    routeProvider_ = std::move(provider);
}

InputSummary Simulation::inputSummary() const noexcept {
    return {
        vehicles_.size(),
        admitted_,
        vehicles_.queuedCount(),
        vehicles_.pendingCount(),
        unroutable_
    };
}

const VehicleManager& Simulation::vehicles() const noexcept {
    return vehicles_;
}

Tick Simulation::currentTick() const noexcept {
    return currentTick_;
}

MetricsSnapshot Simulation::metricsSnapshot() const noexcept {
    const auto summary = inputSummary();

    const auto completed = metrics_.snapshot(0).completed;

    const auto unfinished =
        summary.generated > completed
            ? summary.generated - completed
            : 0;

    return metrics_.snapshot(unfinished);
}

SimulationReport Simulation::createReport() const noexcept {
    const auto summary = inputSummary();

    const auto metrics = metricsSnapshot();

    const Tick finalProcessedTick =
        currentTick_ == 0 ? 0 : currentTick_ - 1;

    SimulationReport report;

    report.finalTick = currentTick_;
    report.generated = summary.generated;
    report.admitted = summary.admitted;
    report.queued = summary.queued;
    report.pending = summary.pending;
    report.unroutable = summary.unroutable;

    report.metrics = metrics;

    report.congestedRoads =
        lastCongestionReport_.congestedRoads.size();

    report.congestedLanes =
        lastCongestionReport_.congestedLanes.size();

    report.activeIncidents =
        currentTick_ == 0
            ? 0
            : incidents_.activeIncidentCount(finalProcessedTick);

    return report;
}

}  // namespace traffic
