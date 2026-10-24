cmake_minimum_required(VERSION 3.16)
project(TrafficManagementSimulation VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(traffic_core
    src/input/scenario.cpp
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

option(TRAFFIC_ENABLE_ANALYZER "Run Clang static analysis and treat project warnings as errors" OFF)
if(TRAFFIC_ENABLE_ANALYZER)
    find_program(TRAFFIC_CLANGXX NAMES clang++)
    if(NOT TRAFFIC_CLANGXX)
        message(FATAL_ERROR "TRAFFIC_ENABLE_ANALYZER requires clang++")
    endif()
    foreach(target traffic_core traffic_sim)
        target_compile_options(${target} PRIVATE
            $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Werror>)
    endforeach()
    add_custom_target(traffic_analyze ALL)
    get_target_property(analysis_sources traffic_core SOURCES)
    list(APPEND analysis_sources src/main.cpp)
    foreach(source IN LISTS analysis_sources)
        get_filename_component(source_name "${source}" NAME_WE)
        add_custom_command(TARGET traffic_analyze POST_BUILD
            COMMAND ${TRAFFIC_CLANGXX} --analyze -std=c++17
                -I${CMAKE_CURRENT_SOURCE_DIR}/include -Wall -Wextra -Wpedantic -Werror
                -Xanalyzer -analyzer-werror ${CMAKE_CURRENT_SOURCE_DIR}/${source}
                -o ${CMAKE_CURRENT_BINARY_DIR}/analysis-${source_name}.plist
            VERBATIM)
    endforeach()
endif()

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

    add_executable(traffic_vehicle_tests
        tests/unit/vehicle/vehicle_manager_test.cpp
        tests/unit/vehicle/lane_queue_test.cpp
        tests/unit/input/scenario_test.cpp
        tests/integration/scenario_input_test.cpp
    )
    target_link_libraries(traffic_vehicle_tests PRIVATE traffic_core GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(traffic_signal_tests DISCOVERY_MODE PRE_TEST)
    gtest_discover_tests(traffic_vehicle_tests DISCOVERY_MODE PRE_TEST)
    add_test(NAME cli_help COMMAND traffic_sim --help)
    add_test(NAME cli_scheduled_demo COMMAND traffic_sim --scenario ${CMAKE_CURRENT_SOURCE_DIR}/config/demo_scenario.txt)
    set_tests_properties(cli_scheduled_demo PROPERTIES PASS_REGULAR_EXPRESSION "Deferred: 2")
    add_test(NAME cli_seeded_demo COMMAND traffic_sim --scenario ${CMAKE_CURRENT_SOURCE_DIR}/config/seeded_scenario.txt)
    add_test(NAME cli_missing_file COMMAND traffic_sim --scenario ${CMAKE_CURRENT_BINARY_DIR}/missing-scenario.txt)
    set_tests_properties(cli_missing_file PROPERTIES WILL_FAIL TRUE)
    add_test(NAME cli_bad_arguments COMMAND traffic_sim --unknown)
    set_tests_properties(cli_bad_arguments PROPERTIES WILL_FAIL TRUE)
endif()
