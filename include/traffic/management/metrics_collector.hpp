#pragma once

#include "traffic/types.hpp"

#include <cstddef>
#include <iosfwd>
#include <unordered_set>

namespace traffic {

struct MetricsSnapshot {
    std::size_t generated{};
    std::size_t completed{};
    std::size_t unfinished{};
    double averageWaitingTicks{};
    std::size_t maximumQueueLength{};
    std::size_t congestionEvents{};
};

class MetricsCollector {
public:
    void recordGenerated();
    void recordCompleted(Tick waitingTicks);
    bool recordCompletedVehicle(VehicleId id, Tick waitingTicks);
    void observeQueue(std::size_t queueLength);
    void recordCongestionEvent();

    MetricsSnapshot snapshot(std::size_t unfinished) const;
    MetricsSnapshot snapshot() const;

    void writeCsv(std::ostream& output, std::size_t unfinished) const;
    void writeCsv(std::ostream& output) const;

    std::size_t generated() const noexcept { return generated_; }
    std::size_t completed() const noexcept { return completed_; }
    Tick totalWaitingTicks() const noexcept { return totalWaitingTicks_; }
    std::size_t maximumQueueLength() const noexcept { return maximumQueueLength_; }
    std::size_t congestionEvents() const noexcept { return congestionEvents_; }
    void reset() noexcept;

private:
    std::size_t generated_{};
    std::size_t completed_{};
    Tick totalWaitingTicks_{};
    std::size_t maximumQueueLength_{};
    std::size_t congestionEvents_{};
    std::unordered_set<VehicleId> completedVehicles_;
};

}  // namespace traffic
