#pragma once

#include "traffic/management/metrics_collector.hpp"

#include <iosfwd>
#include <string>

namespace traffic {

class ReportWriter {
public:
    // Format text summary as a formatted string
    static std::string formatTextSummary(const MetricsSnapshot& snapshot);

    // Stream writers with error handling
    static bool writeTextSummary(std::ostream& output, const MetricsSnapshot& snapshot);
    static bool writeCsv(std::ostream& output, const MetricsSnapshot& snapshot);

    // File export utilities returning true on success, false on I/O failure
    static bool writeTextFile(const std::string& filePath, const MetricsSnapshot& snapshot);
    static bool writeCsvFile(const std::string& filePath, const MetricsSnapshot& snapshot);
};

}  // namespace traffic
