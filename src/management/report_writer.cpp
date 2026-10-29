#include "traffic/management/report_writer.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>

namespace traffic {

std::string ReportWriter::formatTextSummary(const MetricsSnapshot& snapshot) {
    std::ostringstream ss;
    ss << "==================================================\n"
       << "       TRAFFIC SIMULATION PERFORMANCE REPORT      \n"
       << "==================================================\n"
       << "Vehicles Generated       : " << snapshot.generated << '\n'
       << "Vehicles Completed       : " << snapshot.completed << '\n'
       << "Vehicles Unfinished      : " << snapshot.unfinished << '\n'
       << "Average Waiting Time     : " << std::fixed << std::setprecision(2)
       << snapshot.averageWaitingTicks << " ticks\n"
       << "Maximum Queue Length     : " << snapshot.maximumQueueLength << '\n'
       << "Congestion Events        : " << snapshot.congestionEvents << '\n'
       << "==================================================\n";
    return ss.str();
}

bool ReportWriter::writeTextSummary(std::ostream& output, const MetricsSnapshot& snapshot) {
    if (!output.good()) {
        return false;
    }
    output << formatTextSummary(snapshot);
    return output.good();
}

bool ReportWriter::writeCsv(std::ostream& output, const MetricsSnapshot& snapshot) {
    if (!output.good()) {
        return false;
    }
    output << "generated,completed,unfinished,average_waiting_ticks,maximum_queue_length,congestion_events\n"
           << snapshot.generated << ','
           << snapshot.completed << ','
           << snapshot.unfinished << ','
           << std::fixed << std::setprecision(2) << snapshot.averageWaitingTicks << ','
           << snapshot.maximumQueueLength << ','
           << snapshot.congestionEvents << '\n';
    return output.good();
}

bool ReportWriter::writeTextFile(const std::string& filePath, const MetricsSnapshot& snapshot) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    const bool success = writeTextSummary(file, snapshot);
    file.close();
    return success;
}

bool ReportWriter::writeCsvFile(const std::string& filePath, const MetricsSnapshot& snapshot) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    const bool success = writeCsv(file, snapshot);
    file.close();
    return success;
}

}  // namespace traffic
