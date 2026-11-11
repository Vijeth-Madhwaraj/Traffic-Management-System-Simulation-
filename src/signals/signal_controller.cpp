#include "traffic/signals/signal_controller.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace traffic {

void SignalController::configure(const IntersectionId& intersection,
                                 std::vector<SignalPhase> phases,
                                 const std::vector<MovementConflict>& conflicts,
                                 std::optional<AdaptiveSignalPolicy> adaptivePolicy) {
    if (intersection.empty()) {
        throw std::invalid_argument("Signal intersection ID must not be empty");
    }
    if (phases.empty()) {
        throw std::invalid_argument("A controlled intersection must have at least one signal phase");
    }
    if (adaptivePolicy && (adaptivePolicy->minimumGreenDuration == 0 ||
        adaptivePolicy->minimumGreenDuration > adaptivePolicy->maximumGreenDuration ||
        adaptivePolicy->extendQueueThreshold == 0 || adaptivePolicy->adjustment == 0)) {
        throw std::invalid_argument("Invalid adaptive signal timing policy");
    }

    std::unordered_set<MovementId> allMovements;
    for (std::size_t index = 0; index < phases.size(); ++index) {
        const auto& phase = phases[index];
        if (phase.greenDuration == 0 || phase.yellowDuration == 0 || phase.allRedDuration == 0) {
            throw std::invalid_argument("Signal phase durations must all be greater than zero");
        }
        if (adaptivePolicy && (phase.greenDuration < adaptivePolicy->minimumGreenDuration ||
            phase.greenDuration > adaptivePolicy->maximumGreenDuration)) {
            throw std::invalid_argument("Base green duration must be within adaptive timing limits");
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
    std::vector<Tick> greenDurations;
    greenDurations.reserve(phases.size());
    for (const auto& phase : phases) greenDurations.push_back(phase.greenDuration);
    states_[intersection] = State{std::move(phases), 0, SignalColor::Green, 0, 0,
                                  adaptivePolicy, std::move(greenDurations), true, {}};
}

void SignalController::advance(Tick now) {
    advance(now, {});
}

void SignalController::advance(Tick now, const MovementQueueCounts& queues) {
    // Validate first so a backwards tick cannot partially advance the network
    // before an out-of-order intersection is encountered.
    for (const auto& entry : states_) {
        if (now < entry.second.lastAdvanced) {
            throw std::invalid_argument("SignalController::advance requires non-decreasing simulation ticks");
        }
    }

    for (auto& entry : states_) {
        const auto& intersection = entry.first;
        auto& state = entry.second;
        const auto adaptGreen = [&]() {
            if (!state.adaptivePolicy || !state.adaptOnGreenStart) return;
            std::size_t queued = 0;
            const auto intersectionQueues = queues.find(intersection);
            if (intersectionQueues != queues.end()) {
                for (const auto& movement : state.phases[state.activePhase].permittedMovements) {
                    const auto movementQueue = intersectionQueues->second.find(movement);
                    if (movementQueue != intersectionQueues->second.end()) {
                        const auto count = movementQueue->second;
                        queued = count > std::numeric_limits<std::size_t>::max() - queued
                                     ? std::numeric_limits<std::size_t>::max()
                                     : queued + count;
                    }
                }
            }
            auto& duration = state.greenDurations[state.activePhase];
            const auto& policy = *state.adaptivePolicy;
            if (queued >= policy.extendQueueThreshold) {
                duration = policy.maximumGreenDuration - duration < policy.adjustment
                               ? policy.maximumGreenDuration
                               : duration + policy.adjustment;
            } else if (queued == 0) {
                duration = duration - policy.minimumGreenDuration < policy.adjustment
                               ? policy.minimumGreenDuration
                               : duration - policy.adjustment;
            }
            state.adaptOnGreenStart = false;
        };
        const auto currentDuration = [&state]() {
            const auto& phase = state.phases[state.activePhase];
            switch (state.color) {
                case SignalColor::Green: return state.greenDurations[state.activePhase];
                case SignalColor::Yellow: return phase.yellowDuration;
                case SignalColor::Red: return phase.allRedDuration;
            }
            return Tick{0};
        };

        adaptGreen();
        while (now - state.colorStarted >= currentDuration()) {
            const Tick duration = currentDuration();
            state.colorStarted += duration;

            if (state.color == SignalColor::Green) {
                state.color = SignalColor::Yellow;
            } else if (state.color == SignalColor::Yellow) {
                state.color = SignalColor::Red;
            } else {
                if (state.priorityPhases.empty()) {
                    state.activePhase = (state.activePhase + 1) % state.phases.size();
                } else {
                    state.activePhase = state.priorityPhases.front();
                    state.priorityPhases.erase(state.priorityPhases.begin());
                }
                state.color = SignalColor::Green;
                state.adaptOnGreenStart = true;
                adaptGreen();
            }
        }
        if (state.color == SignalColor::Green) {
            auto& pending = state.priorityPhases;
            pending.erase(std::remove(pending.begin(), pending.end(), state.activePhase), pending.end());
        }
        state.lastAdvanced = now;
    }
}

bool SignalController::requestPriority(const IntersectionId& intersection, const MovementId& movement) {
    const auto found = states_.find(intersection);
    if (found == states_.end()) return false;
    auto& state = found->second;
    for (std::size_t index = 0; index < state.phases.size(); ++index) {
        const auto& permitted = state.phases[index].permittedMovements;
        if (std::find(permitted.begin(), permitted.end(), movement) == permitted.end()) continue;
        auto& pending = state.priorityPhases;
        if (std::find(pending.begin(), pending.end(), index) == pending.end()) pending.push_back(index);
        return true;
    }
    return false;
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
                                                         : std::vector<MovementId>{},
                        state.greenDurations[state.activePhase]};
}

}  // namespace traffic
