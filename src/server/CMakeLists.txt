add_executable(server main.c)

target_link_libraries(server PRIVATE
    util
    pthread
)