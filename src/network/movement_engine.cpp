#include "traffic/network/movement_engine.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace traffic {

MovementQueueCounts MovementEngine::readyMovementQueues(Tick now, const RoadNetwork &network,
                                                        const VehicleManager &vehicles) const {
    MovementQueueCounts queues;
    for (const auto &entry : inTransit_) {
        const auto *vehicle = vehicles.find(entry.first);
        const auto *road = network.findRoad(entry.second.road);
        if (!vehicle || vehicle->state != VehicleState::InTransit || !road ||
            vehicle->routeIndex >= vehicle->route.size() ||
            vehicle->route.size() - vehicle->routeIndex < 2 || now < entry.second.enteredAt ||
            now - entry.second.enteredAt < road->travelTime) {
            continue;
        }
        const auto *nextRoad = network.findRoad(vehicle->route[vehicle->routeIndex + 1]);
        if (!nextRoad || !nextRoad->enabled || nextRoad->from != road->to)
            continue;
        const MovementId movement = road->id + "-to-" + nextRoad->id;
        ++queues[road->to][movement];
    }
    return queues;
}

void MovementEngine::requestEmergencyPriority(Tick now, const RoadNetwork &network,
                                              SignalController &signals,
                                              const VehicleManager &vehicles) const {
    for (const auto &entry : inTransit_) {
        const auto *vehicle = vehicles.find(entry.first);
        const auto *road = network.findRoad(entry.second.road);
        if (!vehicle || vehicle->vehicleClass != VehicleClass::Emergency ||
            vehicle->state != VehicleState::InTransit || !road ||
            vehicle->routeIndex >= vehicle->route.size() ||
            vehicle->route.size() - vehicle->routeIndex < 2 || now < entry.second.enteredAt ||
            now - entry.second.enteredAt < road->travelTime)
            continue;
        const auto *next = network.findRoad(vehicle->route[vehicle->routeIndex + 1]);
        if (next && next->enabled && next->from == road->to)
            signals.requestPriority(road->to, road->id + "-to-" + next->id);
    }
}

void MovementEngine::advance(Tick now, RoadNetwork &network, SignalController &signals,
                             VehicleManager &vehicles) {
    if (lastAdvanced_ && now < *lastAdvanced_) {
        throw std::invalid_argument("MovementEngine::advance requires non-decreasing ticks");
    }
    if (lastAdvanced_ && now == *lastAdvanced_)
        return;

    const auto recordWait = [](Vehicle *vehicle) {
        if (vehicle && vehicle->waitingTicks < std::numeric_limits<Tick>::max()) {
            ++vehicle->waitingTicks;
        }
    };

    const auto configuredLanes = vehicles.entryLanes();
    std::vector<VehicleId> readyVehicles;
    readyVehicles.reserve(inTransit_.size());
    for (const auto &transit : inTransit_)
        readyVehicles.push_back(transit.first);
    std::sort(readyVehicles.begin(), readyVehicles.end(), [&](VehicleId left, VehicleId right) {
        const auto *leftTransitRoad = network.findRoad(inTransit_.at(left).road);
        const auto *rightTransitRoad = network.findRoad(inTransit_.at(right).road);
        const auto leftIntersection = leftTransitRoad ? leftTransitRoad->to : IntersectionId{};
        const auto rightIntersection = rightTransitRoad ? rightTransitRoad->to : IntersectionId{};
        return std::tie(leftIntersection, left) < std::tie(rightIntersection, right);
    });

    for (const auto id : readyVehicles) {
        auto transit = inTransit_.find(id);
        auto *vehicle = vehicles.find(id);
        if (transit == inTransit_.end() || !vehicle || vehicle->state != VehicleState::InTransit)
            continue;
        auto *currentRoad = network.findRoad(transit->second.road);
        if (!currentRoad || currentRoad->occupancy == 0 ||
            vehicle->routeIndex >= vehicle->route.size() ||
            vehicle->route[vehicle->routeIndex] != currentRoad->id ||
            now < transit->second.enteredAt ||
            now - transit->second.enteredAt < currentRoad->travelTime) {
            continue;
        }

        if (vehicle->routeIndex + 1 == vehicle->route.size()) {
            if (currentRoad->to == vehicle->destination) {
                --currentRoad->occupancy;
                if (!transit->second.lane.empty())
                    --laneOccupancy_.at(transit->second.lane);
                vehicle->routeIndex = vehicle->route.size();
                vehicle->state = VehicleState::Completed;
                inTransit_.erase(transit);
            } else {
                recordWait(vehicle);
            }
            continue;
        }

        const auto &nextRoadId = vehicle->route[vehicle->routeIndex + 1];
        auto *nextRoad = network.findRoad(nextRoadId);
        if (!nextRoad || !nextRoad->enabled || nextRoad->from != currentRoad->to) {
            recordWait(vehicle);
            continue;
        }

        const MovementId movement = currentRoad->id + "-to-" + nextRoad->id;
        const EntryLane *targetLane = nullptr;
        bool hasTargetLanes = false;
        for (const auto &lane : configuredLanes) {
            if (lane.road != nextRoad->id)
                continue;
            hasTargetLanes = true;
            if (laneOccupancy_[lane.id] < lane.capacity) {
                targetLane = &lane;
                break;
            }
        }
        const bool laneFull = hasTargetLanes && targetLane == nullptr;
        if (!signals.permits(currentRoad->to, movement) ||
            (nextRoad->id != currentRoad->id && nextRoad->occupancy >= nextRoad->capacity) ||
            laneFull) {
            recordWait(vehicle);
            continue;
        }

        const std::string nextLaneId = targetLane ? targetLane->id : std::string{};
        TransitState nextTransit{nextRoad->id, nextLaneId, now};
        if (nextRoad->id != currentRoad->id) {
            --currentRoad->occupancy;
            ++nextRoad->occupancy;
        }
        if (transit->second.lane != nextLaneId) {
            if (!transit->second.lane.empty())
                --laneOccupancy_.at(transit->second.lane);
            if (!nextLaneId.empty())
                ++laneOccupancy_[nextLaneId];
        }
        ++vehicle->routeIndex;
        transit->second = std::move(nextTransit);
    }

    auto lanes = vehicles.entryLanes();
    std::sort(lanes.begin(), lanes.end(), [&](const EntryLane &left, const EntryLane &right) {
        const auto *leftRoad = network.findRoad(left.road);
        const auto *rightRoad = network.findRoad(right.road);
        const auto leftIntersection = leftRoad ? leftRoad->from : IntersectionId{};
        const auto rightIntersection = rightRoad ? rightRoad->from : IntersectionId{};
        return std::tie(leftIntersection, left.id) < std::tie(rightIntersection, right.id);
    });

    for (const auto &entryLane : lanes) {
        auto *queue = vehicles.lane(entryLane.id);
        auto *road = network.findRoad(entryLane.road);
        if (!queue || !road)
            continue;
        while (const auto front = queue->front()) {
            auto *vehicle = vehicles.find(*front);
            if (!vehicle || vehicle->state != VehicleState::Waiting || vehicle->route.empty() ||
                vehicle->route.front() != road->id || vehicle->source != road->from ||
                !road->enabled || road->occupancy >= road->capacity) {
                break;
            }

            // Entry queues are FR-04 waiting lanes. FR-11 downstream lane
            // occupancy starts only when a vehicle crosses into an outgoing lane.
            auto inserted = inTransit_.emplace(*front, TransitState{road->id, {}, now});
            if (!inserted.second)
                break;
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

} // namespace traffic
