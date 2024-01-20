#include <assert.h>

#include <Executor.h>
#include "utils.h"

int executor_invalid_init(void)
{
    MAYBE_UNUSED size_t valid_count = 10;
    MAYBE_UNUSED size_t invalid_count = 0;

    MAYBE_UNUSED size_t valid_capacity = 10;
    MAYBE_UNUSED size_t invalid_capacity = 0;

    MAYBE_UNUSED struct Executor valid_exe;

    assert(init_executor(NULL, invalid_count, invalid_capacity) == -1);
    assert(init_executor(NULL, invalid_count, valid_capacity) == -1);
    assert(init_executor(NULL, valid_count, invalid_capacity) == -1);
    assert(init_executor(NULL, valid_count, valid_capacity) == -1);
    assert(init_executor(&valid_exe, invalid_count, invalid_capacity) == -1);
    assert(init_executor(&valid_exe, invalid_count, valid_capacity) == -1);

    assert(init_executor(&valid_exe, valid_count, invalid_capacity) == -1);
    assert(is_memory_set_zero(&valid_exe, sizeof(struct Executor)) == 1);

    return 0;
}

int executor_valid_init(void)
{
    MAYBE_UNUSED size_t count = 5;
    MAYBE_UNUSED size_t expected_count = 8;
    MAYBE_UNUSED size_t capacity = 10;
    MAYBE_UNUSED size_t expected_capacity = 16;

    MAYBE_UNUSED struct Executor exe;

    assert(init_executor(&exe, count, capacity) == 0);
    assert(exe.current == 0);
    assert(exe.size == 1);
    assert(exe.capacity == expected_count);
    assert(sizeof(exe.frames) == expected_count);
    assert(exe.ioc.capacity == expected_capacity);
    assert(exe.ioc.tail == expected_capacity);

    assert(free_executor(&exe) == 0);
    assert(is_memory_set_zero(&exe, sizeof(struct Executor)) == 1);

    return 0;
}

int executor_invalid_free(void)
{
    assert(free_executor(NULL) == -1);
    return 0;
}

void run_executor_tests(void)
{
    printf("executor_invalid_init %d\n", executor_invalid_init());
    printf("executor_valid_init %d\n", executor_valid_init());
    printf("executor_invalid_free %d\n", executor_invalid_free());
}
