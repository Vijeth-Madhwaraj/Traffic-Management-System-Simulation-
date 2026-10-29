#include "traffic/management/metrics_collector.hpp"

#include <limits>
#include <ostream>

namespace traffic {

void MetricsCollector::recordGenerated() {
    ++generated_;
}

void MetricsCollector::recordCompleted(Tick waitingTicks) {
    ++completed_;
    if (std::numeric_limits<Tick>::max() - totalWaitingTicks_ >= waitingTicks) {
        totalWaitingTicks_ += waitingTicks;
    } else {
        totalWaitingTicks_ = std::numeric_limits<Tick>::max();
    }
}

bool MetricsCollector::recordCompletedVehicle(VehicleId id, Tick waitingTicks) {
    if (id == 0 || completedVehicles_.find(id) != completedVehicles_.end()) {
        // Vehicle already counted as completed: prevent duplicate counting
        return false;
    }
    completedVehicles_.insert(id);
    recordCompleted(waitingTicks);
    return true;
}

void MetricsCollector::observeQueue(std::size_t queueLength) {
    if (queueLength > maximumQueueLength_) {
        maximumQueueLength_ = queueLength;
    }
}

void MetricsCollector::recordCongestionEvent() {
    ++congestionEvents_;
}

MetricsSnapshot MetricsCollector::snapshot(std::size_t unfinished) const {
    const double average = completed_ == 0 ? 0.0 : static_cast<double>(totalWaitingTicks_) / static_cast<double>(completed_);
    return {generated_, completed_, unfinished, average, maximumQueueLength_, congestionEvents_};
}

MetricsSnapshot MetricsCollector::snapshot() const {
    const std::size_t unfinished = generated_ > completed_ ? (generated_ - completed_) : 0;
    return snapshot(unfinished);
}

void MetricsCollector::writeCsv(std::ostream& output, std::size_t unfinished) const {
    const auto values = snapshot(unfinished);
    output << "generated,completed,unfinished,average_waiting_ticks,maximum_queue_length,congestion_events\n"
           << values.generated << ',' << values.completed << ',' << values.unfinished << ','
           << values.averageWaitingTicks << ',' << values.maximumQueueLength << ','
           << values.congestionEvents << '\n';
}

void MetricsCollector::writeCsv(std::ostream& output) const {
    writeCsv(output, generated_ > completed_ ? (generated_ - completed_) : 0);
}

void MetricsCollector::reset() noexcept {
    generated_ = 0;
    completed_ = 0;
    totalWaitingTicks_ = 0;
    maximumQueueLength_ = 0;
    congestionEvents_ = 0;
    completedVehicles_.clear();
}

}  // namespace traffic
