#include "traffic/vehicle/vehicle_manager.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace traffic {

VehicleManager::VehicleManager(std::uint64_t seed) : rng_(seed), seed_(seed) {}

void VehicleManager::configureTopology(std::vector<IntersectionId> intersections, std::vector<Road> roads) {
    if (!vehicles_.empty() || lastGenerated_ || !schedule_.empty() || !streams_.empty() || !lanes_.empty()) throw std::logic_error("Cannot replace topology after vehicles are created");
    std::unordered_set<IntersectionId> nodes;
    std::unordered_map<RoadId, Road> edges;
    for (const auto& id : intersections) {
        if (id.empty() || !nodes.insert(id).second) throw std::invalid_argument("Invalid or duplicate intersection");
    }
    if (nodes.empty()) throw std::invalid_argument("Topology requires intersections");
    for (const auto& road : roads) {
        if (road.id.empty() || !nodes.count(road.from) || !nodes.count(road.to) ||
            road.capacity == 0 || road.travelTime == 0 || road.occupancy > road.capacity ||
            !edges.emplace(road.id, road).second) throw std::invalid_argument("Invalid or duplicate road");
    }
    intersections_ = std::move(nodes);
    roads_ = std::move(edges);
}

void VehicleManager::validateVehicle(const Vehicle& vehicle) const {
    if (!intersections_.count(vehicle.source) || !intersections_.count(vehicle.destination) ||
        vehicle.source == vehicle.destination) throw std::invalid_argument("Vehicle requires distinct known endpoints");
    if (vehicle.vehicleClass != VehicleClass::Standard && vehicle.vehicleClass != VehicleClass::Emergency)
        throw std::invalid_argument("Unknown vehicle class");
    if (vehicle.state != VehicleState::Waiting || vehicle.routeIndex != 0 || vehicle.waitingTicks != 0)
        throw std::invalid_argument("New vehicle must have initial state");
    auto node = vehicle.source;
    for (const auto& id : vehicle.route) {
        const auto road = roads_.find(id);
        if (road == roads_.end() || !road->second.enabled || road->second.from != node)
            throw std::invalid_argument("Vehicle route contains an invalid road connection");
        node = road->second.to;
    }
    if (!vehicle.route.empty() && node != vehicle.destination)
        throw std::invalid_argument("Vehicle route does not reach destination");
}

void VehicleManager::configureLanes(std::vector<EntryLane> lanes) {
    if (lastGenerated_ || !vehicles_.empty() || !schedule_.empty() || !streams_.empty())
        throw std::logic_error("Configure lanes before arrivals");
    std::map<std::string, LaneState> replacement;
    for (const auto& spec : lanes) {
        if (spec.id.empty() || !roads_.count(spec.road) || spec.capacity == 0 ||
            spec.capacity > roads_.at(spec.road).capacity ||
            !replacement.emplace(spec.id, LaneState{spec, LaneQueue(spec.capacity), {}}).second)
            throw std::invalid_argument("Invalid or duplicate entry lane");
    }
    lanes_ = std::move(replacement);
}

void VehicleManager::validateEntry(const Vehicle& vehicle, const std::string& laneId) const {
    const auto found = lanes_.find(laneId);
    if (found == lanes_.end()) throw std::invalid_argument("Unknown entry lane: " + laneId);
    const auto& road = roads_.at(found->second.spec.road);
    if (vehicle.source != road.from || (!vehicle.route.empty() && vehicle.route.front() != road.id))
        throw std::invalid_argument("Vehicle source or route does not match entry lane");
}

AdmissionBatch VehicleManager::advanceArrivals(Tick now, const RouteProvider& routes) {
    if (lastAdmission_ && *lastAdmission_ == now) return {};
    if (lastGenerated_ && (!lastAdmission_ || *lastGenerated_ != *lastAdmission_))
        throw std::logic_error("Do not mix generate() and advanceArrivals() in one run");
    for (const auto& arrival : schedule_) validateEntry(arrival.vehicle, arrival.lane);
    for (const auto& stream : streams_) validateEntry(stream.vehicle, stream.lane);
    // Resolve routes before consuming generation state: a failed provider can be retried.
    auto replacement = *this;
    AdmissionBatch batch;
    for (const auto& event : replacement.generate(now)) {
        auto& vehicle = *replacement.find(event.id);
        batch.generated.push_back(event.id);
        if (vehicle.route.empty() && routes) vehicle.route = routes(vehicle.source, vehicle.destination);
        if (vehicle.route.empty()) {
            vehicle.state = VehicleState::Unroutable;
            batch.unroutable.push_back(event.id);
        } else {
            replacement.validateVehicle(vehicle);
            replacement.validateEntry(vehicle, event.lane);
            replacement.lanes_.at(event.lane).pending.push_back(event.id);
        }
    }
    for (auto& entry : replacement.lanes_) {
        auto& state = entry.second;
        while (!state.pending.empty() && state.queue.hasSpace()) {
            const auto id = state.pending.front();
            if (!state.queue.enqueue(id))
                throw std::logic_error("Pending vehicle is invalid or already queued");
            state.pending.pop_front();
            batch.admitted.push_back(id);
        }
    }
    replacement.lastAdmission_ = now;
    *this = std::move(replacement);
    return batch;
}

LaneQueue* VehicleManager::lane(const std::string& id) noexcept {
    const auto found = lanes_.find(id);
    return found == lanes_.end() ? nullptr : &found->second.queue;
}
const LaneQueue* VehicleManager::lane(const std::string& id) const noexcept {
    const auto found = lanes_.find(id);
    return found == lanes_.end() ? nullptr : &found->second.queue;
}
std::vector<EntryLaneSnapshot> VehicleManager::laneSnapshots() const {
    std::vector<EntryLaneSnapshot> snapshots;
    snapshots.reserve(lanes_.size());
    for (const auto& entry : lanes_) {
        const auto& state = entry.second;
        snapshots.push_back({entry.first, state.spec.road, state.queue.capacity(),
            state.queue.snapshot(), {state.pending.begin(), state.pending.end()}});
    }
    return snapshots;
}

std::size_t VehicleManager::pendingCount() const noexcept {
    std::size_t count = 0;
    for (const auto& entry : lanes_) count += entry.second.pending.size();
    return count;
}
std::size_t VehicleManager::queuedCount() const noexcept {
    std::size_t count = 0;
    for (const auto& entry : lanes_) count += entry.second.queue.size();
    return count;
}

void VehicleManager::setSchedule(std::vector<ScheduledArrival> arrivals) {
    if (lastGenerated_) throw std::logic_error("Cannot change arrivals after generation starts");
    for (const auto& arrival : arrivals) {
        if (arrival.lane.empty()) throw std::invalid_argument("Arrival requires an entry lane");
        validateVehicle(arrival.vehicle);
        if (!lanes_.empty()) validateEntry(arrival.vehicle, arrival.lane);
    }
    std::stable_sort(arrivals.begin(), arrivals.end(), [](const auto& a, const auto& b) { return a.tick < b.tick; });
    schedule_ = std::move(arrivals);
    nextArrival_ = 0;
}

void VehicleManager::setStreams(std::vector<ArrivalStream> streams) {
    if (lastGenerated_) throw std::logic_error("Cannot change streams after generation starts");
    std::unordered_set<std::string> ids;
    for (const auto& stream : streams) {
        if (stream.id.empty() || !ids.insert(stream.id).second || stream.lane.empty() ||
            stream.firstTick >= stream.endTick || !std::isfinite(stream.probability) ||
            stream.probability < 0 || stream.probability > 1)
            throw std::invalid_argument("Invalid arrival stream");
        validateVehicle(stream.vehicle);
        if (!lanes_.empty()) validateEntry(stream.vehicle, stream.lane);
    }
    std::sort(streams.begin(), streams.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    streams_ = std::move(streams);
}

void VehicleManager::reset() {
    for (auto& entry : lanes_) {
        entry.second.queue.clear();
        entry.second.pending.clear();
    }
    lastAdmission_.reset();
    vehicles_.clear();
    nextId_ = 1;
    nextArrival_ = 0;
    lastGenerated_.reset();
    rng_.seed(seed_);
}

std::uint64_t VehicleManager::seed() const noexcept { return seed_; }

std::vector<GeneratedArrival> VehicleManager::generate(Tick now) {
    if (lastGenerated_ && now == *lastGenerated_) return {};
    if ((!lastGenerated_ && now != 0) ||
        (lastGenerated_ && (*lastGenerated_ == std::numeric_limits<Tick>::max() || now != *lastGenerated_ + 1)))
        throw std::invalid_argument("Generate requires consecutive ticks starting at zero");
    std::vector<GeneratedArrival> result;
    while (nextArrival_ < schedule_.size() && schedule_[nextArrival_].tick == now) {
        const auto& arrival = schedule_[nextArrival_];
        result.push_back({arrival.lane, addVehicle(arrival.vehicle)});
        ++nextArrival_;
    }
    for (const auto& stream : streams_) {
        if (now < stream.firstTick || now >= stream.endTick) continue;
        // Use explicit integer thresholds rather than library-specific distributions.
        const auto threshold = static_cast<std::uint64_t>(stream.probability * 4294967296.0);
        if ((rng_() >> 32U) < threshold)
            result.push_back({stream.lane, addVehicle(stream.vehicle)});
    }
    lastGenerated_ = now;
    return result;
}

VehicleId VehicleManager::addVehicle(Vehicle vehicle) {
    validateVehicle(vehicle);
    if (nextId_ == std::numeric_limits<VehicleId>::max()) throw std::overflow_error("Vehicle IDs exhausted");
    vehicle.id = nextId_++;
    const auto id = vehicle.id;
    vehicles_.emplace(id, std::move(vehicle));
    return id;
}

const Vehicle* VehicleManager::find(VehicleId id) const noexcept {
    const auto it = vehicles_.find(id);
    return it == vehicles_.end() ? nullptr : &it->second;
}

Vehicle* VehicleManager::find(VehicleId id) noexcept {
    const auto it = vehicles_.find(id);
    return it == vehicles_.end() ? nullptr : &it->second;
}

std::size_t VehicleManager::size() const noexcept { return vehicles_.size(); }

}  // namespace traffic
