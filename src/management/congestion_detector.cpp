
#include "traffic/management/congestion_detector.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace traffic {

CongestionDetector::CongestionDetector(
    CongestionThresholds thresholds)
    : thresholds_(thresholds) {
    setThresholds(thresholds);
}

void CongestionDetector::setThresholds(
    CongestionThresholds thresholds) {
    if (!std::isfinite(thresholds.roadOccupancyRatio) ||
        thresholds.roadOccupancyRatio < 0.0 ||
        thresholds.roadOccupancyRatio > 1.0) {
        throw std::invalid_argument(
            "Road occupancy ratio threshold must be between 0.0 and 1.0");
    }

    if (!std::isfinite(thresholds.queueOccupancyRatio) ||
        thresholds.queueOccupancyRatio < 0.0 ||
        thresholds.queueOccupancyRatio > 1.0) {
        throw std::invalid_argument(
            "Queue occupancy ratio threshold must be between 0.0 and 1.0");
    }

    thresholds_ = thresholds;
}

const CongestionThresholds&
CongestionDetector::thresholds() const noexcept {
    return thresholds_;
}

double CongestionDetector::roadOccupancyRatio(
    const Road& road) const noexcept {
    if (road.capacity == 0) {
        return 0.0;
    }

    return static_cast<double>(road.occupancy) /
           static_cast<double>(road.capacity);
}

double CongestionDetector::queueOccupancyRatio(
    const LaneQueue& queue) const noexcept {
    if (queue.capacity() == 0) {
        return 0.0;
    }

    return static_cast<double>(queue.size()) /
           static_cast<double>(queue.capacity());
}

bool CongestionDetector::isRoadCongested(
    const Road& road) const noexcept {
    if (road.capacity == 0 || road.occupancy == 0) {
        return false;
    }

    return roadOccupancyRatio(road) >=
           (thresholds_.roadOccupancyRatio - 1e-9);
}

bool CongestionDetector::isQueueCongested(
    const LaneQueue& queue) const noexcept {
    if (queue.empty() || queue.capacity() == 0) {
        return false;
    }

    if (thresholds_.queueLengthThreshold > 0 &&
        queue.size() >= thresholds_.queueLengthThreshold) {
        return true;
    }

    return queueOccupancyRatio(queue) >=
           (thresholds_.queueOccupancyRatio - 1e-9);
}

std::vector<RoadId> CongestionDetector::detectCongestedRoads(
    const RoadNetwork& network) const {
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
    const VehicleManager& vehicles,
    const std::vector<EntryLane>& lanes) const {
    std::vector<std::string> result;

    for (const auto& laneSpec : lanes) {
        const auto* queue = vehicles.lane(laneSpec.id);

        if (queue != nullptr && isQueueCongested(*queue)) {
            result.push_back(laneSpec.id);
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

CongestionReport CongestionDetector::evaluate(
    const RoadNetwork& network,
    const VehicleManager& vehicles,
    const std::vector<EntryLane>& lanes) const {
    CongestionReport report;

    report.congestedRoads = detectCongestedRoads(network);
    report.congestedLanes = detectCongestedLanes(vehicles, lanes);
    report.totalRoadsChecked = network.roads().size();
    report.totalLanesChecked = lanes.size();

    return report;
}

}  // namespace traffic
