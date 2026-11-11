
#include "traffic/simulation/simulation.hpp"

#include <stdexcept>
#include <utility>

namespace traffic {

Simulation::Simulation(SimulationConfig config)
    : config_(config), vehicles_(config.seed) {}

void Simulation::configureScenario(const Scenario& scenario) {
    ScenarioLoader::validate(scenario);

    Simulation replacement(scenario.config);

    // Configure vehicle topology and arrival schedules.
    replacement.vehicles_.configureTopology(
        scenario.intersections,
        scenario.roads
    );

    replacement.vehicles_.configureLanes(scenario.lanes);
    replacement.vehicles_.setSchedule(scenario.arrivals);
    replacement.vehicles_.setStreams(scenario.streams);

    // Add intersections to the road network.
    for (const auto& intersection : scenario.intersections) {
        if (!replacement.network_.addIntersection(intersection)) {
            throw std::invalid_argument(
                "Invalid scenario intersection"
            );
        }
    }

    // Add roads to the road network.
    for (const auto& road : scenario.roads) {
        if (!replacement.network_.addRoad(road)) {
            throw std::invalid_argument(
                "Invalid scenario road"
            );
        }
    }

    // FR-14: Register scheduled incidents.
    for (const auto& incident : scenario.incidents) {
        replacement.incidents_.schedule(incident);
    }

    // Configure traffic signals, including adaptive policies.
    for (const auto& plan : scenario.signals) {
        replacement.configureSignals(
            plan.intersection,
            plan.phases,
            plan.conflicts,
            plan.adaptivePolicy
        );
    }

    replacement.scenario_ = scenario;

    // Replace current simulation only after successful configuration.
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
    const std::vector<MovementConflict>& conflicts,
    std::optional<AdaptiveSignalPolicy> adaptivePolicy
) {
    if (currentTick_ != 0) {
        throw std::logic_error(
            "Configure signals before starting simulation"
        );
    }

    signals_.configure(
        intersection,
        std::move(phases),
        conflicts,
        adaptivePolicy
    );
}

bool Simulation::requestPriority(const IntersectionId& intersection, const MovementId& movement) {
    return scenario_ && signals_.requestPriority(intersection, movement);
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

    // FR-14: Apply incidents before calculating routes.
    incidents_.advance(currentTick_, network_);

    // FR-15: Use injected routing or shortest-path routing
    // that avoids disabled roads.
    const RouteProvider routes = routeProvider_
        ? *routeProvider_
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
        routes,
        routeProvider_ ? std::function<std::vector<RoadId>(const EntryLane&, const Vehicle&)>{}
                       : [this](const EntryLane& lane, const Vehicle& vehicle) {
            const auto* first = network_.findRoad(lane.road);
            if (!first || !first->enabled) return std::vector<RoadId>{};
            auto suffix = network_.shortestRoute(first->to, vehicle.destination);
            if (first->to != vehicle.destination && suffix.empty()) return suffix;
            suffix.insert(suffix.begin(), first->id);
            return suffix;
        }
    );

    admitted_ += arrivals.admitted.size();
    unroutable_ += arrivals.unroutable.size();

    // FR-16: Record generated vehicles and retain their IDs.
    for (const auto vehicleId : arrivals.generated) {
        trackedVehicles_.push_back(vehicleId);
        metrics_.recordGenerated();
    }

    // FR-13: Evaluate road and lane congestion.
    lastCongestionReport_ = congestionDetector_.evaluate(
        network_,
        vehicles_,
        scenario_->lanes
    );

    // Master feature: Update signals using movement-aware queues.
    const auto movementQueues = movement_.readyMovementQueues(
        currentTick_,
        network_,
        vehicles_
    );

    movement_.requestEmergencyPriority(currentTick_, network_, signals_, vehicles_);
    signals_.advance(currentTick_, movementQueues);

    // Advance vehicle movement.
    movement_.advance(
        currentTick_,
        network_,
        signals_,
        vehicles_
    );

    // FR-16: Record completed vehicles and waiting times.
    for (const auto vehicleId : trackedVehicles_) {
        const Vehicle* vehicle = vehicles_.find(vehicleId);

        if (vehicle != nullptr &&
            vehicle->state == VehicleState::Completed &&
            recordedCompletions_.insert(vehicleId).second) {

            metrics_.recordCompleted(vehicle->waitingTicks);
        }
    }

    // FR-16: Record queue lengths safely.
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

// FR-16: Return simulation performance metrics.
MetricsSnapshot Simulation::metricsSnapshot() const noexcept {
    const auto summary = inputSummary();

    const auto completed = metrics_.snapshot(0).completed;

    const auto unfinished =
        summary.generated > completed
            ? summary.generated - completed
            : 0;

    return metrics_.snapshot(unfinished);
}

// FR-17: Prepare the final simulation report.
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

// Master feature: Return signal status snapshots.
std::vector<std::pair<IntersectionId, SignalStatus>>
Simulation::signalSnapshots(Tick tick) const {
    std::vector<std::pair<IntersectionId, SignalStatus>> result;

    if (!scenario_) {
        return result;
    }

    result.reserve(scenario_->signals.size());

    for (const auto& plan : scenario_->signals) {
        const auto status = signals_.status(
            plan.intersection,
            tick
        );

        if (status) {
            result.emplace_back(plan.intersection, *status);
        }
    }

    return result;
}

}  // namespace traffic
