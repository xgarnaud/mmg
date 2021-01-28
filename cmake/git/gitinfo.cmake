if(GIT_FOUND)
    EXECUTE_PROCESS(COMMAND "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                        RESULT_VARIABLE   GIT_RESULT
                        OUTPUT_VARIABLE   MMG_GIT_BRANCH
                        ERROR_VARIABLE    GIT_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE )
    IF( NOT ${GIT_RESULT} EQUAL 0 )
        MESSAGE( SEND_ERROR "Command '${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD' failed with following output:\n${GIT_ERROR}" )
    ENDIF()

    EXECUTE_PROCESS(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                        RESULT_VARIABLE   GIT_RESULT
                        OUTPUT_VARIABLE   MMG_GIT_COMMIT
                        ERROR_VARIABLE    GIT_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE )
    IF( NOT ${GIT_RESULT} EQUAL 0 )
        MESSAGE( SEND_ERROR "Command '${GIT_EXECUTABLE} rev-parse HEAD' failed with following output:\n${GIT_ERROR}" )
    ENDIF()

    EXECUTE_PROCESS(COMMAND "${GIT_EXECUTABLE}" show -s --format="%ci" ${MMG_GIT_COMMIT}
                        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                        RESULT_VARIABLE   GIT_RESULT
                        OUTPUT_VARIABLE   MMG_GIT_DATE
                        ERROR_VARIABLE    GIT_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE )
    IF( NOT ${GIT_RESULT} EQUAL 0 )
        MESSAGE( SEND_ERROR "Command '${GIT_EXECUTABLE} show -s --format="%ci" ${MMG_GIT_COMMIT}' failed with following output:\n${GIT_ERROR}" )
    ENDIF()

    FILE(WRITE ${COMMON_BINARY_DIR}/git_log_mmg.h
        "#ifndef _GIT_LOG_MMG_H\n"
        "#define _GIT_LOG_MMG_H\n"
        "#define MMG_GIT_BRANCH ${MMG_GIT_BRANCH}\n"
        "#define MMG_GIT_COMMIT ${MMG_GIT_COMMIT}\n"
        "#define MMG_GIT_DATE   ${MMG_GIT_DATE}\n"
        "#endif\n"
    )
else()
    FILE(WRITE ${COMMON_BINARY_DIR}/src/common/git_log_mmg.h
        "#ifndef _GIT_LOG_MMG_H\n"
        "#define _GIT_LOG_MMG_H\n"
        "#define MMG_GIT_BRANCH \"No git branch found\"\n"
        "#define MMG_GIT_COMMIT \"No git commit found\"\n"
        "#define MMG_GIT_DATE   \"No git commit found\"\n"
        "#endif\n"
)
endif()