find_package(Threads REQUIRED)

set(ECHO_CLIENT_SOURCES
    echo-client.c
)
add_executable(echo-client ${ECHO_CLIENT_SOURCES})
target_link_libraries(echo-client PRIVATE
    libcring
    Threads::Threads
)

set(ECHO_SERVER_SOURCES
    echo-server.c
)
add_executable(echo-server ${ECHO_SERVER_SOURCES})
target_link_libraries(echo-server PRIVATE
    libcring
    Threads::Threads
)
