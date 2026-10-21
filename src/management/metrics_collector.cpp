#include "traffic/management/metrics_collector.hpp"

#include <ostream>

namespace traffic {

void MetricsCollector::recordGenerated() { ++generated_; }
void MetricsCollector::recordCompleted(Tick waitingTicks) { ++completed_; totalWaitingTicks_ += waitingTicks; }
void MetricsCollector::observeQueue(std::size_t queueLength) {
    if (queueLength > maximumQueueLength_) maximumQueueLength_ = queueLength;
}

MetricsSnapshot MetricsCollector::snapshot(std::size_t unfinished) const {
    const double average = completed_ == 0 ? 0.0 : static_cast<double>(totalWaitingTicks_) / completed_;
    return {generated_, completed_, unfinished, average, maximumQueueLength_};
}

void MetricsCollector::writeCsv(std::ostream& output, std::size_t unfinished) const {
    const auto values = snapshot(unfinished);
    output << "generated,completed,unfinished,average_waiting_ticks,maximum_queue_length\n"
           << values.generated << ',' << values.completed << ',' << values.unfinished << ','
           << values.averageWaitingTicks << ',' << values.maximumQueueLength << '\n';
}

}  // namespace traffic
