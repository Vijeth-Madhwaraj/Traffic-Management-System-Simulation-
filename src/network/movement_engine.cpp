#include "traffic/network/movement_engine.hpp"

#include <algorithm>
#include <limits>
#include <string>

namespace traffic {
namespace {
MovementId movementId(const RoadId& incoming, const RoadId& outgoing) {
    return incoming + "-to-" + outgoing;
}
}

void MovementEngine::advance(Tick now, RoadNetwork& network, SignalController& signals,
                             VehicleManager& vehicles) {
    // Admit the front vehicle from each entry queue onto its first road.
    // Entry queues are FIFO; roads enforce their own independent capacity.
    for (const auto& laneId : vehicles.laneIds()) {
        auto* queue = vehicles.lane(laneId);
        const auto entryRoadId = vehicles.entryRoad(laneId);
        if (!queue || !entryRoadId || queue->empty()) continue;
        auto* road = network.findRoad(*entryRoadId);
        if (!road || !road->enabled || road->occupancy >= road->capacity) continue;
        const auto front = queue->front();
        if (!front) continue;
        auto* vehicle = vehicles.find(*front);
        if (!vehicle || vehicle->state != VehicleState::Waiting || vehicle->route.empty() ||
            vehicle->route.front() != road->id) continue;
        queue->pop();
        ++road->occupancy;
        vehicle->state = VehicleState::InTransit;
        vehicle->routeIndex = 0;
        enteredRoadAt_[vehicle->id] = now;
    }

    // Iterate by ID for deterministic outcomes when several vehicles compete
    // for the same downstream road in a single tick.
    const auto count = vehicles.size();
    for (VehicleId id = 1; id <= count; ++id) {
        auto* vehicle = vehicles.find(id);
        if (!vehicle || vehicle->state != VehicleState::InTransit || vehicle->route.empty()) continue;
        auto entered = enteredRoadAt_.find(id);
        if (entered == enteredRoadAt_.end() || now < entered->second) continue;
        if (vehicle->routeIndex >= vehicle->route.size()) continue;

        auto* currentRoad = network.findRoad(vehicle->route[vehicle->routeIndex]);
        if (!currentRoad) {
            vehicle->state = VehicleState::Unroutable;
            enteredRoadAt_.erase(id);
            continue;
        }
        const Tick elapsed = now - entered->second;
        if (elapsed < currentRoad->travelTime) continue;

        // Reaching the destination completes the trip after the final road's
        // travel time; no signal is needed to leave the network.
        if (vehicle->routeIndex + 1 >= vehicle->route.size()) {
            if (currentRoad->occupancy > 0) --currentRoad->occupancy;
            vehicle->state = VehicleState::Completed;
            enteredRoadAt_.erase(id);
            continue;
        }

        auto* nextRoad = network.findRoad(vehicle->route[vehicle->routeIndex + 1]);
        if (!nextRoad || !nextRoad->enabled) {
            // Re-plan from the intersection at the end of the current road.
            // Keep the road being traversed as the route prefix.
            const auto detour = network.shortestRoute(currentRoad->to, vehicle->destination);
            if (detour.empty()) {
                if (currentRoad->occupancy > 0) --currentRoad->occupancy;
                vehicle->state = VehicleState::Unroutable;
                enteredRoadAt_.erase(id);
                continue;
            }
            std::vector<RoadId> replacement;
            replacement.reserve(vehicle->routeIndex + 1 + detour.size());
            replacement.insert(replacement.end(), vehicle->route.begin(),
                               vehicle->route.begin() + static_cast<std::ptrdiff_t>(vehicle->routeIndex + 1));
            replacement.insert(replacement.end(), detour.begin(), detour.end());
            vehicle->route = std::move(replacement);
            nextRoad = network.findRoad(vehicle->route[vehicle->routeIndex + 1]);
        }
        if (!nextRoad || nextRoad->occupancy >= nextRoad->capacity) {
            ++vehicle->waitingTicks;
            continue;
        }

        // Unsignalized intersections allow movement. Controlled intersections
        // require the exact incoming-to-outgoing movement to be green.
        const auto movement = movementId(currentRoad->id, nextRoad->id);
        const auto signalStatus = signals.status(currentRoad->to, now);
        if (signalStatus && !signals.permits(currentRoad->to, movement)) {
            ++vehicle->waitingTicks;
            continue;
        }

        if (currentRoad->occupancy > 0) --currentRoad->occupancy;
        ++nextRoad->occupancy;
        ++vehicle->routeIndex;
        enteredRoadAt_[id] = now;
    }
}

}  // namespace traffic
