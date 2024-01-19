find_package(Threads REQUIRED)

set(IOCONTEXT_TESTS_SOURCES
    io-context-test.c
)
add_executable(io-context-test ${IOCONTEXT_TESTS_SOURCES})
target_link_libraries(io-context-test PRIVATE
    libcring
    Threads::Threads
)
