#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

typedef void (*accept_cb)(int/*fd*/, int/*error*/);
typedef void (*read_cb)(size_t/*read length*/, int/*error*/);
typedef void (*write_cb)(size_t/*write length*/, int/*error*/);

enum RequestType : uint32_t {
	ACCEPT = 1,
	READ = 2,
    WRITE = 4
};

struct Token {
    int fd;
    RequestType type;

    //CACHE_GUARD;

    union {
        accept_cb accept;
        read_cb read;
        write_cb write;
        //void (*accept_cb)(int/*fd*/, int/*error*/);
        //void (*read_cb)(size_t/*read length*/, int/*error*/);
        //void (*write_cb)(size_t/*write length*/, int/*error*/);
    }; //cache_aligned;
};

struct IOContext {
    uint32_t tail;
    const Token** available_tokens;

    //CACHE_GUARD;

    uint32_t capacity;
    const struct Token* tokens;

    //CACHE_GUARD;
    const struct io_uring* ring;
};

static inline Token* get_token(IOContext* ioc)
{
    if (likely(ioc->tail != 0)) {
        Token* token = *(ioc->available_tokens + ioc->tail);
        --ioc->tail;
        return token;
    }
    
    return NULL;
}

static inline void release_token(IOContext* ioc, Token* token)
{
    if (unlikely(token == NULL || ioc->tail == ioc->capacity))
        return;
    
    ++ioc->tail;
    *(ioc->available_tokens + ioc->tail) = token;
}

static inline int request_accept(IOContext* ioc, int fd, ) {
    Token* token = get_token(ioc);
    if (unlikely(token == NULL))
        return -1;

    struct io_uring_sqe *sqe = io_uring_get_sqe(ioc->ring);
    if (unlikely(sqe == NULL)) {
        release_token(ioc, token);
        return -1;
    }

    io_uring_prep_accept(sqe, fd, nullptr, nullptr, 0);
    token->type = RequestType::ACCEPT;
    token->fd = fd;
    io_uring_sqe_set_data(sqe, (void*)token);
}

static inline int request_read(IOContext* ioc, int fd, void* buffer, size_t size) {
    Token* token = get_token(ioc);
    if (unlikely(token == NULL))
        return -1;

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    if (unlikely(sqe == NULL)) {
        release_token(ioc, token);
        return -1;
    }

    io_uring_prep_read(sqe, fd, buffers, size, 0);
    token->type = RequestType::READ;
    token->fd = fd;
    io_uring_sqe_set_data(sqe, (void*)token);
}

static inline int request_write(IOContext* ioc, int fd, void* buffer, size_t size, write_cb cb) {
    Token* token = get_token(ioc);
    if (unlikely(token == NULL))
        return -1;

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);
    if (unlikely(sqe == NULL)) {
        release_token(ioc, token);
        return -1;
    }

    io_uring_prep_write(sqe, fd, buffers, size, 0);
    token.type = RequestType::WRITE;
    token->fd = fd;
    token->write = cb;
    io_uring_sqe_set_data(sqe, (void*)token);
}

static inline int process(IOContext* ioc, size_t batch) {
    static struct io_uring_cqe* cqes[1024];
    if (!batch)
        return batch;

    batch = batch > 1024 ? 1024 : batch;
    struct io_uring_cqe *cqe = NULL;

    io_uring_submit(ioc->ring);
    unsigned count = io_uring_peek_batch_cqe(&ring, &cqes[0], batch);

    if (count == 0) {
        int peek_result = io_uring_wait_cqe(&ring, &cqes[0]);
        if (peek_result != 0)
            return peek_result;

        count = 1;
    }

    for (int i = 0; i < count; ++i) {
        cqe = cqes[i];
        Token* token = (Token*)cqe->user_data;
        if (token->type == RequestType::ACCEPT) {
            token->accept(cqe->res);
            release_token(token);
        }
        else if (token->type == RequestType::READ) {
            token->read(cqe->res, 0);
            release_token(token);
        }
        else if (token->type == ReqType::WRITE) {
            token->write(cqe->res, 0);
            release_token(token);
        }
    }

    io_uring_cq_advance(ioc->ring, count);
    return count;
}

#ifdef __cplusplus
}
#endif

#endif
