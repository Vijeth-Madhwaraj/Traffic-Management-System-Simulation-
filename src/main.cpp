#include "traffic/input/scenario.hpp"
#include "traffic/simulation/simulation.hpp"

#include <exception>
#include <iostream>
#include <string>

namespace {
void help() {
    std::cout << "Traffic Management System (Simulation)\n"
                 "Usage: traffic_sim --scenario <file> [--lane-status]\n"
                 "       traffic_sim --validate <file>\n"
                 "       traffic_sim --help\n"
                 "Loads validated text scenarios and reports generated/queued/deferred arrivals.\n";
}
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        help();
        return 0;
    }
    const bool validateOnly = argc == 3 && std::string(argv[1]) == "--validate";
    const bool laneStatus = argc == 4 && std::string(argv[3]) == "--lane-status";
    const bool runScenario = argc >= 2 && std::string(argv[1]) == "--scenario" &&
                             (argc == 3 || laneStatus);
    if (!validateOnly && !runScenario) {
        help();
        std::cerr << "[ERROR][Console] Supply --scenario <file>, --validate <file>, or --help.\n";
        return 2;
    }
    try {
        const auto scenario = traffic::ScenarioLoader::loadFile(argv[2]);
        if (validateOnly) {
            std::cout << "Scenario valid\nSeed: " << scenario.config.seed
                      << "\nDuration (ticks): " << scenario.config.duration
                      << "\nIntersections: " << scenario.intersections.size()
                      << "\nRoads: " << scenario.roads.size()
                      << "\nEntry lanes: " << scenario.lanes.size()
                      << "\nScheduled arrivals: " << scenario.arrivals.size()
                      << "\nArrival streams: " << scenario.streams.size() << '\n';
            return 0;
        }
        traffic::Simulation simulation(scenario.config);
        simulation.configureScenario(scenario);
        std::cout << "Seed: " << scenario.config.seed << "\nDuration (ticks): " << scenario.config.duration << '\n';
        simulation.run();
        const auto summary = simulation.inputSummary();
        std::cout << "Generated: " << summary.generated << "\nAdmitted to entry queues: " << summary.admitted
                  << "\nQueued: " << summary.queued << "\nDeferred: " << summary.pending
                  << "\nUnroutable: " << summary.unroutable << '\n'
                  << "Movement and automatic routing are pending implementation; trip completion is not verified.\n";
        if (laneStatus) {
            for (const auto& lane : simulation.vehicles().laneSnapshots()) {
                std::cout << "Lane " << lane.id << " (road " << lane.road << "): queued="
                          << lane.queued.size() << '/' << lane.capacity
                          << ", deferred=" << lane.pending.size() << "\n  Queued IDs:";
                for (const auto id : lane.queued) std::cout << ' ' << id;
                std::cout << "\n  Deferred IDs:";
                for (const auto id : lane.pending) std::cout << ' ' << id;
                std::cout << '\n';
            }
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "[ERROR][Scenario] " << error.what() << '\n';
        return 1;
    }
}
