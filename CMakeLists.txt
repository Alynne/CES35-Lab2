cmake_minimum_required(VERSION 3.17)
project(ces35-lab2)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

file(GLOB HTTP_SRC
        "src/http/*.cpp"
        "src/http/**/*.cpp")

file(GLOB SERVER_SRC
        "src/server/*.cpp"
        "src/server/**/*.cpp")

file(GLOB CLIENT_SRC
        "src/client/*.cpp"
        "src/client/**/*.cpp")

set(INCLUDES_ROOT_FOLDER ${CMAKE_SOURCE_DIR}/includes/)

## Http library
add_library(http STATIC ${HTTP_SRC})
target_include_directories(http PRIVATE ${INCLUDES_ROOT_FOLDER})

## Server binary
add_executable(server ${SERVER_SRC})
target_link_libraries(server http)
target_include_directories(server PRIVATE ${INCLUDES_ROOT_FOLDER})
set_target_properties(server PROPERTIES
        OUTPUT_NAME "web-server"
)

## Client binary
add_executable(client ${SERVER_SRC})
target_link_libraries(client http)
target_include_directories(client PRIVATE ${INCLUDES_ROOT_FOLDER})
set_target_properties(client PROPERTIES
        OUTPUT_NAME "web-client"
)

## Common configuration
set_target_properties(http server client 
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

