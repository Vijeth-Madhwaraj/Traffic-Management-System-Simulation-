#pragma once

#include "traffic/network/road_network.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/vehicle/vehicle_manager.hpp"

namespace traffic {

class MovementEngine {
public:
    // Integration dependency (FR-06): use the exact MovementId values from
    // each intersection's SignalPhase plan. Check permits() immediately before
    // crossing; it denies unknown movements and all Yellow/Red states.
    void advance(Tick now, RoadNetwork& network, SignalController& signals, VehicleManager& vehicles);
};

}  // namespace traffic
