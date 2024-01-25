find_package(Threads REQUIRED)

set(ECHO_CLIENT_SOURCES
    echo/echo-client.c
)
add_executable(echo-client ${ECHO_CLIENT_SOURCES})
target_link_libraries(echo-client PRIVATE
    libcring
    Threads::Threads
)
target_include_directories(echo-client PRIVATE utility)

set(ECHO_SERVER_SOURCES
    echo/echo-server.c
)
add_executable(echo-server ${ECHO_SERVER_SOURCES})
target_link_libraries(echo-server PRIVATE
    libcring
    Threads::Threads
)
target_include_directories(echo-server PRIVATE utility)


set(CHAT_SERVER_SOURCES
    chat/chat-server.c
)
add_executable(chat-server ${CHAT_SERVER_SOURCES})
target_link_libraries(chat-server PRIVATE
    libcring
    Threads::Threads
)
target_include_directories(chat-server PRIVATE utility)

