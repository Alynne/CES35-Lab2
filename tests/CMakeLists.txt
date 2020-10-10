set(BINARY ${CMAKE_PROJECT_NAME}_tst)
file(GLOB TEST_SOURCES
        "*.h"
        "*.cpp"
        "**/*.cpp")

add_executable(${BINARY} ${TEST_SOURCES})
target_include_directories(${BINARY} PRIVATE ${INCLUDES_ROOT_FOLDER})
add_test(NAME ${BINARY} COMMAND ${BINARY})
target_link_libraries(${BINARY} PUBLIC http gtest)
target_include_directories(${BINARY} PRIVATE ${INCLUDES_ROOT_FOLDER})
