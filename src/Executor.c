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
    /*TODO: release context*/
    current->is_ready = 0;

    struct Frame *next = move_to_next_ready_frame(executor);
    if (next != main_frame(executor))
        swapcontext(&current->exe, &next->exe);
}

void async_exec(struct Executor *executor, Func fn, void *data)
{
    // TODO: check capacity
    executor->frames[executor->size] =
        (struct Frame *)malloc(sizeof(struct Frame));
    struct Frame *frame = executor->frames[executor->size++];

    getcontext(&frame->exe);
    frame->exe.uc_stack.ss_sp = frame->stack;
    frame->exe.uc_stack.ss_size = sizeof(frame->stack);
    frame->exe.uc_link = &main_frame(executor)->exe;
    makecontext(&frame->exe, (void (*)(void))func_wrapper, 3, fn, executor,
                data);

    frame->is_ready = 1;
}

void init_executor(struct Executor *executor, size_t count, size_t capacity)
{
    memset(executor, 0, sizeof(*executor));
    executor->capacity = align32pow2(count + 1);
    executor->frames =
        (struct Frame **)malloc(sizeof(struct Frame *) * executor->capacity);
    executor->current = 0;

    executor->frames[0] = (struct Frame *)malloc(sizeof(struct Frame));
    executor->size = 1;
    executor->frames[0]->is_ready = 1;
    init_io_context(&executor->ioc, capacity);
}

void run(struct Executor *executor)
{
    while (1) {
        struct Frame *current = get_current_frame(executor);
        struct Frame *next = move_to_next_ready_frame(executor);
        if (next != current) {
            swapcontext(&current->exe, &next->exe);
        }

        //TODO: if (is_empty(executor))
        //    return;
        process(&executor->ioc, BATCH_SIZE);
    }
}
