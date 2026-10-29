#include "traffic/management/report_writer.hpp"
#include <gtest/gtest.h>
#include <sstream>
using namespace traffic;

TEST(ReportWriterTest, WritesCsvHeaderAndValues) {
    std::ostringstream output;
    const MetricsSnapshot snapshot{4, 2, 2, 3.5, 7, 1};
    ASSERT_TRUE(ReportWriter::writeCsv(output, snapshot));
    EXPECT_NE(output.str().find("generated,completed,unfinished,average_waiting_ticks,maximum_queue_length,congestion_events"), std::string::npos);
    EXPECT_NE(output.str().find("4,2,2,3.50,7,1"), std::string::npos);
}

TEST(ReportWriterTest, FormatsTextSummary) {
    const MetricsSnapshot snapshot{1, 1, 0, 2.0, 1, 0};
    const auto text = ReportWriter::formatTextSummary(snapshot);
    EXPECT_NE(text.find("TRAFFIC SIMULATION PERFORMANCE REPORT"), std::string::npos);
    EXPECT_NE(text.find("Vehicles Completed       : 1"), std::string::npos);
}
