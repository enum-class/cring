find_package(Threads REQUIRED)

set(TESTS_SOURCES
    main-test.c
    io-context-test.c
    executor-test.c
)

add_executable(run_test ${TESTS_SOURCES})
target_link_libraries(run_test PRIVATE
    libcring
    Threads::Threads
)
