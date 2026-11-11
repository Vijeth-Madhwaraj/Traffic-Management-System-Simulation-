#pragma once

#include "traffic/network/road_network.hpp"
#include "traffic/signals/signal_controller.hpp"
#include "traffic/vehicle/vehicle_manager.hpp"

#include <map>
#include <optional>

namespace traffic {

class MovementEngine {
public:
    // Ready-to-cross queues used by adaptive signal timing.
    MovementQueueCounts readyMovementQueues(
        Tick now, const RoadNetwork& network, const VehicleManager& vehicles) const;

    // Entry-lane vehicles start on their first route road. At an intersection,
    // a crossing requires the matching `<incoming-road>-to-<outgoing-road>`
    // movement to be Green and the outgoing road and any configured lane on it
    // to have capacity.
    // Integration dependency (FR-06): use the exact MovementId values from
    // each intersection's SignalPhase plan. Check permits() immediately before
    // crossing; it denies unknown movements and all Yellow/Red states.
    void requestEmergencyPriority(Tick now, const RoadNetwork& network,
                                  SignalController& signals, const VehicleManager& vehicles) const;
    void advance(Tick now, RoadNetwork& network, SignalController& signals, VehicleManager& vehicles);

private:
    struct TransitState {
        RoadId road;
        std::string lane;
        Tick enteredAt{};
    };

    std::map<VehicleId, TransitState> inTransit_;
    std::map<std::string, std::size_t> laneOccupancy_;
    std::optional<Tick> lastAdvanced_;
};

}  // namespace traffic
