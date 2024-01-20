#ifndef EXECUTOR_H
#define EXECUTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ucontext.h>

#include "IOContext.h"

#define STACK_SIZE 8192
#define BATCH_SIZE 1024

/**
 * @struct Frame
 * @brief Represents a frame or context for a task or coroutine in the Cring library.
 *
 * The Frame structure encapsulates the essential components needed for managing
 * the state and execution of an individual asynchronous task or coroutine.
 *
 * - `ucontext_t exe`: Execution context associated with the task, including the program
 *    counter, stack pointer, and register values.
 * - `ssize_t result`: Result or status of the execution of the associated task.
 * - `int is_ready`: Flag indicating whether the task is ready for execution or has completed.
 *
 * This structure is integral to the asynchronous programming model in Cring, providing
 * a container for the context and result of individual tasks within the event loop.
 */
struct Frame {
    ucontext_t exe;
    ssize_t result;
    int is_ready;
};

/**
 * @struct Executor
 * @brief Represents the executor for managing asynchronous tasks in the Cring library.
 *
 * The Executor structure holds information about the execution context, the current
 * task being processed, the total number of tasks, the maximum capacity of tasks,
 * and an array of Frame pointers representing individual tasks or coroutines.
 *
 * - `struct IOContext ioc`: The IOContext associated with the executor for managing I/O operations.
 * - `size_t current`: Index of the currently active task in the executor.
 * - `size_t size`: Current number of tasks scheduled in the executor.
 * - `size_t capacity`: Maximum number of tasks the executor can handle.
 * - `struct Frame **frames`: Dynamic array of Frame pointers representing individual tasks.
 *
 * This structure plays a crucial role in orchestrating and managing the asynchronous
 * execution of tasks within the Cring event loop.
 */
struct Executor {
    struct IOContext ioc;
    size_t current;
    size_t size;

    size_t capacity;
    struct Frame **frames;
};

typedef void (*Func)(struct Executor *, void *);

/**
 * Retrieve the last frame from the Executor's frame array.
 *
 * This static inline function returns a pointer to the last frame in the
 * frame stack of the given Executor.
 *
 * @param executor
 *   A pointer to the Executor structure from which to retrieve the last frame.
 * @return
 *   A pointer to the last frame in the frames array.
 */
static inline struct Frame *get_last_frame(struct Executor *executor)
{
    return executor->frames[executor->size - 1];
}

/**
 * Retrieve the current frame from the Executor's frame stack.
 *
 * This static inline function returns a pointer to the current frame in the
 * frame stack of the given Executor. The current frame typically represents
 * the currently executing function's frame.
 *
 * @param executor
 *   A pointer to the Executor structure from which to retrieve the current frame.
 * @return
 *   A pointer to the current frame in the frame stack.
 */
static inline struct Frame *get_current_frame(struct Executor *executor)
{
    return executor->frames[executor->current];
}

/**
 * Retrieve the main frame from the Executor's frame stack.
 *
 * This static inline function returns a pointer to the main frame in the frame
 * stack of the given Executor. The main frame typically represents the
 * starting point or entry point of the program execution.
 *
 * @param executor
 *   A pointer to the Executor structure from which to retrieve the main frame.
 * @return
 *   A pointer to the main frame in the frame stack.
 */
static inline struct Frame *main_frame(struct Executor *executor)
{
    return executor->frames[0];
}

/**
 * Move to the next ready frame in the Executor's frame stack.
 *
 * This static inline function advances to the next frame in the frame stack
 * of the given Executor that is marked as ready for execution. It updates the
 * 'current' index of the Executor and retrieves the corresponding frame.
 * The function continues looping until it finds a frame marked as ready.
 *
 * @param executor
 *   A pointer to the Executor structure to navigate and find the next ready frame.
 * @return
 *   A pointer to the next ready frame in the frame stack.
 */
static inline struct Frame *move_to_next_ready_frame(struct Executor *executor)
{
    struct Frame *frame = NULL;

    do {
        ++executor->current;
        executor->current = executor->current % executor->size;
        frame = get_current_frame(executor);
    } while (!frame->is_ready);

    return frame;
}

/**
 * Suspend the execution of the current frame in the Executor.
 *
 * This static inline function suspends the execution of the current frame in
 * the given Executor. It sets the 'is_ready' flag of the current frame to 0,
 * indicating that it is not ready for immediate execution. It then proceeds to
 * find and switch to the next frame in the frame stack that is marked as ready.
 * The function utilizes the swapcontext function for context switching.
 *
 * @param executor
 *   A pointer to the Executor structure containing the current frame.
 */
static inline void suspend_current_frame(struct Executor *executor)
{
    struct Frame *current = get_current_frame(executor);
    current->is_ready = 0;
    struct Frame *next = move_to_next_ready_frame(executor);
    swapcontext(&current->exe, &next->exe);
}

/**
 * Callback function for asynchronous wait completion.
 *
 * This function is a callback invoked upon the completion of an asynchronous
 * wait operation. It sets the 'is_ready' flag of the associated frame to 1,
 * indicating that the frame is ready for execution.
 *
 * @param data
 *   A pointer to the data associated with the asynchronous wait operation,
 *   typically pointing to a Frame structure.
 */
void wait_fn(void *data);

/**
 * Asynchronously wait for a specified period in the Executor.
 *
 * This static inline function initiates an asynchronous wait operation in the
 * given Executor, allowing the current frame to be suspended for the specified
 * duration. It utilizes the request_wait function with the provided time
 * specification, wait function, and current frame. Upon a successful request,
 * the function suspends the current frame's execution until the wait completes.
 *
 * @param executor
 *   A pointer to the Executor structure managing the asynchronous wait.
 * @param ts
 *   A pointer to the timespec structure specifying the wait duration.
 * @return
 *   0 on success, or an error code on failure.
 */
static inline int async_wait(struct Executor *executor,
                             struct __kernel_timespec *ts)
{
    struct Frame *frame = get_current_frame(executor);
    int ret = request_wait(&executor->ioc, ts, &wait_fn, frame);
    if (unlikely(ret < 0)) {
        LOG_ERROR("wait request failed %d", ret);
        return ret;
    }
    suspend_current_frame(executor);
    return 0;
}

/**
 * Callback function for asynchronous accept completion.
 *
 * This function is a callback invoked upon the completion of an asynchronous
 * accept operation. It sets the 'result' field of the associated frame to the
 * accepted file descriptor and marks the frame as ready for execution.
 *
 * @param fd
 *   The accepted file descriptor resulting from the accept operation.
 * @param data
 *   A pointer to the data associated with the asynchronous accept operation,
 *   typically pointing to a Frame structure.
 */
void accept_fn(int fd, void *data);

/**
 * Asynchronously initiate an accept operation in the Executor.
 *
 * This static inline function initiates an asynchronous accept operation on
 * the given file descriptor within the provided Executor. It utilizes the
 * request_accept function with the specified file descriptor, accept function,
 * and current frame. Upon a successful request, the function suspends the
 * current frame's execution until the accept operation completes.
 *
 * @param executor
 *   A pointer to the Executor structure managing the asynchronous accept.
 * @param fd
 *   The file descriptor on which to perform the accept operation.
 * @return
 *   0 on success, or an error code on failure. The result of the operation
 *   can be retrieved from the associated frame's 'result' field.
 */
static inline int async_accept(struct Executor *executor, int fd)
{
    struct Frame *frame = get_current_frame(executor);
    int ret = request_accept(&executor->ioc, fd, &accept_fn, frame);
    if (unlikely(ret < 0)) {
        LOG_ERROR("accept request failed %d", ret);
        return ret;
    }
    suspend_current_frame(executor);
    return frame->result;
}

/**
 * Callback function for asynchronous read completion.
 *
 * This function is a callback invoked upon the completion of an asynchronous
 * read operation. It sets the 'result' field of the associated frame to the
 * length of the read data and marks the frame as ready for execution.
 *
 * @param length
 *   The length of the data read during the asynchronous read operation.
 * @param data
 *   A pointer to the data associated with the asynchronous read operation,
 *   typically pointing to a Frame structure.
 */
void read_fn(ssize_t length, void *data);

/**
 * Asynchronously initiate a read operation in the Executor.
 *
 * This static inline function initiates an asynchronous read operation on
 * the given file descriptor within the provided Executor. It utilizes the
 * request_read function with the specified file descriptor, buffer, size,
 * read function, and current frame. Upon a successful request, the function
 * suspends the current frame's execution until the read operation completes.
 *
 * @param executor
 *   A pointer to the Executor structure managing the asynchronous read.
 * @param fd
 *   The file descriptor on which to perform the read operation.
 * @param buffer
 *   A pointer to the buffer where the read data will be stored.
 * @param size
 *   The number of bytes to read.
 * @return
 *   The number of bytes read on success, or an error code on failure.
 *   The result of the operation can be retrieved from the associated frame's 'result' field.
 */
static inline ssize_t async_read(struct Executor *executor, int fd,
                                 void *buffer, size_t size)
{
    struct Frame *frame = get_current_frame(executor);
    int ret = request_read(&executor->ioc, fd, buffer, size, &read_fn, frame);
    if (unlikely(ret < 0)) {
        LOG_ERROR("read request failed %d", ret);
        return ret;
    }
    suspend_current_frame(executor);
    return frame->result;
}

/**
 * Callback function for asynchronous write completion.
 *
 * This function is a callback invoked upon the completion of an asynchronous
 * write operation. It sets the 'result' field of the associated frame to the
 * length of the written data and marks the frame as ready for execution.
 *
 * @param length
 *   The length of the data written during the asynchronous write operation.
 * @param data
 *   A pointer to the data associated with the asynchronous write operation,
 *   typically pointing to a Frame structure.
 */
void write_fn(ssize_t length, void *data);

/**
 * Asynchronously initiate a write operation in the Executor.
 *
 * This static inline function initiates an asynchronous write operation on
 * the given file descriptor within the provided Executor. It utilizes the
 * request_write function with the specified file descriptor, buffer, size,
 * write function, and current frame. Upon a successful request, the function
 * suspends the current frame's execution until the write operation completes.
 *
 * @param executor
 *   A pointer to the Executor structure managing the asynchronous write.
 * @param fd
 *   The file descriptor on which to perform the write operation.
 * @param buffer
 *   A pointer to the buffer containing the data to be written.
 * @param size
 *   The number of bytes to write.
 * @return
 *   The number of bytes written on success, or an error code on failure.
 *   The result of the operation can be retrieved from the associated frame's 'result' field.
 */
static inline ssize_t async_write(struct Executor *executor, int fd,
                                  void *buffer, size_t size)
{
    struct Frame *frame = get_current_frame(executor);
    int ret = request_write(&executor->ioc, fd, buffer, size, &write_fn, frame);
    if (unlikely(ret < 0)) {
        LOG_ERROR("write request failed %d", ret);
        return ret;
    }

    suspend_current_frame(executor);
    return frame->result;
}

/**
 * Wrapper function for asynchronous task execution in the Executor.
 *
 * This function serves as a higher-level wrapper for executing a function within
 * the context of a cooperative multitasking environment managed by an Executor.
 * It executes the provided function, typically performing an asynchronous task,
 * and then manages the completion of the task by calling the 'manage_async_finish'
 * function.
 *
 * @param fn
 *   The function to execute asynchronously within the Executor.
 * @param executor
 *   A pointer to the Executor structure managing the cooperative multitasking.
 * @param data
 *   Additional data to be passed to the asynchronous function.
 */
void execute(Func fn, struct Executor *executor, void *data);

/**
 * Swap the current frame with the last frame in the Executor's frame stack.
 *
 * This static inline function swaps the positions of the current frame and the
 * last frame in the frame stack of the given Executor. If the last frame is not
 * the same as the current frame, no swap occurs. The function returns a pointer
 * to the newly positioned current frame after the swap.
 *
 * @param executor
 *   A pointer to the Executor structure managing the frame stack.
 * @return
 *   A pointer to the current frame after the swap, or NULL if no swap occurred.
 */
static inline struct Frame *
swap_current_frame_with_last_frame(struct Executor *executor)
{
    struct Frame *current = get_current_frame(executor);
    struct Frame *last = get_last_frame(executor);

    if (last == current)
        return NULL;

    struct Frame *tmp = last;
    executor->frames[executor->size - 1] = current;
    executor->frames[executor->current] = tmp;
    return executor->frames[executor->current];
}

/**
 * Manage the completion of an asynchronous task in the Executor.
 *
 * This static inline function manages the completion of an asynchronous task
 * within the given Executor. It swaps the current frame with the last frame,
 * reduces the frame stack size, and performs context switching based on the
 * readiness of the current frame and the availability of the next ready frame.
 *
 * @param executor
 *   A pointer to the Executor structure managing the asynchronous task.
 */
static inline void manage_async_finish(struct Executor *executor)
{
    struct Frame *current = swap_current_frame_with_last_frame(executor);
    --executor->size;

    if (current) {
        if (current->is_ready) {
            swapcontext(&executor->frames[executor->size]->exe, &current->exe);
        } else {
            struct Frame *next = move_to_next_ready_frame(executor);
            if (next != main_frame(executor))
                swapcontext(&executor->frames[executor->size]->exe, &next->exe);
        }
    }
}

/**
 * Asynchronously execute a function within the cooperative multitasking environment.
 *
 * This function initiates the asynchronous execution of the provided function
 * within the cooperative multitasking environment managed by the given Executor.
 * It creates a new frame, associates the function and data with it, sets it as ready
 * for execution, and increments the frame stack. If the frame stack is at capacity,
 * the function returns an error code.
 *
 * @param executor
 *   A pointer to the Executor structure managing the cooperative multitasking.
 * @param fn
 *   The asynchronous task function to execute within the Executor.
 * @param data
 *   Additional data to be passed to the asynchronous task.
 * @return
 *   0 on success, -1 on failure (e.g., if the frame stack is at capacity).
 */
int async_exec(struct Executor *executor, Func fn, void *data);

/**
 * Initialize the Executor for cooperative multitasking.
 *
 * This function initializes the Executor structure for cooperative multitasking.
 * It allocates memory for the frame stack, initializes each frame, sets up the
 * execution stack, and initializes the associated I/O context. The Executor is
 * prepared to manage cooperative multitasking with the specified count of frames
 * and a given capacity. Memory allocations are aligned for performance.
 *
 * @param executor
 *   A pointer to the Executor structure to be initialized.
 * @param count
 *   The count of frames to be created in the Executor.
 * @param capacity
 *   The capacity of the Executor for handling asynchronous tasks.
 * @return
 *   0 on success, -1 on failure (e.g., memory allocation failure).
 */
int init_executor(struct Executor *executor, size_t count, size_t capacity);

/**
 * Free resources associated with the Executor structure.
 *
 * This function releases resources allocated for the Executor structure,
 * including the I/O context, frame stack memory, and individual frames.
 * It ensures proper cleanup to prevent memory leaks.
 *
 * @param executor
 *   A pointer to the Executor structure whose resources need to be freed.
 * @return
 *   0 on success, -1 on failure.
 */
int free_executor(struct Executor *executor);

/**
 * Run the cooperative multitasking loop in the Executor.
 *
 * This function enters the cooperative multitasking loop within the given Executor.
 * It iteratively swaps between ready frames, executing asynchronous tasks and processing
 * I/O events using the specified batch size. The loop continues until there is only one
 * remaining frame in the Executor, at which point it breaks out of the loop.
 *
 * @param executor
 *   A pointer to the Executor structure managing cooperative multitasking.
 */
void run(struct Executor *executor);

#ifdef __cplusplus
}
#endif

#endif
