#include "traffic/simulation/simulation.hpp"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--help") {
        std::cout << "Traffic Management System (Simulation)\n"
                     "Usage: traffic_sim [--help]\n"
                     "Scenario loading and simulation options are not implemented yet.\n";
        return 0;
    }

    std::cout << "Traffic simulation skeleton. See README.md for the project roadmap.\n";
    return 0;
}
