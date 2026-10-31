execute_process(
    COMMAND "${TRAFFIC_SIM}" --validate "${SCENARIO}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
if(NOT result STREQUAL "0" OR NOT error STREQUAL "")
    message(FATAL_ERROR "Scenario validation failed: ${result}: ${error}")
endif()
foreach(expected "Scenario valid" "Seed: 42" "Duration (ticks): 6" "Intersections: 4"
                 "Roads: 3" "Entry lanes: 1" "Scheduled arrivals: 3" "Arrival streams: 0")
    string(FIND "${output}" "${expected}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Missing validation summary '${expected}': ${output}")
    endif()
endforeach()
if(output MATCHES "Generated:|Admitted to entry queues:|Deferred:")
    message(FATAL_ERROR "Validation must not run the simulation: ${output}")
endif()
execute_process(
    COMMAND "${TRAFFIC_SIM}" --validate "${SCENARIO}" --scenario "${SCENARIO}"
    RESULT_VARIABLE bad_arguments)
if(NOT bad_arguments STREQUAL "2")
    message(FATAL_ERROR "Mixed CLI modes must return argument-error code 2")
endif()
