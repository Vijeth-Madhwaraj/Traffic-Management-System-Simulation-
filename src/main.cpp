#include "traffic/input/scenario.hpp"
#include "traffic/management/report_writer.hpp"
#include "traffic/simulation/simulation.hpp"

#include <exception>
#include <iostream>
#include <string>

namespace {
void help() {
    std::cout << "Traffic Management System (Simulation)\n"
                 "Usage: traffic_sim --scenario <file> [--report <text-file>] [--csv <csv-file>]\n"
                 "       traffic_sim --help\n"
                 "Runs a validated scenario and optionally writes performance reports.\n";
}
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        help();
        return 0;
    }

    std::string scenarioPath;
    std::string textReportPath;
    std::string csvReportPath;
    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        if ((arg == "--scenario" || arg == "--report" || arg == "--csv") && i + 1 < argc) {
            const std::string value(argv[++i]);
            if (arg == "--scenario") scenarioPath = value;
            else if (arg == "--report") textReportPath = value;
            else csvReportPath = value;
        } else {
            help();
            std::cerr << "[ERROR][Console] Invalid or incomplete argument: " << arg << '\n';
            return 2;
        }
    }
    if (scenarioPath.empty()) {
        help();
        std::cerr << "[ERROR][Console] Supply --scenario <file>.\n";
        return 2;
    }

    try {
        const auto scenario = traffic::ScenarioLoader::loadFile(scenarioPath);
        traffic::Simulation simulation(scenario.config);
        simulation.configureScenario(scenario);
        std::cout << "Seed: " << scenario.config.seed << "\nDuration (ticks): " << scenario.config.duration << '\n';
        simulation.run();
        const auto summary = simulation.inputSummary();
        std::cout << "Generated: " << summary.generated << "\nAdmitted to entry queues: " << summary.admitted
                  << "\nQueued: " << summary.queued << "\nDeferred: " << summary.pending
                  << "\nUnroutable: " << summary.unroutable << '\n';

        const auto metrics = simulation.metricsSnapshot();
        std::cout << traffic::ReportWriter::formatTextSummary(metrics);
        if (!textReportPath.empty() && !traffic::ReportWriter::writeTextFile(textReportPath, metrics)) {
            std::cerr << "[ERROR][Report] Could not write text report: " << textReportPath << '\n';
            return 1;
        }
        if (!csvReportPath.empty() && !traffic::ReportWriter::writeCsvFile(csvReportPath, metrics)) {
            std::cerr << "[ERROR][Report] Could not write CSV report: " << csvReportPath << '\n';
            return 1;
        }
        if (!textReportPath.empty()) std::cout << "Text report written: " << textReportPath << '\n';
        if (!csvReportPath.empty()) std::cout << "CSV report written: " << csvReportPath << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "[ERROR][Scenario] " << error.what() << '\n';
        return 1;
    }
}
