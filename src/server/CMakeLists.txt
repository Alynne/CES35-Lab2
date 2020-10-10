file(GLOB SERVER_SRC
        ${HEADER_FILES}
        "*.h"
        "*.cpp"
        "**/*.cpp")

add_executable(server ${SERVER_SRC})
target_link_libraries(server http)
target_include_directories(server PRIVATE ${INCLUDES_ROOTS_FOLDER})
set_target_properties(server PROPERTIES
        OUTPUT_NAME "web-server"
)
