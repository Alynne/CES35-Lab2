set(BINARY ${CMAKE_PROJECT_NAME}_tst)
file(GLOB TEST_SOURCES
        "*.h"
        "*.cpp"
        "**/*.cpp")

add_executable(${BINARY} ${TEST_SOURCES})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC http gtest)
