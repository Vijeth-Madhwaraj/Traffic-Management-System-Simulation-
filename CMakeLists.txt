cmake_minimum_required(VERSION 3.16)
project(TrafficManagementSimulation VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(traffic_core
    src/vehicle/vehicle_manager.cpp
    src/vehicle/lane_queue.cpp
    src/signals/signal_controller.cpp
    src/network/road_network.cpp
    src/network/movement_engine.cpp
    src/management/incident_manager.cpp
    src/management/metrics_collector.cpp
    src/simulation/simulation.cpp
)
target_include_directories(traffic_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(traffic_core PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic>
)

add_executable(traffic_sim src/main.cpp)
target_link_libraries(traffic_sim PRIVATE traffic_core)

# Tests are intentionally left as a starter area. Add GoogleTest and register
# module-level tests here once the team agrees on the shared interfaces.
