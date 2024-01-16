find_package(Threads REQUIRED)

set(PINGPONG_CLIENT_SOURCES
    pingpong-client.c
)
add_executable(pingpong-client ${PINGPONG_CLIENT_SOURCES})
target_link_libraries(pingpong-client PRIVATE
    libcring
    Threads::Threads
)

set(PINGPONG_SERVER_SOURCES
    pingpong-server.c
)
add_executable(pingpong-server ${PINGPONG_SERVER_SOURCES})
target_link_libraries(pingpong-server PRIVATE
    libcring
    Threads::Threads
)
