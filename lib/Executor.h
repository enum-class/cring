#ifndef EXECUTOR_H
#define EXECUTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ucontext.h>

#include "IOContext.h"

#define STACK 8192
#define BATCH_SIZE 1024

struct Frame {
    ucontext_t exe;
    ssize_t result;
    int is_ready;
    uint8_t stack[STACK]; // TODO ?
};

struct Executor {
    struct IOContext ioc;
    size_t size;
    size_t capacity;
    size_t current;
    struct Frame **frames;
};

typedef void (*Func)(struct Executor *, void *);

static inline struct Frame *get_current_frame(struct Executor *executor)
{
    return executor->frames[executor->current];
}

static inline struct Frame *main_frame(struct Executor *executor)
{
    return executor->frames[0];
}

static inline struct Frame *move_to_next_ready_frame(struct Executor *executor)
{
    struct Frame *frame = NULL;

    do {
        executor->current = (++executor->current) % executor->size;
        frame = get_current_frame(executor);
    } while (!frame->is_ready);

    return frame;
}

static inline void wait(struct Executor *executor)
{
    struct Frame *current = get_current_frame(executor);
    current->is_ready = 0;
    struct Frame *next = move_to_next_ready_frame(executor);
    swapcontext(&current->exe, &next->exe);
}

void wait_fn(void *data);

static inline void async_wait(struct Executor *executor,
                              struct __kernel_timespec *ts)
{
    struct Frame *frame = get_current_frame(executor);
    request_wait(&executor->ioc, ts, &wait_fn, frame);
    wait(executor);
    return;
}

void accept_fn(int fd, void *data);

static inline int async_accept(struct Executor *executor, int fd)
{
    struct Frame *frame = get_current_frame(executor);
    request_accept(&executor->ioc, fd, &accept_fn, frame);
    wait(executor);
    return frame->result;
}

void read_fn(ssize_t length, void *data);

static inline ssize_t async_read(struct Executor *executor, int fd,
                                 void *buffer, size_t size)
{
    struct Frame *frame = get_current_frame(executor);
    request_read(&executor->ioc, fd, buffer, size, &read_fn, frame);
    wait(executor);
    return frame->result;
}

void write_fn(ssize_t length, void *data);

static inline ssize_t async_write(struct Executor *executor, int fd,
                                  void *buffer, size_t size)
{
    struct Frame *frame = get_current_frame(executor);
    request_write(&executor->ioc, fd, buffer, size, &write_fn, frame);
    wait(executor);
    return frame->result;
}

void func_wrapper(Func fn, struct Executor *executor, void *data);

void async_exec(struct Executor *executor, Func fn, void *data);

void init_executor(struct Executor *executor, size_t count, size_t capacity);

void run(struct Executor *executor);

#ifdef __cplusplus
}
#endif

#endif
