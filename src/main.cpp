#include "traffic/input/scenario.hpp"
#include "traffic/simulation/simulation.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
void help() {
    std::cout << "Traffic Management System (Simulation)\n"
                 "Usage: traffic_sim --scenario <file> [--show-scenario] [--lane-status] [--trace]\n"
                 "       traffic_sim --validate <file>\n"
                 "       traffic_sim --help\n"
                 "--show-scenario prints the configured network and traffic without running it.\n"
                 "--trace prints the scenario overview, then each tick's signals, vehicle movement, waits and deferrals.\n";
}

const char* colorName(traffic::SignalColor color) {
    switch (color) {
        case traffic::SignalColor::Green: return "GREEN";
        case traffic::SignalColor::Yellow: return "YELLOW";
        case traffic::SignalColor::Red: return "RED";
    }
    return "UNKNOWN";
}

const char* stateName(traffic::VehicleState state) {
    switch (state) {
        case traffic::VehicleState::Waiting: return "waiting";
        case traffic::VehicleState::InTransit: return "in transit";
        case traffic::VehicleState::Completed: return "completed";
        case traffic::VehicleState::Unroutable: return "unroutable";
    }
    return "unknown";
}

const char* className(traffic::VehicleClass vehicleClass) {
    return vehicleClass == traffic::VehicleClass::Emergency ? "emergency" : "standard";
}

std::string vehicleCount(std::size_t count) {
    return std::to_string(count) + (count == 1 ? " vehicle" : " vehicles");
}

void printRoute(const std::vector<traffic::RoadId>& route) {
    if (route.empty()) {
        std::cout << "automatic route";
        return;
    }
    for (std::size_t index = 0; index < route.size(); ++index) {
        if (index > 0) std::cout << " -> ";
        std::cout << route[index];
    }
}

void printScenario(const traffic::Scenario& scenario, const std::string& path) {
    std::cout << "Scenario: " << std::filesystem::path(path).filename().string() << '\n'
              << "Seed: " << scenario.config.seed << " | Duration: " << scenario.config.duration << " ticks\n"
              << "Network: " << scenario.intersections.size() << " intersections, "
              << scenario.roads.size() << " roads\n  Roads:\n";
    for (const auto& road : scenario.roads) {
        std::cout << "    " << road.id << ": " << road.from << " -> " << road.to
                  << " (capacity " << road.capacity << ", travel " << road.travelTime << " tick(s))\n";
    }

    std::cout << "  Entry lanes:\n";
    for (const auto& lane : scenario.lanes) {
        std::cout << "    " << lane.id << " -> road " << lane.road
                  << " (queue capacity " << lane.capacity << ")\n";
    }

    std::cout << "  Scheduled arrivals: " << scenario.arrivals.size() << '\n';
    for (const auto& arrival : scenario.arrivals) {
        std::cout << "    tick " << arrival.tick << ": " << arrival.lane << ", "
                  << className(arrival.vehicle.vehicleClass) << ", route ";
        printRoute(arrival.vehicle.route);
        std::cout << '\n';
    }
    std::cout << "  Arrival streams:\n";
    if (scenario.streams.empty()) std::cout << "    none\n";
    for (const auto& stream : scenario.streams) {
        std::cout << "    " << stream.id << ": lane " << stream.lane << ", ticks ["
                  << stream.firstTick << ',' << stream.endTick << "), " << std::fixed
                  << std::setprecision(1) << stream.probability * 100.0 << "% per tick, batch "
                  << stream.vehiclesPerArrival << ", destination " << stream.vehicle.destination
                  << ", route ";
        printRoute(stream.vehicle.route);
        std::cout << '\n' << std::defaultfloat;
    }

    std::cout << "  Signal plans:\n";
    if (scenario.signals.empty()) std::cout << "    none\n";
    for (const auto& plan : scenario.signals) {
        std::cout << "    " << plan.intersection << ":\n";
        for (std::size_t index = 0; index < plan.phases.size(); ++index) {
            const auto& phase = plan.phases[index];
            std::cout << "      phase " << index + 1 << " (green " << phase.greenDuration
                      << ", yellow " << phase.yellowDuration << ", all-red " << phase.allRedDuration
                      << "): ";
            for (std::size_t movement = 0; movement < phase.permittedMovements.size(); ++movement) {
                if (movement > 0) std::cout << ", ";
                std::cout << phase.permittedMovements[movement];
            }
            std::cout << '\n';
        }
    }
}

void printIds(const char* label, const std::vector<traffic::VehicleId>& ids) {
    std::cout << label;
    if (ids.empty()) std::cout << " none";
    for (const auto id : ids) std::cout << ' ' << id;
    std::cout << '\n';
}

void printTraceTick(traffic::Simulation& simulation,
                    const traffic::AdmissionBatch& arrivals,
                    const std::vector<traffic::Vehicle>& previousVehicles,
                    traffic::Tick tick) {
    std::cout << "\nTick " << tick << '\n';
    printIds("  Generated:", arrivals.generated);
    printIds("  Admitted to entry queue:", arrivals.admitted);
    printIds("  Unroutable:", arrivals.unroutable);

    const auto signalStatuses = simulation.signalSnapshots(tick);
    for (const auto& entry : signalStatuses) {
        const auto& status = entry.second;
        std::cout << "  Signal " << entry.first << ": phase " << status.phaseIndex + 1
                  << ' ' << colorName(status.color);
        if (!status.permittedMovements.empty()) {
            std::cout << " (permits";
            for (const auto& movement : status.permittedMovements) std::cout << ' ' << movement;
            std::cout << ')';
        }
        std::cout << '\n';
    }

    const auto vehicles = simulation.vehicles().vehicleSnapshots();
    const auto lanes = simulation.vehicles().laneSnapshots();
    const auto completed = std::count_if(vehicles.begin(), vehicles.end(), [](const auto& vehicle) {
        return vehicle.state == traffic::VehicleState::Completed;
    });
    std::vector<std::string> roadEvents;
    std::vector<std::string> waitEvents;
    std::vector<std::string> completedEvents;
    std::size_t queuedCount = 0;
    std::size_t deferredCount = 0;
    std::cout << "  Reached destination so far: " << completed << '\n';
    std::cout << "  Vehicles:\n";
    if (vehicles.empty()) std::cout << "    none\n";
    for (const auto& vehicle : vehicles) {
        const auto previous = std::find_if(previousVehicles.begin(), previousVehicles.end(),
            [&](const traffic::Vehicle& candidate) { return candidate.id == vehicle.id; });
        std::cout << "    #" << vehicle.id << ": ";
        if (vehicle.state == traffic::VehicleState::InTransit && vehicle.routeIndex < vehicle.route.size()) {
            const auto& road = vehicle.route[vehicle.routeIndex];
            if (previous == previousVehicles.end() || previous->state == traffic::VehicleState::Waiting) {
                std::cout << "entered road " << road;
                roadEvents.push_back("vehicle #" + std::to_string(vehicle.id) + " enters " + road);
            } else if (vehicle.routeIndex > previous->routeIndex && previous->routeIndex < previous->route.size()) {
                std::cout << "moved " << previous->route[previous->routeIndex] << " -> " << road;
                roadEvents.push_back("vehicle #" + std::to_string(vehicle.id) + " moves from " +
                                     previous->route[previous->routeIndex] + " to " + road);
            } else {
                std::cout << "traveling on " << road;
            }
            if (previous != previousVehicles.end() && vehicle.waitingTicks > previous->waitingTicks) {
                waitEvents.push_back("vehicle #" + std::to_string(vehicle.id) + " waits at a downstream limit");
            }
        } else {
            if (vehicle.state == traffic::VehicleState::Waiting) {
                const bool deferred = std::any_of(lanes.begin(), lanes.end(), [&](const auto& lane) {
                    return std::find(lane.pending.begin(), lane.pending.end(), vehicle.id) != lane.pending.end();
                });
                std::cout << (deferred ? "deferred (waiting)" : "waiting in entry queue");
                if (deferred) ++deferredCount;
                else ++queuedCount;
            } else {
                std::cout << stateName(vehicle.state);
                if (vehicle.state == traffic::VehicleState::Completed &&
                    (previous == previousVehicles.end() || previous->state != traffic::VehicleState::Completed)) {
                    completedEvents.push_back("vehicle #" + std::to_string(vehicle.id) + " reaches destination");
                }
            }
        }
        if (vehicle.waitingTicks > 0) std::cout << "; accumulated wait " << vehicle.waitingTicks << " tick(s)";
        std::cout << '\n';
    }

    for (const auto& lane : lanes) {
        std::cout << "  Lane " << lane.id << " queued=" << lane.queued.size() << '/' << lane.capacity;
        printIds("; deferred:", lane.pending);
    }

    std::ostringstream explanation;
    explanation << "  In plain English: Tick " << tick << ": ";
    if (arrivals.generated.empty()) explanation << "no new vehicles arrive";
    else explanation << vehicleCount(arrivals.generated.size()) << " arrive";
    if (!arrivals.admitted.empty()) {
        explanation << "; " << vehicleCount(arrivals.admitted.size()) << " are admitted to the road network";
    }
    const auto appendEvents = [&explanation](const std::vector<std::string>& events) {
        for (const auto& event : events) explanation << "; " << event;
    };
    appendEvents(roadEvents);
    appendEvents(waitEvents);
    appendEvents(completedEvents);
    if (queuedCount > 0) explanation << "; " << queuedCount << " wait in entry lanes";
    if (deferredCount > 0) explanation << "; " << deferredCount << " are deferred because entry lanes are full";
    for (const auto& entry : signalStatuses) {
        explanation << "; " << entry.first << " is " << colorName(entry.second.color);
        if (!entry.second.permittedMovements.empty()) {
            explanation << " for ";
            for (std::size_t index = 0; index < entry.second.permittedMovements.size(); ++index) {
                if (index > 0) explanation << ", ";
                explanation << entry.second.permittedMovements[index];
            }
        }
    }
    explanation << "; " << vehicleCount(static_cast<std::size_t>(completed))
                << (completed == 1 ? " has" : " have") << " reached the destination so far.\n";
    std::cout << explanation.str();
}
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        help();
        return 0;
    }
    const bool validateOnly = argc == 3 && std::string(argv[1]) == "--validate";
    bool laneStatus = false;
    bool trace = false;
    bool showScenario = false;
    if (argc >= 3 && std::string(argv[1]) == "--scenario") {
        for (int index = 3; index < argc; ++index) {
            const std::string option(argv[index]);
            if (option == "--lane-status" && !laneStatus) laneStatus = true;
            else if (option == "--trace" && !trace) trace = true;
            else if (option == "--show-scenario" && !showScenario) showScenario = true;
            else {
                help();
                std::cerr << "[ERROR][Console] Unknown or repeated scenario option: " << option << '\n';
                return 2;
            }
        }
    }
    const bool runScenario = argc >= 3 && std::string(argv[1]) == "--scenario";
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
        if (showScenario || trace) printScenario(scenario, argv[2]);
        if (showScenario && !trace && !laneStatus) return 0;
        traffic::Simulation simulation(scenario.config);
        simulation.configureScenario(scenario);
        if (!trace && !showScenario) {
            std::cout << "Seed: " << scenario.config.seed << "\nDuration (ticks): "
                      << scenario.config.duration << '\n';
        }
        if (trace) {
            while (simulation.currentTick() < scenario.config.duration) {
                const auto previousVehicles = simulation.vehicles().vehicleSnapshots();
                const auto arrivals = simulation.step();
                printTraceTick(simulation, arrivals, previousVehicles, simulation.currentTick() - 1);
            }
        } else {
            simulation.run();
        }
        const auto summary = simulation.inputSummary();
        const auto vehicles = simulation.vehicles().vehicleSnapshots();
        const auto completed = std::count_if(vehicles.begin(), vehicles.end(), [](const auto& vehicle) {
            return vehicle.state == traffic::VehicleState::Completed;
        });
        const auto inTransit = std::count_if(vehicles.begin(), vehicles.end(), [](const auto& vehicle) {
            return vehicle.state == traffic::VehicleState::InTransit;
        });
        std::cout << "\nGenerated: " << summary.generated << "\nAdmitted to entry queues: " << summary.admitted
                  << "\nQueued: " << summary.queued << "\nDeferred: " << summary.pending
                  << "\nReached destination: " << completed
                  << "\nIn transit: " << inTransit
                  << "\nUnroutable: " << summary.unroutable << '\n'
                  << "Explicit-route movement is enabled; automatic routing and detailed trip metrics remain pending.\n";
        if (laneStatus && !trace) {
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
