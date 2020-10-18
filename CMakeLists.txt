cmake_minimum_required(VERSION 3.17)
project(ces35-lab2)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 17)

if (NOT ${APPLE})
        # Ubuntu 18 has filesystem support in experimental stage.
        add_definitions(-DEXPERIMENTALFS)
        link_libraries(stdc++fs)
endif()

set(INCLUDES_ROOT_FOLDER ${CMAKE_SOURCE_DIR}/includes/)
file(GLOB HEADER_FILES "${INCLUDES_ROOT_FOLDER}/*.h" "${INCLUDES_ROOT_FOLDER}/**/*.h")

add_subdirectory(third-party/googletest)
add_subdirectory(src/http)
add_subdirectory(src/client)
add_subdirectory(src/server)
add_subdirectory(tests)

## Common configuration
set_target_properties(http server client PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
