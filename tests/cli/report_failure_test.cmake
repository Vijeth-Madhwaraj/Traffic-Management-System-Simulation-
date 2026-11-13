execute_process(COMMAND "${TRAFFIC_SIM}" --scenario "${SCENARIO}" --report "${REPORT_PATH}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(status EQUAL 0 OR output MATCHES "CSV report saved")
    message(FATAL_ERROR "Failed report write was reported as success: ${output}")
endif()
if(NOT error MATCHES "(Failed to write|Unable to open) report file")
    message(FATAL_ERROR "Missing report error: ${error}")
endif()
