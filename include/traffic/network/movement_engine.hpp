#pragma once

#include "traffic/network/road_network.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/vehicle/vehicle_manager.hpp"

namespace traffic {

class MovementEngine {
public:
    void advance(Tick now, RoadNetwork& network, SignalController& signals, VehicleManager& vehicles);
};

}  // namespace traffic
