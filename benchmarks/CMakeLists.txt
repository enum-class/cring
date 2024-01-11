set(PINGPONG_SOURCE_FILES
    pingpong-client.c
)

find_package(Threads REQUIRED)

add_executable(pingpong-client ${PINGPONG_SOURCE_FILES})
target_link_libraries(pingpong-client PRIVATE
    libcring
    Threads::Threads
)
