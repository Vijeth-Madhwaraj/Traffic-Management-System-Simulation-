#pragma once

#include "traffic/types.hpp"

#include <cstddef>
#include <iosfwd>

namespace traffic {

struct MetricsSnapshot {
    std::size_t generated{};
    std::size_t completed{};
    std::size_t unfinished{};
    double averageWaitingTicks{};
    std::size_t maximumQueueLength{};
};

class MetricsCollector {
public:
    void recordGenerated();
    void recordCompleted(Tick waitingTicks);
    void observeQueue(std::size_t queueLength);
    MetricsSnapshot snapshot(std::size_t unfinished) const;
    void writeCsv(std::ostream& output, std::size_t unfinished) const;

private:
    std::size_t generated_{};
    std::size_t completed_{};
    Tick totalWaitingTicks_{};
    std::size_t maximumQueueLength_{};
};

}  // namespace traffic
