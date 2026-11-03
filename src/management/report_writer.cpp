
#include "traffic/management/report_writer.hpp"

#include <iomanip>
#include <ostream>

namespace traffic {

void ReportWriter::writeCsv(
    std::ostream& output,
    const SimulationReport& report
) {
    output
        << "metric,value\n"
        << "final_tick," << report.finalTick << '\n'
        << "generated," << report.generated << '\n'
        << "admitted," << report.admitted << '\n'
        << "queued," << report.queued << '\n'
        << "pending," << report.pending << '\n'
        << "unroutable," << report.unroutable << '\n'
        << "completed," << report.metrics.completed << '\n'
        << "unfinished," << report.metrics.unfinished << '\n'
        << std::fixed << std::setprecision(2)
        << "average_waiting_ticks,"
        << report.metrics.averageWaitingTicks << '\n'
        << "maximum_queue_length,"
        << report.metrics.maximumQueueLength << '\n'
        << "congested_roads," << report.congestedRoads << '\n'
        << "congested_lanes," << report.congestedLanes << '\n'
        << "active_incidents," << report.activeIncidents << '\n';
}

}  // namespace traffic
