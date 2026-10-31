execute_process(
    COMMAND "${TRAFFIC_SIM}" --scenario "${SCENARIO}" --lane-status
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
if(NOT result STREQUAL "0" OR NOT error STREQUAL "")
    message(FATAL_ERROR "Lane status failed: ${result}: ${error}")
endif()
foreach(expected "Generated: 3" "Admitted to entry queues: 3" "Lane entry (road AB): queued=0/1, deferred=0"
                 "  Queued IDs:\n  Deferred IDs:\n")
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

# A full, slow entry road leaves a real queue/backlog even with movement enabled.
execute_process(
    COMMAND "${TRAFFIC_SIM}" --scenario "${BLOCKED_SCENARIO}" --lane-status
    RESULT_VARIABLE blocked_result
    OUTPUT_VARIABLE blocked_output
    ERROR_VARIABLE blocked_error)
if(NOT blocked_result STREQUAL "0" OR NOT blocked_error STREQUAL "")
    message(FATAL_ERROR "Blocked-lane scenario failed: ${blocked_result}: ${blocked_error}")
endif()
foreach(expected "Generated: 3" "Admitted to entry queues: 2"
                 "Lane entry (road AB): queued=1/1, deferred=1"
                 "  Queued IDs: 2\n  Deferred IDs: 3\n")
    string(FIND "${blocked_output}" "${expected}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Missing blocked-lane detail '${expected}': ${blocked_output}")
    endif()
endforeach()
