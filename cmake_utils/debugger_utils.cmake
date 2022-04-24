function(debugger_breakpoint_met IS_BREAKPOINT)
    get_cmake_property(VARIABLE_NAMES VARIABLES)
    list(SORT VARIABLE_NAMES)
    set(DEBUGGER_STATE "")
    foreach (VARIABLE_NAME ${VARIABLE_NAMES})
        string(APPEND DEBUGGER_STATE "${VARIABLE_NAME}=${${VARIABLE_NAME}}|")
    endforeach()
    execute_process(
        COMMAND
        /home/mikhail/CLionProjects/cmake-debugger/cmake-build-debug/breakpoint_subprocess/breakpoint_subprocess
        "${DEBUGGER_STATE}"
        ${IS_BREAKPOINT}
    )
endfunction()