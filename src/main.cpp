
#include "traffic/input/scenario.hpp"
#include "traffic/management/report_writer.hpp"
#include "traffic/simulation/simulation.hpp"

#include <exception>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void help() {
    std::cout
        << "Traffic Management System (Simulation)\n"
        << "Usage:\n"
        << "  traffic_sim --scenario <file>\n"
        << "  traffic_sim --scenario <file> --report <output.csv>\n"
        << "  traffic_sim --help\n"
        << "Loads a scenario, runs the simulation and displays performance metrics.\n"
        << "The optional --report argument exports results as CSV.\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        help();
        return 0;
    }

    std::string scenarioPath;
    std::string reportPath;

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];

        if (argument == "--scenario" && index + 1 < argc) {
            scenarioPath = argv[++index];
        } else if (argument == "--report" && index + 1 < argc) {
            reportPath = argv[++index];
        } else {
            help();
            std::cerr
                << "[ERROR][Console] Invalid arguments. "
                << "Supply --scenario <file>, optionally with "
                << "--report <output.csv>, or use --help.\n";
            return 2;
        }
    }

    if (scenarioPath.empty()) {
        help();
        std::cerr
            << "[ERROR][Console] A scenario file is required.\n";
        return 2;
    }

    try {
        const auto scenario =
            traffic::ScenarioLoader::loadFile(scenarioPath);

        traffic::Simulation simulation(scenario.config);

        simulation.configureScenario(scenario);

        std::cout
            << "Seed: " << scenario.config.seed
            << "\nDuration (ticks): "
            << scenario.config.duration << '\n';

        simulation.run();

        const auto summary = simulation.inputSummary();

        std::cout
            << "Generated: " << summary.generated
            << "\nAdmitted to entry queues: " << summary.admitted
            << "\nQueued: " << summary.queued
            << "\nDeferred: " << summary.pending
            << "\nUnroutable: " << summary.unroutable
            << '\n';

        const auto metrics = simulation.metricsSnapshot();

        std::cout
            << "Completed: " << metrics.completed
            << "\nUnfinished: " << metrics.unfinished
            << "\nAverage waiting ticks: "
            << metrics.averageWaitingTicks
            << "\nMaximum queue length: "
            << metrics.maximumQueueLength
            << '\n';

        std::cout
            << "Movement is not implemented; "
            << "completed-trip metrics are not yet available.\n";

        if (!reportPath.empty()) {
            std::ofstream reportFile(reportPath);

            if (!reportFile.is_open()) {
                throw std::runtime_error(
                    "Unable to open report file: " + reportPath
                );
            }

            traffic::ReportWriter::writeCsv(
                reportFile,
                simulation.createReport()
            );

            if (!reportFile.good()) {
                throw std::runtime_error(
                    "Failed to write report file: " + reportPath
                );
            }

            reportFile.close();

            std::cout
                << "CSV report saved to: "
                << reportPath << '\n';
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr
            << "[ERROR][Scenario] "
            << error.what() << '\n';

        return 1;
    }
}
