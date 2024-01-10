cmake_minimum_required(VERSION 3.19)

project (CRing C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED On)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS On)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Executor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IOContext.c
)

find_package(Threads REQUIRED)

add_library(libcring STATIC ${SOURCE_FILES})
target_link_libraries(libcring PUBLIC
    uring
    Threads::Threads
)
target_include_directories(libcring PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

option(CRING_BENCHMARK "Enable benchmarking" OFF)
option(CRING_TEST "Enable tests" OFF)
option(CRING_EXAMPLES "Enable examples" ON)

if (CRING_BENCHMARK)
    add_subdirectory(benchmarks)
endif()

if (CRING_TEST)
    add_subdirectory(tests)
endif()

if (CRING_EXAMPLES)
    add_subdirectory(examples)
endif()