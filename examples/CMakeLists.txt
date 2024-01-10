set(PINGPONG_SOURCE_FILES
    ping-pong.c
)
add_executable(pingpong ${PINGPONG_SOURCE_FILES})
target_link_libraries(pingpong PRIVATE libcring)
