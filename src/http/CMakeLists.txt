file(GLOB HTTP_SRC
        ${HEADER_FILES}
        "*.cpp"
        "**/*.cpp")

add_library(http STATIC ${HTTP_SRC})
target_include_directories(http PRIVATE ${INCLUDES_ROOT_FOLDER})
