#ifndef EXECUTOR_H
#define EXECUTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#define SIGSTKSZ 8192;

struct Frame {
    ucontext_t exe;
    size_t result;
    int is_ready;
    uint8_t stack[SIGSTKSZ]
};

struct Executor {
    struct IOContext ioc;
    size_t size;
    size_t current;
    const Frame* frames[10];
};

Frame* get_current_frame(const Executor* executor) {
    return executor->frames[executor->current];
}

Frame* move_to_next_ready_frame(Executor* executor) {
    Frame* fram = NULL;

    do {
        executor->current = (++executor->current) % size;
        frame = get_current_frame();
    } while (!frame->is_ready)

    return frame;
}

void read_cb(size_t length, int error, void* data) {
    Frame* frame = (Frame*)data;
    frame->result = error < 0 ? error : length;
    frame->is_ready = true;
}

void write_cb(size_t length, int error, void* data) {
    Frame* frame = (Frame*)data;
    frame->result = error < 0 ? error : length;
    frame->is_ready = true;
}

void wait(const Executor* executor, Frame* current) {
    Frame* current = get_current_frame(executor);
    Frame* next = move_to_next_ready_frame(executor);
    swap_context(&current->exe, &next->exe);
}

ssize_t async_read(const Executor* executor, int fd, void* buffer, size_t size) {
    Frame* frame = get_current_frame(executor);
    request_read(executor->ioc, fd, buffer, size, read_cb, frame);
    wait(executor);
    return frame->result;
}

ssize_t async_write(const Executor* executor, int fd, void* buffer, size_t size) {
    Frame* frame = get_current_frame(executor);
    request_wrire(executor->ioc, fd, buffer, size, write_cb, frame);
    wait(executor);
}

Frame* main_frame(const Executor* executor) {
    return executor->frames[0];
}

void func_wrapper(fn, executor) {
    fn(executor);
    /*release context*/
    swap_context(main_frame(executor)->exe, move_to_next_ready_frame(executor)->next/*if != main*/);
    /*else return*/
}

void async_exec(const Executor* executor, void(*)(void) fn) {
    Frame* frame = executor->frames[size++];
    getcontext(&frame->exe);
    frame->ctx.uc_stack.ss_sp = frame->stack;
    frame->ctx.uc_stack.ss_size = sizeof(frame->stack);
    frame->ctx.uc_link = &uctx_main;
    makecontext(&frame->exe, fnc_wrapper, 2, fn, executor);

    frame->is_ready= true;
}

void init(Executor* exe, count) {
    memset(exe, 0, sizeof(*exe));
    exe->size = align32pow2(count + 1);
    exe->current = 0;
    // inital datastructers
    // init main frame
}

void run(Executor) {
    while (true) {
        swap_context(main_frame()->exe, move_to_next_ready_frame(executor)->exe/*if != main*/);
        if (is_empty(executor))
            return;
        process(executor->ioc, 1024);
    }
}

#ifdef __cplusplus
}
#endif

#endif
