cmake_minimum_required(VERSION 3.16)

project (CRing C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED On)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS On)
add_compile_options(-Wall -Wextra -pedantic -Werror)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-zero-variadic-macro-arguments")
endif()

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Executor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/IOContext.c
)

add_library(libcring STATIC ${SOURCE_FILES})
target_link_libraries(libcring PUBLIC
    uring
)

target_include_directories(libcring PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

option(CRING_BENCHMARK "Enable benchmarking" OFF)
option(CRING_TEST "Enable tests" OFF)
option(CRING_EXAMPLES "Enable examples" OFF)

if (CRING_BENCHMARK)
    add_subdirectory(benchmarks)
endif()

if (CRING_TESTS)
    add_subdirectory(tests)
endif()

if (CRING_EXAMPLES)
    add_subdirectory(examples)
endif()

add_custom_target(
    clang-tidy-check clang-tidy -p ${CMAKE_BINARY_DIR}/compile_commands.json -checks=cert* --warnings-as-errors=* -header-filter=.* ${SOURCE_FILES}
    DEPENDS ${SOURCE_FILES}
)
