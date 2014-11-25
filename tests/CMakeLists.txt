############################################################
#           Build Tests
############################################################

SET(CMAKE_BUILD_TYPE Debug)

FILE(MAKE_DIRECTORY
    ${PROJECT_SOURCE_DIR}/build/reports
    ${PROJECT_SOURCE_DIR}/build/reports/tests
    ${PROJECT_SOURCE_DIR}/build/reports/coverage
)


FILE(GLOB_RECURSE
    TESTS_SRC
    *.cpp
    *.cc
    *.cxx
    *.c
    )

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
ADD_EXECUTABLE(tests ${TESTS_SRC})
ADD_DEPENDENCIES(tests ${LIBRARY_NAME}static)
TARGET_LINK_LIBRARIES(tests ${LIBRARY_NAME}static)

