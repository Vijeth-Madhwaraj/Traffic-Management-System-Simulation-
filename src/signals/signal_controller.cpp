#include "traffic/signals/signal_controller.hpp"

namespace traffic {

void SignalController::configure(const IntersectionId& intersection, std::vector<SignalPhase> phases) {
    states_[intersection] = State{std::move(phases), 0, 0};
}

void SignalController::advance(Tick now) {
    // TODO: validate timings and implement safe phase transitions (including
    // yellow/all-red clearance if the team chooses to model those states).
    for (auto& entry : states_) {
        auto& state = entry.second;
        if (!state.phases.empty() && now - state.phaseStarted >= state.phases[state.activePhase].greenDuration) {
            state.activePhase = (state.activePhase + 1) % state.phases.size();
            state.phaseStarted = now;
        }
    }
}

bool SignalController::permits(const IntersectionId&, const std::string&) const {
    // TODO: consult the active phase and enforce the intersection conflict table.
    return false;
}

}  // namespace traffic
