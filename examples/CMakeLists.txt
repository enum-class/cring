set(PINGPONG_SOURCE_FILES
    echo-server.c
)

find_package(Threads REQUIRED)

add_executable(pingpong ${PINGPONG_SOURCE_FILES})
target_link_libraries(pingpong PRIVATE
    libcring
    Threads::Threads
)
