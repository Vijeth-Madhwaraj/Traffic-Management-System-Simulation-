#pragma once

#include "traffic/types.hpp"

#include <optional>
#include <cstddef>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

namespace traffic {

struct SignalPhase {
    // Movement IDs must use the same naming convention as MovementEngine.
    std::vector<MovementId> permittedMovements;
    Tick greenDuration{10};
    Tick yellowDuration{2};
    Tick allRedDuration{1};
};

struct AdaptiveSignalPolicy {
    // A phase extends when its permitted ready-to-cross queue reaches the
    // threshold, shortens when that queue is empty, and otherwise holds.
    Tick minimumGreenDuration{1};
    Tick maximumGreenDuration{1000000};
    std::size_t extendQueueThreshold{2};
    Tick adjustment{1};
};

using MovementQueueCounts =
    std::unordered_map<IntersectionId, std::unordered_map<MovementId, std::size_t>>;

using MovementConflict = std::pair<MovementId, MovementId>;

struct SignalStatus {
    std::size_t phaseIndex{};
    SignalColor color{SignalColor::Red};
    Tick colorElapsed{};
    std::vector<MovementId> permittedMovements;
    Tick greenDuration{};
};

class SignalController {
public:
    // Configure once before the simulation starts. The network/intersection
    // owner must provide every conflicting movement pair for this intersection.
    // Invalid or unsafe plans throw std::invalid_argument; an invalid plan does
    // not replace a previously configured plan.
    void configure(const IntersectionId& intersection,
                   std::vector<SignalPhase> phases,
                   const std::vector<MovementConflict>& conflicts,
                   std::optional<AdaptiveSignalPolicy> adaptivePolicy = std::nullopt);

    // Call with non-decreasing simulation ticks. The no-queue overload treats
    // adaptive phase queues as empty. Large time jumps apply each elapsed transition.
    void advance(Tick now);
    void advance(Tick now, const MovementQueueCounts& queues);

    // MovementEngine must check this immediately before moving a vehicle. It
    // returns true only during Green; Yellow and Red always deny movement.
    bool permits(const IntersectionId& intersection, const MovementId& movement) const;
    std::optional<SignalStatus> status(const IntersectionId& intersection, Tick now) const;

private:
    struct State {
        std::vector<SignalPhase> phases;
        std::size_t activePhase{};
        SignalColor color{SignalColor::Red};
        Tick colorStarted{};
        Tick lastAdvanced{};
        std::optional<AdaptiveSignalPolicy> adaptivePolicy;
        std::vector<Tick> greenDurations;
        bool adaptOnGreenStart{true};
    };
    std::unordered_map<IntersectionId, State> states_;
};

}  // namespace traffic
