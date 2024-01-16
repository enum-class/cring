#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <liburing.h>

#include <stdint.h>

#include "Common.h"

#define MAX_BATCH_SIZE 1024

typedef void (*wait_cb)(void * /*data*/);
typedef void (*accept_cb)(int /*fd*/, void * /*data*/);
typedef void (*read_cb)(ssize_t /*read length*/, void * /*data*/);
typedef void (*write_cb)(ssize_t /*write length*/, void * /*data*/);
typedef void (*Cb)(void);

enum RequestType { ACCEPT = 1, READ = 2, WRITE = 4, WAIT = 8 };

// TODO: take care of cache
struct Token {
    int fd;
    enum RequestType type;

    Cb cb;
    void *data;
};

// TODO: take care of cache
struct IOContext {
    uint32_t tail;
    struct Token **available_tokens;

    uint32_t capacity;
    struct Token *tokens;

    struct io_uring ring;
};

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

static inline struct Token *get_token(struct IOContext *ioc)
{
    if (likely(ioc->tail != 0)) {
        struct Token *token = ioc->available_tokens[ioc->tail];
        --ioc->tail;
        return token;
    }

    return NULL;
}

static inline void release_token(struct IOContext *ioc, struct Token *token)
{
    if (unlikely(token == NULL || ioc->tail == ioc->capacity))
        return;

    ++ioc->tail;
    ioc->available_tokens[ioc->tail] = token;
}

int request_wait(struct IOContext *ioc, struct __kernel_timespec *ts,
                 wait_cb cb, void *data);
int request_accept(struct IOContext *ioc, int fd, accept_cb cb, void *data);
int request_read(struct IOContext *ioc, int fd, void *buffer, size_t size,
                 read_cb cb, void *data);
int request_write(struct IOContext *ioc, int fd, void *buffer, size_t size,
                  write_cb cb, void *data);
int process(struct IOContext *ioc, size_t batch);

#ifdef __cplusplus
}
#endif

#endif
