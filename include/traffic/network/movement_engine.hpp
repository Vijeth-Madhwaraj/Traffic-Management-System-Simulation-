#pragma once

#include "traffic/network/road_network.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/vehicle/vehicle_manager.hpp"

#include <map>
#include <optional>

namespace traffic {

class MovementEngine {
public:
    // Entry-lane vehicles start on their first route road. At an intersection,
    // a crossing requires the matching `<incoming-road>-to-<outgoing-road>`
    // movement to be Green and the outgoing road to have capacity.
    // Integration dependency (FR-06): use the exact MovementId values from
    // each intersection's SignalPhase plan. Check permits() immediately before
    // crossing; it denies unknown movements and all Yellow/Red states.
    void advance(Tick now, RoadNetwork& network, SignalController& signals, VehicleManager& vehicles);

private:
    struct TransitState {
        RoadId road;
        Tick enteredAt{};
    };

    std::map<VehicleId, TransitState> inTransit_;
    std::optional<Tick> lastAdvanced_;
};

}  // namespace traffic
