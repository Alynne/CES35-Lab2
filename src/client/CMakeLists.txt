file(GLOB CLIENT_SRC
        "*.cpp"
        "**/*.cpp")

add_executable(client ${CLIENT_SRC})
target_link_libraries(client http)
target_include_directories(client PRIVATE ${INCLUDES_ROOT_FOLDER})
set_target_properties(client PROPERTIES
        OUTPUT_NAME "web-client"
)
