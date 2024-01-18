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

void execute(Func fn, struct Executor *executor, void *data)
{
    fn(executor, data);
    manage_async_finish(executor);
}

int async_exec(struct Executor *executor, Func fn, void *data)
{
    if (unlikely(executor->size >= executor->capacity)) {
        LOG_ERROR("Reach frame capacity = %lu limit.\n", executor->capacity);
        return -1;
    }

    struct Frame *frame = executor->frames[executor->size++];
    getcontext(&frame->exe);
    makecontext(&frame->exe, (void (*)(void))execute, 3, fn, executor, data);
    frame->is_ready = 1;

    return 0;
}

int free_executor(struct Executor *executor)
{
    if (!executor) {
        LOG_ERROR("NULL executor\n");
        return -1;
    }

    free_io_context(&executor->ioc);

    if (!executor->frames) {
        LOG_ERROR("uninitialized executor\n");
        return -1;
    }

    if (executor->frames[0]->exe.uc_stack.ss_sp)
        free(executor->frames[0]->exe.uc_stack.ss_sp);

    for (size_t i = 0; i < executor->capacity; ++i)
        if (executor->frames[i])
            free(executor->frames[i]);

    free(executor->frames);
    memset(executor, 0, sizeof(*executor));
    return 0;
}

int init_executor(struct Executor *executor, size_t count, size_t capacity)
{
    memset(executor, 0, sizeof(*executor));
    executor->capacity = align32pow2(count + 1);
    executor->frames =
        (struct Frame **)malloc(sizeof(struct Frame *) * executor->capacity);
    uint8_t *stack_mem = (uint8_t *)malloc(executor->capacity * STACK_SIZE);
    if (!stack_mem || !executor->frames)
        LOG_ERROR("unable to allocate memory\n");

    struct Frame *frame = NULL;
    for (size_t i = 0; i < executor->capacity; ++i) {
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

    if (init_io_context(&executor->ioc, capacity) < 0) {
        LOG_ERROR("error in io context init");
        return -1;
    }

    return 0;
}

void run(struct Executor *executor)
{
    if (!executor) {
        LOG_ERROR("NULL executor\n");
        return;
    }

    struct Frame *current = NULL;
    struct Frame *next = NULL;
    int ret = 0;

    while (1) {
        current = get_current_frame(executor);
        next = move_to_next_ready_frame(executor);

        if (next != current) {
            swapcontext(&current->exe, &next->exe);
        }

        if (executor->size <= 1)
            break;

        ret = process(&executor->ioc, BATCH_SIZE);
        if (unlikely(ret < 0)) {
            LOG_ERROR("io context process returned %d.\n", ret);
            break;
        }
    }
}
