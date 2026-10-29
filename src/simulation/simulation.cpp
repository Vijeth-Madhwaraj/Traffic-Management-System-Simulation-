#include "traffic/simulation/simulation.hpp"

#include <stdexcept>
#include <utility>

namespace traffic {

Simulation::Simulation(SimulationConfig config) : config_(config), vehicles_(config.seed) {}

void Simulation::configureScenario(const Scenario& scenario) {
    ScenarioLoader::validate(scenario);
    Simulation replacement(scenario.config);
    replacement.vehicles_.configureTopology(scenario.intersections, scenario.roads);
    replacement.vehicles_.configureLanes(scenario.lanes);
    replacement.vehicles_.setSchedule(scenario.arrivals);
    replacement.vehicles_.setStreams(scenario.streams);
    for (const auto& road : scenario.roads) {
        if (!replacement.network_.addRoad(road)) throw std::invalid_argument("Invalid scenario road");
    }
    for (const auto& plan : scenario.signals)
        replacement.configureSignals(plan.intersection, plan.phases, plan.conflicts);
    replacement.scenario_ = scenario;
    *this = std::move(replacement);
}

void Simulation::scheduleIncident(Incident incident) {
    if (currentTick_ != 0) throw std::logic_error("Schedule incidents before starting simulation");
    incidents_.schedule(std::move(incident));
}

void Simulation::setRouteProvider(RouteProvider provider) {
    if (currentTick_ != 0) throw std::logic_error("Set route provider before starting simulation");
    routeProvider_ = std::move(provider);
}

void Simulation::configureSignals(const IntersectionId& intersection,
                                  std::vector<SignalPhase> phases,
                                  const std::vector<MovementConflict>& conflicts) {
    if (currentTick_ != 0) throw std::logic_error("Configure signals before starting simulation");
    signals_.configure(intersection, std::move(phases), conflicts);
}

AdmissionBatch Simulation::step() {
    if (!scenario_) throw std::logic_error("Load a scenario before running simulation");
    if (currentTick_ >= config_.duration) return {};
    incidents_.advance(currentTick_, network_, vehicles_);
    const RouteProvider routes = routeProvider_ ? routeProvider_ : RouteProvider{
        [this](const IntersectionId& source, const IntersectionId& destination) {
            return network_.shortestRoute(source, destination);
        }};
    auto arrivals = vehicles_.advanceArrivals(currentTick_, routes);
    admitted_ += arrivals.admitted.size();
    unroutable_ += arrivals.unroutable.size();
    for (std::size_t index = 0; index < arrivals.generated.size(); ++index) metrics_.recordGenerated();
    // FR-13: measure congestion before signal advancement, as specified by the tick design.
    lastCongestionReport_ = congestionDetector_.evaluate(network_, vehicles_, scenario_->lanes);
    if (lastCongestionReport_.hasCongestion() && !wasCongested_) metrics_.recordCongestionEvent();
    wasCongested_ = lastCongestionReport_.hasCongestion();
    signals_.advance(currentTick_);
    movement_.advance(currentTick_, network_, signals_, vehicles_);
    // Record a completed trip once, when the movement subsystem marks it complete.
    for (VehicleId id = 1; id <= vehicles_.size(); ++id) {
        const auto* vehicle = vehicles_.find(id);
        if (vehicle && vehicle->state == VehicleState::Completed) {
            metrics_.recordCompletedVehicle(id, vehicle->waitingTicks);
        }
    }
    for (const auto& lane : scenario_->lanes) {
        const auto* queue = vehicles_.lane(lane.id);
        if (queue) metrics_.observeQueue(queue->size());
    }
    ++currentTick_;
    return arrivals;
}

void Simulation::run() {
    if (!scenario_) throw std::logic_error("Load a scenario before running simulation");
    while (currentTick_ < config_.duration) step();
}

void Simulation::reset() {
    if (!scenario_) throw std::logic_error("Load a scenario before resetting simulation");
    const auto scenario = *scenario_;
    const auto scheduledIncidents = incidents_.incidents();
    auto provider = routeProvider_;
    configureScenario(scenario);
    routeProvider_ = std::move(provider);
    for (const auto& incident : scheduledIncidents) incidents_.schedule(incident);
}

InputSummary Simulation::inputSummary() const noexcept {
    return {vehicles_.size(), admitted_, vehicles_.queuedCount(), vehicles_.pendingCount(), unroutable_};
}
const VehicleManager& Simulation::vehicles() const noexcept { return vehicles_; }
MetricsSnapshot Simulation::metricsSnapshot() const noexcept { return metrics_.snapshot(); }
CongestionReport Simulation::congestionReport() const { return lastCongestionReport_; }
Tick Simulation::currentTick() const noexcept { return currentTick_; }

}  // namespace traffic
