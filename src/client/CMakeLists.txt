add_executable(client main.c)

target_link_libraries(client PRIVATE
    util
    pthread
)