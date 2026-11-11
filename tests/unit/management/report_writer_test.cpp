
#include "traffic/management/report_writer.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

namespace traffic {

TEST(ReportWriterTest, WritesCsvHeader) {
    SimulationReport report;
    std::ostringstream output;

    ReportWriter::writeCsv(output, report);

    EXPECT_NE(output.str().find("metric,value"), std::string::npos);
}

TEST(ReportWriterTest, WritesSimulationCounts) {
    SimulationReport report;

    report.finalTick = 50;
    report.generated = 10;
    report.admitted = 7;
    report.queued = 3;
    report.pending = 1;
    report.unroutable = 2;

    std::ostringstream output;
    ReportWriter::writeCsv(output, report);

    const std::string csv = output.str();

    EXPECT_NE(csv.find("final_tick,50"), std::string::npos);
    EXPECT_NE(csv.find("generated,10"), std::string::npos);
    EXPECT_NE(csv.find("admitted,7"), std::string::npos);
    EXPECT_NE(csv.find("queued,3"), std::string::npos);
    EXPECT_NE(csv.find("pending,1"), std::string::npos);
    EXPECT_NE(csv.find("unroutable,2"), std::string::npos);
}

TEST(ReportWriterTest, WritesPerformanceAndIncidentMetrics) {
    SimulationReport report;

    report.metrics.completed = 2;
    report.metrics.unfinished = 5;
    report.metrics.averageWaitingTicks = 4.5;
    report.metrics.maximumQueueLength = 8;

    report.congestedRoads = 2;
    report.congestedLanes = 1;
    report.activeIncidents = 3;

    std::ostringstream output;
    ReportWriter::writeCsv(output, report);

    const std::string csv = output.str();

    EXPECT_NE(csv.find("completed,2"), std::string::npos);
    EXPECT_NE(csv.find("unfinished,5"), std::string::npos);
    EXPECT_NE(csv.find("average_waiting_ticks,4.50"), std::string::npos);
    EXPECT_NE(csv.find("maximum_queue_length,8"), std::string::npos);
    EXPECT_NE(csv.find("congested_roads,2"), std::string::npos);
    EXPECT_NE(csv.find("congested_lanes,1"), std::string::npos);
    EXPECT_NE(csv.find("active_incidents,3"), std::string::npos);
}

} // namespace traffic
