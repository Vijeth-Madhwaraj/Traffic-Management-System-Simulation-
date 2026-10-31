#pragma once

#include "traffic/types.hpp"
#include "traffic/vehicle/lane_queue.hpp"

#include <optional>
#include <functional>
#include <map>
#include <random>
#include <unordered_set>
#include <unordered_map>

namespace traffic {

struct ScheduledArrival {
    Tick tick{};
    std::string lane;
    Vehicle vehicle;
};

struct ArrivalStream {
    static constexpr std::size_t maxVehiclesPerArrival = 1000;
    std::string id;
    std::string lane;
    Tick firstTick{};
    Tick endTick{}; // exclusive
    double probability{}; // one probability decision per active tick
    Vehicle vehicle;
    std::size_t vehiclesPerArrival{1}; // vehicles created when that decision succeeds
};

struct GeneratedArrival {
    std::string lane;
    VehicleId id{};
};

struct EntryLane {
    std::string id;
    RoadId road;
    std::size_t capacity{};
};

struct EntryLaneSnapshot {
    std::string id;
    RoadId road;
    std::size_t capacity{};
    std::vector<VehicleId> queued;
    std::vector<VehicleId> pending;
};

using RouteProvider = std::function<std::vector<RoadId>(const IntersectionId&, const IntersectionId&)>;

struct AdmissionBatch {
    std::vector<VehicleId> generated;
    std::vector<VehicleId> admitted;
    std::vector<VehicleId> unroutable;
};

class VehicleManager {
public:
    explicit VehicleManager(std::uint64_t seed);
    void configureTopology(std::vector<IntersectionId> intersections, std::vector<Road> roads);
    void validateVehicle(const Vehicle& vehicle) const;
    void configureLanes(std::vector<EntryLane> lanes);
    AdmissionBatch advanceArrivals(Tick now, const RouteProvider& routes);
    std::vector<EntryLane> entryLanes() const;
    LaneQueue* lane(const std::string& id) noexcept;
    const LaneQueue* lane(const std::string& id) const noexcept;
    // Independent value snapshots, sorted by lane ID; reading does not advance traffic.
    std::vector<EntryLaneSnapshot> laneSnapshots() const;
    std::size_t pendingCount() const noexcept;
    std::size_t queuedCount() const noexcept;
    void setSchedule(std::vector<ScheduledArrival> arrivals);
    void setStreams(std::vector<ArrivalStream> streams);
    void reset();
    std::uint64_t seed() const noexcept;
    std::vector<GeneratedArrival> generate(Tick now);
    VehicleId addVehicle(Vehicle vehicle);
    const Vehicle* find(VehicleId id) const noexcept;
    Vehicle* find(VehicleId id) noexcept;
    std::size_t size() const noexcept;

private:
    struct LaneState {
        EntryLane spec;
        LaneQueue queue;
        std::deque<VehicleId> pending;
    };
    void validateEntry(const Vehicle& vehicle, const std::string& lane) const;
    std::map<std::string, LaneState> lanes_;
    std::optional<Tick> lastAdmission_;
    std::unordered_set<IntersectionId> intersections_;
    std::unordered_map<RoadId, Road> roads_;
    std::vector<ArrivalStream> streams_;
    std::mt19937_64 rng_;
    std::vector<ScheduledArrival> schedule_;
    std::size_t nextArrival_{};
    std::optional<Tick> lastGenerated_;
    std::uint64_t seed_;
    VehicleId nextId_{1};
    std::unordered_map<VehicleId, Vehicle> vehicles_;
};

}  // namespace traffic
