execute_process(
    COMMAND "${TRAFFIC_SIM}" --scenario "${SCENARIO}" --lane-status
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
if(NOT result STREQUAL "0" OR NOT error STREQUAL "")
    message(FATAL_ERROR "Lane status failed: ${result}: ${error}")
endif()
foreach(expected "Generated: 3" "Lane entry (road AB): queued=1/1, deferred=2"
                 "Queued IDs: 1" "Deferred IDs: 2 3")
    string(FIND "${output}" "${expected}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Missing lane detail '${expected}': ${output}")
    endif()
endforeach()
execute_process(
    COMMAND "${TRAFFIC_SIM}" --scenario "${SCENARIO}"
    RESULT_VARIABLE default_result
    OUTPUT_VARIABLE default_output)
if(NOT default_result STREQUAL "0" OR default_output MATCHES "Queued IDs:")
    message(FATAL_ERROR "Default output must remain concise")
endif()
execute_process(
    COMMAND "${TRAFFIC_SIM}" --validate "${SCENARIO}" --lane-status
    RESULT_VARIABLE mixed_modes)
if(NOT mixed_modes STREQUAL "2")
    message(FATAL_ERROR "Lane status applies to runs, not validation mode")
endif()
