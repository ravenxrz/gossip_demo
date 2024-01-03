function(AddCoverage target)
  find_program(LCOV_PATH lcov REQUIRED)
  find_program(GENHTML_PATH genhtml REQUIRED)
  add_custom_target(coverage-${target}
    COMMAND ${LCOV_PATH} -d . --zerocounters
    COMMAND $<TARGET_FILE:${target}>
    COMMAND ${LCOV_PATH} -d . --capture -o coverage.info
    COMMAND ${LCOV_PATH} -r coverage.info '/usr/include/*' '/usr/local/*' '*/build/*'
                         -o filtered.info
    COMMAND ${GENHTML_PATH} -o coverage-${target}
                            filtered.info --legend
    COMMAND rm -rf coverage.info filtered.info
    COMMAND find . -name "*gcda*" | xargs rm -rf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )
endfunction()
