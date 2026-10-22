#include "traffic/simulation/simulation.hpp"

#include <utility>

namespace traffic {

Simulation::Simulation(SimulationConfig config) : config_(config), vehicles_(config.seed) {}

void Simulation::configureSignals(const IntersectionId& intersection,
                                  std::vector<SignalPhase> phases,
                                  const std::vector<MovementConflict>& conflicts) {
    signals_.configure(intersection, std::move(phases), conflicts);
}

void Simulation::run() {
    // TODO: define and implement the agreed per-tick order. Start with one
    // deterministic vehicle and the end-to-end milestone in the project plan.
    for (currentTick_ = 0; currentTick_ < config_.duration; ++currentTick_) {
        incidents_.advance(currentTick_, network_);
        signals_.advance(currentTick_);
        movement_.advance(currentTick_, network_, signals_, vehicles_);
    }
}

Tick Simulation::currentTick() const noexcept { return currentTick_; }

}  // namespace traffic
