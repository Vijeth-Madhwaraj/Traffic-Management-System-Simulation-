#pragma once

#include "traffic/types.hpp"

#include <unordered_map>
#include <vector>

namespace traffic {

struct SignalPhase {
    std::vector<std::string> permittedMovements;
    Tick greenDuration{10};
};

class SignalController {
public:
    void configure(const IntersectionId& intersection, std::vector<SignalPhase> phases);
    void advance(Tick now);
    bool permits(const IntersectionId& intersection, const std::string& movement) const;

private:
    struct State { std::vector<SignalPhase> phases; std::size_t activePhase{}; Tick phaseStarted{}; };
    std::unordered_map<IntersectionId, State> states_;
};

}  // namespace traffic
