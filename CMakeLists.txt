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

include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    # GoogleTest is fetched at configure time when BUILD_TESTING is enabled.
    # Pin a release tag so local and CI builds use the same test framework.
    set(gtest_force_shared_crt ON CACHE BOOL "Use the shared MSVC runtime" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "Do not build Google Mock for these unit tests" FORCE)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.15.2
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)

    add_executable(traffic_signal_tests
        tests/unit/signals/signal_controller_test.cpp
    )
    target_link_libraries(traffic_signal_tests PRIVATE traffic_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(traffic_signal_tests DISCOVERY_MODE PRE_TEST)
endif()
