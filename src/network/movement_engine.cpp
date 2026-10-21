#include "traffic/network/movement_engine.hpp"

namespace traffic {

void MovementEngine::advance(Tick, RoadNetwork&, SignalController&, VehicleManager&) {
    // TODO: move only signal-eligible vehicles and enforce downstream capacity
    // and travel-time constraints.
}

}  // namespace traffic
