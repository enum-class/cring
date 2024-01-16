#include "Executor.h"

#include <stdlib.h>
#include <string.h>

#include "IOContext.h"

void read_fn(ssize_t length, void *data)
{
    struct Frame *frame = (struct Frame *)data;
    frame->result = length;
    frame->is_ready = 1;
}

void write_fn(ssize_t length, void *data)
{
    struct Frame *frame = (struct Frame *)data;
    frame->result = length;
    frame->is_ready = 1;
}

void wait_fn(void *data)
{
    struct Frame *frame = (struct Frame *)data;
    frame->is_ready = 1;
}

void accept_fn(int fd, void *data)
{
    struct Frame *frame = (struct Frame *)data;
    frame->result = fd;
    frame->is_ready = 1;
}

void func_wrapper(Func fn, struct Executor *executor, void *data)
{
    fn(executor, data);
    struct Frame *current = get_current_frame(executor);
    current->is_ready = 0;

    if (executor->frames[executor->size - 1] != current) {
        struct Frame *tmp = executor->frames[executor->size - 1];
        executor->frames[executor->size - 1] =
            executor->frames[executor->current];
        executor->frames[executor->current] = tmp;

        --executor->size;

        current = get_current_frame(executor);
        if (current->is_ready) {
            swapcontext(&executor->frames[executor->size]->exe, &current->exe);
        } else {
            struct Frame *next = move_to_next_ready_frame(executor);
            if (next != main_frame(executor))
                swapcontext(&executor->frames[executor->size]->exe, &next->exe);
        }

    } else {
        --executor->size;
    }
}

int async_exec(struct Executor *executor, Func fn, void *data)
{
    if (executor->size >= executor->capacity)
        return -1;

    struct Frame *frame = executor->frames[executor->size++];
    if (!frame)
        return -1;
    getcontext(&frame->exe);
    makecontext(&frame->exe, (void (*)(void))func_wrapper, 3, fn, executor,
                data);
    frame->is_ready = 1;

    return 0;
}

int init_executor(struct Executor *executor, size_t count, size_t capacity)
{
    memset(executor, 0, sizeof(*executor));
    executor->capacity = align32pow2(count + 1);
    executor->frames =
        (struct Frame **)malloc(sizeof(struct Frame *) * executor->capacity);

    struct Frame *frame = NULL;
    uint8_t *stack_mem = (uint8_t *)malloc(executor->capacity * STACK_SIZE);

    for (int i = 0; i < executor->capacity; ++i) {
        executor->frames[i] = (struct Frame *)malloc(sizeof(struct Frame));
        executor->frames[i]->is_ready = 0;
        frame = executor->frames[i];
        frame->exe.uc_stack.ss_sp = &stack_mem[i * STACK_SIZE];
        frame->exe.uc_stack.ss_size = STACK_SIZE;
        frame->exe.uc_link = &main_frame(executor)->exe;
    }

    executor->size = 1;
    executor->current = 0;
    executor->frames[0]->is_ready = 1;

    if (init_io_context(&executor->ioc, capacity) < 0)
        return -1;

    return 0;
}

void run(struct Executor *executor)
{
    while (1) {
        struct Frame *current = get_current_frame(executor);
        struct Frame *next = move_to_next_ready_frame(executor);
        if (next != current) {
            swapcontext(&current->exe, &next->exe);
        }

        if (executor->size < 2)
            break;

        process(&executor->ioc, BATCH_SIZE);
    }
}
