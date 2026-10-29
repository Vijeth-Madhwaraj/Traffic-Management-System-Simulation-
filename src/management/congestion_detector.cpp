#include "traffic/management/congestion_detector.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace traffic {

CongestionDetector::CongestionDetector(CongestionThresholds thresholds) noexcept
    : thresholds_(thresholds) {
    if (thresholds_.roadOccupancyRatio < 0.0 || thresholds_.roadOccupancyRatio > 1.0 ||
        !std::isfinite(thresholds_.roadOccupancyRatio)) {
        thresholds_.roadOccupancyRatio = 0.8;
    }
    if (thresholds_.queueOccupancyRatio < 0.0 || thresholds_.queueOccupancyRatio > 1.0 ||
        !std::isfinite(thresholds_.queueOccupancyRatio)) {
        thresholds_.queueOccupancyRatio = 0.8;
    }
}

void CongestionDetector::setThresholds(CongestionThresholds thresholds) {
    if (thresholds.roadOccupancyRatio < 0.0 || thresholds.roadOccupancyRatio > 1.0 ||
        !std::isfinite(thresholds.roadOccupancyRatio)) {
        throw std::invalid_argument("Road occupancy ratio threshold must be between 0.0 and 1.0");
    }
    if (thresholds.queueOccupancyRatio < 0.0 || thresholds.queueOccupancyRatio > 1.0 ||
        !std::isfinite(thresholds.queueOccupancyRatio)) {
        throw std::invalid_argument("Queue occupancy ratio threshold must be between 0.0 and 1.0");
    }
    thresholds_ = thresholds;
}

const CongestionThresholds& CongestionDetector::thresholds() const noexcept {
    return thresholds_;
}

double CongestionDetector::roadOccupancyRatio(const Road& road) const noexcept {
    if (road.capacity == 0) return 0.0;
    return static_cast<double>(road.occupancy) / static_cast<double>(road.capacity);
}

double CongestionDetector::queueOccupancyRatio(const LaneQueue& queue) const noexcept {
    if (queue.capacity() == 0) return 0.0;
    return static_cast<double>(queue.size()) / static_cast<double>(queue.capacity());
}

bool CongestionDetector::isRoadCongested(const Road& road) const noexcept {
    if (road.capacity == 0 || road.occupancy == 0) return false;
    const double ratio = roadOccupancyRatio(road);
    // Boundary check: ratio >= threshold
    return ratio >= (thresholds_.roadOccupancyRatio - 1e-9);
}

bool CongestionDetector::isQueueCongested(const LaneQueue& queue) const noexcept {
    if (queue.empty() || queue.capacity() == 0) return false;

    if (thresholds_.queueLengthThreshold > 0 && queue.size() >= thresholds_.queueLengthThreshold) {
        return true;
    }

    const double ratio = queueOccupancyRatio(queue);
    return ratio >= (thresholds_.queueOccupancyRatio - 1e-9);
}

std::vector<RoadId> CongestionDetector::detectCongestedRoads(const RoadNetwork& network) const {
    std::vector<RoadId> result;
    for (const auto& [id, road] : network.roads()) {
        if (isRoadCongested(road)) {
            result.push_back(id);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

std::vector<std::string> CongestionDetector::detectCongestedLanes(
    const VehicleManager& vehicles, const std::vector<EntryLane>& lanes) const {
    std::vector<std::string> result;
    for (const auto& laneSpec : lanes) {
        const auto* q = vehicles.lane(laneSpec.id);
        if (q && isQueueCongested(*q)) {
            result.push_back(laneSpec.id);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

CongestionReport CongestionDetector::evaluate(
    const RoadNetwork& network, const VehicleManager& vehicles,
    const std::vector<EntryLane>& lanes) const {
    CongestionReport report;
    report.congestedRoads = detectCongestedRoads(network);
    report.congestedLanes = detectCongestedLanes(vehicles, lanes);
    report.totalRoadsChecked = network.roads().size();
    report.totalLanesChecked = lanes.size();
    return report;
}

}  // namespace traffic
