#include "traffic/network/movement_engine.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace traffic {

void MovementEngine::advance(Tick now, RoadNetwork& network, SignalController& signals,
                             VehicleManager& vehicles) {
    if (lastAdvanced_ && now < *lastAdvanced_) {
        throw std::invalid_argument("MovementEngine::advance requires non-decreasing ticks");
    }
    if (lastAdvanced_ && now == *lastAdvanced_) return;

    const auto recordWait = [](Vehicle* vehicle) {
        if (vehicle && vehicle->waitingTicks < std::numeric_limits<Tick>::max()) {
            ++vehicle->waitingTicks;
        }
    };

    std::vector<VehicleId> readyVehicles;
    readyVehicles.reserve(inTransit_.size());
    for (const auto& transit : inTransit_) readyVehicles.push_back(transit.first);
    std::sort(readyVehicles.begin(), readyVehicles.end(), [&](VehicleId left, VehicleId right) {
        const auto* leftTransitRoad = network.findRoad(inTransit_.at(left).road);
        const auto* rightTransitRoad = network.findRoad(inTransit_.at(right).road);
        const auto leftIntersection = leftTransitRoad ? leftTransitRoad->to : IntersectionId{};
        const auto rightIntersection = rightTransitRoad ? rightTransitRoad->to : IntersectionId{};
        return std::tie(leftIntersection, left) < std::tie(rightIntersection, right);
    });

    for (const auto id : readyVehicles) {
        auto transit = inTransit_.find(id);
        auto* vehicle = vehicles.find(id);
        if (transit == inTransit_.end() || !vehicle || vehicle->state != VehicleState::InTransit) continue;
        auto* currentRoad = network.findRoad(transit->second.road);
        if (!currentRoad || currentRoad->occupancy == 0 ||
            vehicle->routeIndex >= vehicle->route.size() ||
            vehicle->route[vehicle->routeIndex] != currentRoad->id ||
            now < transit->second.enteredAt || now - transit->second.enteredAt < currentRoad->travelTime) {
            continue;
        }

        if (vehicle->routeIndex + 1 == vehicle->route.size()) {
            if (currentRoad->to == vehicle->destination) {
                --currentRoad->occupancy;
                vehicle->routeIndex = vehicle->route.size();
                vehicle->state = VehicleState::Completed;
                inTransit_.erase(transit);
            } else {
                recordWait(vehicle);
            }
            continue;
        }

        const auto& nextRoadId = vehicle->route[vehicle->routeIndex + 1];
        auto* nextRoad = network.findRoad(nextRoadId);
        if (!nextRoad || !nextRoad->enabled || nextRoad->from != currentRoad->to) {
            recordWait(vehicle);
            continue;
        }

        const MovementId movement = currentRoad->id + "-to-" + nextRoad->id;
        if (!signals.permits(currentRoad->to, movement) ||
            (nextRoad->id != currentRoad->id && nextRoad->occupancy >= nextRoad->capacity)) {
            recordWait(vehicle);
            continue;
        }

        TransitState nextTransit{nextRoad->id, now};
        if (nextRoad->id != currentRoad->id) {
            --currentRoad->occupancy;
            ++nextRoad->occupancy;
        }
        ++vehicle->routeIndex;
        transit->second = std::move(nextTransit);
    }

    auto lanes = vehicles.entryLanes();
    std::sort(lanes.begin(), lanes.end(), [&](const EntryLane& left, const EntryLane& right) {
        const auto* leftRoad = network.findRoad(left.road);
        const auto* rightRoad = network.findRoad(right.road);
        const auto leftIntersection = leftRoad ? leftRoad->from : IntersectionId{};
        const auto rightIntersection = rightRoad ? rightRoad->from : IntersectionId{};
        return std::tie(leftIntersection, left.id) < std::tie(rightIntersection, right.id);
    });

    for (const auto& entryLane : lanes) {
        auto* queue = vehicles.lane(entryLane.id);
        auto* road = network.findRoad(entryLane.road);
        if (!queue || !road) continue;
        while (const auto front = queue->front()) {
            auto* vehicle = vehicles.find(*front);
            if (!vehicle || vehicle->state != VehicleState::Waiting || vehicle->route.empty() ||
                vehicle->route.front() != road->id || vehicle->source != road->from || !road->enabled ||
                road->occupancy >= road->capacity) {
                recordWait(vehicle);
                break;
            }

            auto inserted = inTransit_.emplace(*front, TransitState{road->id, now});
            if (!inserted.second) break;
            const auto popped = queue->pop();
            if (!popped || *popped != *front) {
                inTransit_.erase(inserted.first);
                break;
            }
            ++road->occupancy;
            vehicle->state = VehicleState::InTransit;
        }
    }

    lastAdvanced_ = now;
}

}  // namespace traffic
