#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <liburing.h>

#include "Common.h"

#define MAX_BATCH_SIZE 1024

typedef void (*wait_cb)(void * /*data*/);
typedef void (*accept_cb)(int /*fd*/, void * /*data*/);
typedef void (*read_cb)(ssize_t /*read length*/, void * /*data*/);
typedef void (*write_cb)(ssize_t /*write length*/, void * /*data*/);
typedef void (*Cb)(void);

/**
 * @enum RequestType
 * @brief Enumeration defining different types of asynchronous requests in Cring.
 *
 * The RequestType enum provides distinct values for various asynchronous operations,
 * allowing the Cring library to differentiate between different types of tasks.
 *
 * - `ACCEPT (1)`: Represents an asynchronous accept operation, typically used for
 *   handling new incoming connections.
 * - `READ (2)`: Represents an asynchronous read operation, used for reading data
 *   from a file descriptor or socket.
 * - `WRITE (4)`: Represents an asynchronous write operation, used for writing data
 *   to a file descriptor or socket.
 * - `WAIT (8)`: Represents a wait operation, indicating a task that waits for a specific
 *   condition or event to occur.
 */
enum RequestType { ACCEPT = 1, READ = 2, WRITE = 4, WAIT = 8 };

/**
 * @struct Token
 * @brief Represents a token associated with an asynchronous operation in Cring.
 *
 * The Token structure encapsulates information about an asynchronous task,
 * providing a convenient way to pass relevant data within the Cring library.
 *
 * - `int fd`: File descriptor associated with the asynchronous task.
 * - `enum RequestType type`: Type of asynchronous request, defining the nature
 *    of the operation (e.g., ACCEPT, READ, WRITE, WAIT).
 * - `Cb cb`: Callback function to be executed upon completion of the asynchronous task.
 * - `void *data`: Additional data associated with the task, providing flexibility
 *    for user-specific information.
 *
 * The structure is packed to ensure minimal memory overhead. Users can leverage
 * Token instances when interacting with the Cring library to handle asynchronous tasks.
 */
struct Token {
    int fd;
    enum RequestType type;
    Cb cb;
    void *data;
} __attribute__((packed));

/**
 * @struct IOContext
 * @brief Represents the I/O context for asynchronous operations in Cring.
 *
 * The IOContext structure manages the underlying io_uring instance and
 * additional information required for handling asynchronous I/O operations.
 *
 * - `uint32_t tail`: Tail index used in the circular buffer of the io_uring instance.
 * - `struct io_uring ring`: The io_uring instance responsible for managing I/O operations.
 * - `uint32_t capacity`: Maximum capacity of the circular buffer in the io_uring instance.
 * - `struct Token **available_tokens`: Array of pointers to available Token instances,
 *    used for associating tasks with I/O operations.
 *
 * The IOContext structure provides a central component for handling I/O operations
 * within the Cring library. Users interact with this structure when scheduling and
 * managing asynchronous tasks in the event loop.
 */
struct IOContext {
    uint32_t tail;
    struct io_uring ring;
    uint32_t capacity;
    struct Token **available_tokens;
};

/**
 * Free resources associated with the IOContext structure.
 *
 * This function releases resources allocated for the IOContext structure,
 * including the io_uring ring and token arrays. It ensures proper cleanup
 * to prevent memory leaks.
 *
 * @param ioc
 *   A pointer to the IOContext structure whose resources need to be freed.
 * @return
 *   0 on success, -1 on failure.
 */
int free_io_context(struct IOContext *ioc);

/**
 * Initialize the IOContext structure with the specified capacity.
 *
 * @param ioc
 *   A pointer to the IOContext structure to be initialized.
 * @param capacity
 *   The desired capacity for the IOContext, representing the maximum number
 *   of tokens in the ring.
 * @return
 *   0 on success, -1 on failure.
 */
int init_io_context(struct IOContext *ioc, size_t capacity);

/**
 * Get a token from the IOContext's available tokens.
 *
 * This function retrieves a token from the available tokens in the IOContext.
 * Tokens are used as user_data, pass to sqe, and retrive in completion time.
 * The function decreases the tail index to mark the token as in use.
 *
 * @param ioc
 *   A pointer to the IOContext structure from which to get a token.
 * @return
 *   A pointer to the retrieved token on success, or NULL if no available tokens.
 */
static inline struct Token *get_token(struct IOContext *ioc)
{
    if (likely(ioc->tail != 0)) {
        struct Token *token = ioc->available_tokens[--ioc->tail];
        return token;
    }

    LOG_DEBUG("Run out of tokens. tail: %u, capacity: %u\n", ioc->tail,
              ioc->capacity);
    return NULL;
}

/**
 * Release a token back to the IOContext's available tokens.
 *
 * This function releases a token back to the available tokens in the IOContext.
 * The token becomes available for reuse, and the tail index is increased to
 * reflect the updated available token count.
 *
 * @param ioc
 *   A pointer to the IOContext structure to which the token will be released.
 * @param token
 *   A pointer to the token that needs to be released.
 */
static inline void release_token(struct IOContext *ioc, struct Token *token)
{
    if (unlikely(token == NULL || ioc->tail == ioc->capacity)) {
        LOG_DEBUG("tokens bag gets full. tail: %u, capacity: %u\n", ioc->tail,
                  ioc->capacity);
        return;
    }

    ioc->available_tokens[ioc->tail] = token;
    ++ioc->tail;
}

/**
 * Initiate a wait request using io_uring for the specified duration.
 *
 * This function prepares a wait request using io_uring for the specified duration,
 * associating it with a token and callback function. The token is acquired using
 * the get_token function. If the token cannot be obtained, the function returns -1.
 * The function sets up the necessary io_uring_sqe for the wait operation and assigns
 * the provided callback function and data to the token.
 *
 * @param ioc
 *   A pointer to the IOContext structure representing the io_uring context.
 * @param ts
 *   A pointer to the __kernel_timespec structure specifying the duration to wait.
 * @param cb
 *   A callback function to be executed when the wait operation completes.
 * @param data
 *   A pointer to user data to be passed to the callback function.
 * @return
 *   0 on success, -1 on failure. Returns -1 if a token cannot be obtained or if
 *   there is an issue with io_uring_sqe setup.
 */
int request_wait(struct IOContext *ioc, struct __kernel_timespec *ts,
                 wait_cb cb, void *data);

/**
 * Initiate an accept request using io_uring for the specified file descriptor.
 *
 * This function prepares an accept request using io_uring for the specified file descriptor,
 * associating it with a token and callback function. The token is acquired using the get_token
 * function. If the token cannot be obtained, the function returns -1. The function sets up
 * the necessary io_uring_sqe for the accept operation and assigns the provided callback function,
 * file descriptor, and data to the token.
 *
 * @param ioc
 *   A pointer to the IOContext structure representing the io_uring context.
 * @param fd
 *   The file descriptor for which the accept operation is initiated.
 * @param cb
 *   A callback function to be executed when the accept operation completes.
 * @param data
 *   A pointer to user data to be passed to the callback function.
 * @return
 *   0 on success, -1 on failure. Returns -1 if a token cannot be obtained or if
 *   there is an issue with io_uring_sqe setup.
 */
int request_accept(struct IOContext *ioc, int fd, accept_cb cb, void *data);

/**
 * Initiate a read request using io_uring for the specified file descriptor.
 *
 * This function prepares a read request using io_uring for the specified file descriptor,
 * associating it with a token and callback function. The token is acquired using the get_token
 * function. If the token cannot be obtained, the function returns -1. The function sets up
 * the necessary io_uring_sqe for the read operation and assigns the provided callback function,
 * file descriptor, buffer, size, and data to the token.
 *
 * @param ioc
 *   A pointer to the IOContext structure representing the io_uring context.
 * @param fd
 *   The file descriptor from which to read.
 * @param buffer
 *   A pointer to the buffer where the read data will be stored.
 * @param size
 *   The size of the buffer.
 * @param cb
 *   A callback function to be executed when the read operation completes.
 * @param data
 *   A pointer to user data to be passed to the callback function.
 * @return
 *   0 on success, -1 on failure. Returns -1 if a token cannot be obtained or if
 *   there is an issue with io_uring_sqe setup.
 */
int request_read(struct IOContext *ioc, int fd, void *buffer, size_t size,
                 read_cb cb, void *data);

/**
 * Initiate a write request using io_uring for the specified file descriptor.
 *
 * This function prepares a write request using io_uring for the specified file descriptor,
 * associating it with a token and callback function. The token is acquired using the get_token
 * function. If the token cannot be obtained, the function returns -1. The function sets up
 * the necessary io_uring_sqe for the write operation and assigns the provided callback function,
 * file descriptor, buffer, size, and data to the token.
 *
 * @param ioc
 *   A pointer to the IOContext structure representing the io_uring context.
 * @param fd
 *   The file descriptor to which data will be written.
 * @param buffer
 *   A pointer to the buffer containing the data to be written.
 * @param size
 *   The size of the data to be written.
 * @param cb
 *   A callback function to be executed when the write operation completes.
 * @param data
 *   A pointer to user data to be passed to the callback function.
 * @return
 *   0 on success, -1 on failure. Returns -1 if a token cannot be obtained or if
 *   there is an issue with io_uring_sqe setup.
 */
int request_write(struct IOContext *ioc, int fd, void *buffer, size_t size,
                  write_cb cb, void *data);

/**
 * Process completion queue entries for the given IOContext.
 *
 * This function submits a batch of io-uring operations, processes completion
 * queue entries, and invokes corresponding callback functions based on the type
 * of associated tokens. It releases resources associated with the processed
 * tokens to prevent memory leaks.
 *
 * @param ioc
 *   A pointer to the IOContext structure representing the io-uring instance.
 * @param batch
 *   The size of the batch of operations to be processed, limited to MAX_BATCH_SIZE.
 * @return
 *   The number of processed entries on success, or an error code on failure.
 */
int process(struct IOContext *ioc, size_t batch);

#ifdef __cplusplus
}
#endif

#endif
