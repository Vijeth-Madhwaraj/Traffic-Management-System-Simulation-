#include "traffic/signals/signal_controller.hpp"

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace traffic {

void SignalController::configure(const IntersectionId& intersection,
                                 std::vector<SignalPhase> phases,
                                 const std::vector<MovementConflict>& conflicts) {
    if (intersection.empty()) {
        throw std::invalid_argument("Signal intersection ID must not be empty");
    }
    if (phases.empty()) {
        throw std::invalid_argument("A controlled intersection must have at least one signal phase");
    }

    std::unordered_set<MovementId> allMovements;
    for (std::size_t index = 0; index < phases.size(); ++index) {
        const auto& phase = phases[index];
        if (phase.greenDuration == 0 || phase.yellowDuration == 0 || phase.allRedDuration == 0) {
            throw std::invalid_argument("Signal phase durations must all be greater than zero");
        }
        if (phase.permittedMovements.empty()) {
            throw std::invalid_argument("Signal phase must permit at least one movement");
        }

        std::unordered_set<MovementId> uniqueMovements;
        for (const auto& movement : phase.permittedMovements) {
            if (movement.empty()) {
                throw std::invalid_argument("Signal movement IDs must not be empty");
            }
            if (!uniqueMovements.insert(movement).second) {
                throw std::invalid_argument("Signal phase contains a duplicate movement ID: " + movement);
            }
            allMovements.insert(movement);
        }

        for (const auto& conflict : conflicts) {
            const bool firstPresent = uniqueMovements.count(conflict.first) != 0;
            const bool secondPresent = uniqueMovements.count(conflict.second) != 0;
            if (firstPresent && secondPresent) {
                throw std::invalid_argument("Signal phase " + std::to_string(index) +
                                            " permits conflicting movements '" + conflict.first +
                                            "' and '" + conflict.second + "'");
            }
        }
    }

    for (const auto& conflict : conflicts) {
        if (conflict.first.empty() || conflict.second.empty() || conflict.first == conflict.second) {
            throw std::invalid_argument("Movement conflict pairs must contain two distinct, non-empty IDs");
        }
        if (allMovements.count(conflict.first) == 0 || allMovements.count(conflict.second) == 0) {
            throw std::invalid_argument("Movement conflict references an ID not present in any signal phase");
        }
    }

    // The first phase is active at tick zero. Apply only after validation so a
    // bad replacement plan cannot leave the intersection in a partial state.
    states_[intersection] = State{std::move(phases), 0, SignalColor::Green, 0, 0};
}

void SignalController::advance(Tick now) {
    // Validate first so a backwards tick cannot partially advance the network
    // before an out-of-order intersection is encountered.
    for (const auto& entry : states_) {
        if (now < entry.second.lastAdvanced) {
            throw std::invalid_argument("SignalController::advance requires non-decreasing simulation ticks");
        }
    }

    for (auto& entry : states_) {
        auto& state = entry.second;
        const auto currentDuration = [&state]() {
            const auto& phase = state.phases[state.activePhase];
            switch (state.color) {
                case SignalColor::Green: return phase.greenDuration;
                case SignalColor::Yellow: return phase.yellowDuration;
                case SignalColor::Red: return phase.allRedDuration;
            }
            return Tick{0};
        };

        while (now - state.colorStarted >= currentDuration()) {
            const Tick duration = currentDuration();
            state.colorStarted += duration;

            if (state.color == SignalColor::Green) {
                state.color = SignalColor::Yellow;
            } else if (state.color == SignalColor::Yellow) {
                state.color = SignalColor::Red;
            } else {
                state.activePhase = (state.activePhase + 1) % state.phases.size();
                state.color = SignalColor::Green;
            }
        }
        state.lastAdvanced = now;
    }
}

bool SignalController::permits(const IntersectionId& intersection, const MovementId& movement) const {
    const auto it = states_.find(intersection);
    if (it == states_.end() || it->second.color != SignalColor::Green) {
        return false;
    }

    const auto& movements = it->second.phases[it->second.activePhase].permittedMovements;
    return std::find(movements.begin(), movements.end(), movement) != movements.end();
}

std::optional<SignalStatus> SignalController::status(const IntersectionId& intersection, Tick now) const {
    const auto it = states_.find(intersection);
    if (it == states_.end()) {
        return std::nullopt;
    }
    if (now < it->second.colorStarted || now > it->second.lastAdvanced) {
        throw std::invalid_argument("Signal status requires a tick already processed by advance()");
    }

    const auto& state = it->second;
    const auto& phase = state.phases[state.activePhase];
    return SignalStatus{state.activePhase, state.color, now - state.colorStarted,
                        state.color == SignalColor::Green ? phase.permittedMovements
                                                         : std::vector<MovementId>{}};
}

}  // namespace traffic
