
#pragma once

#include "traffic/management/metrics_collector.hpp"
#include "traffic/types.hpp"

#include <cstddef>
#include <iosfwd>

namespace traffic {

struct SimulationReport {
    Tick finalTick{};
    std::uint64_t seed{};

    std::size_t generated{};
    std::size_t admitted{};
    std::size_t queued{};
    std::size_t pending{};
    std::size_t unroutable{};

    MetricsSnapshot metrics;

    std::size_t congestedRoads{};
    std::size_t congestedLanes{};
    std::size_t activeIncidents{};
};

class ReportWriter {
public:
    static void writeCsv(
        std::ostream& output,
        const SimulationReport& report
    );
};

}  // namespace traffic
