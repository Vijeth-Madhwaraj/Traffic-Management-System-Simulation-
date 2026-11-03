execute_process(
    COMMAND "${TRAFFIC_SIM}" --scenario "${SCENARIO}" --lane-status
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error)
if(NOT result STREQUAL "0" OR NOT error STREQUAL "")
    message(FATAL_ERROR "Batch-arrival scenario failed: ${result}: ${error}")
endif()
foreach(expected "Generated: 12" "Admitted to entry queues: 4" "Deferred: 8"
                 "Lane entry (road AB): queued=0/1, deferred=8"
                 "  Deferred IDs: 5 6 7 8 9 10 11 12")
    string(FIND "${output}" "${expected}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Missing batch-arrival result '${expected}': ${output}")
    endif()
endforeach()
