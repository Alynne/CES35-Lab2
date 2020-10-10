cmake_minimum_required(VERSION 3.17)
project(lab2)

set(CMAKE_C_STANDARD 11)

file(GLOB HTTP_SRC
        "includes/http.h"
        "src/http/*.cpp"
        "src/http/**/*.cpp")

file(GLOB SERVER_SRC
        "src/server/*.cpp"
        "src/server/**/*.cpp")

file(GLOB CLIENT_SRC
        "src/client/*.cpp"
        "src/client/**/*.cpp")

add_library(http STATIC ${HTTP_SRC})

add_executable(server ${SERVER_SRC})
target_link_libraries(server http)

add_executable(client ${SERVER_SRC})
target_link_libraries(client http)
