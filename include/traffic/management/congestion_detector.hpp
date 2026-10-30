#pragma once

#include "traffic/network/road_network.hpp"
#include "traffic/types.hpp"
#include "traffic/vehicle/lane_queue.hpp"
#include "traffic/vehicle/vehicle_manager.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace traffic {

struct CongestionThresholds {
    // Occupancy ratio threshold in range [0.0, 1.0]. Defaults to 0.8 (80%).
    double roadOccupancyRatio{0.8};
    // Queue occupancy ratio threshold in range [0.0, 1.0]. Defaults to 0.8 (80%).
    double queueOccupancyRatio{0.8};
    // Optional absolute queue length threshold. 0 means disabled.
    std::size_t queueLengthThreshold{0};
};

struct CongestionReport {
    std::vector<RoadId> congestedRoads;
    std::vector<std::string> congestedLanes;
    std::size_t totalRoadsChecked{};
    std::size_t totalLanesChecked{};

    bool hasCongestion() const noexcept {
        return !congestedRoads.empty() || !congestedLanes.empty();
    }
};

class CongestionDetector {
public:
    explicit CongestionDetector(CongestionThresholds thresholds = {}) noexcept;

    void setThresholds(CongestionThresholds thresholds);
    const CongestionThresholds& thresholds() const noexcept;

    // Single-element inspection with safe division-by-zero checks
    bool isRoadCongested(const Road& road) const noexcept;
    bool isQueueCongested(const LaneQueue& queue) const noexcept;
    double roadOccupancyRatio(const Road& road) const noexcept;
    double queueOccupancyRatio(const LaneQueue& queue) const noexcept;

    // Network-wide inspection
    std::vector<RoadId> detectCongestedRoads(const RoadNetwork& network) const;
    std::vector<std::string> detectCongestedLanes(const VehicleManager& vehicles,
                                                const std::vector<EntryLane>& lanes) const;

    CongestionReport evaluate(const RoadNetwork& network,
                              const VehicleManager& vehicles,
                              const std::vector<EntryLane>& lanes) const;

private:
    CongestionThresholds thresholds_;
};

}  // namespace traffic
