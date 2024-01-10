#include "IOContext.h"

#include <string.h>

void init_io_context(struct IOContext *ioc)
{
    memset(ioc, 0, sizeof(*ioc));
    ioc->capacity = 1024;
    ioc->tail = 1023;
    for (int i = 0; i < 1024; i++)
        ioc->available_tokens[i] = &ioc->tokens[i];

    struct io_uring_params params;
    memset(&params, 0, sizeof(params));
    io_uring_queue_init_params(1024, &ioc->ring, &params);
}

int request_accept(struct IOContext *ioc, int fd, accept_cb cb, void *data)
{
    struct Token *token = get_token(ioc);
    if (unlikely(token == NULL))
        return -1;

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ioc->ring);
    if (unlikely(sqe == NULL)) {
        release_token(ioc, token);
        return -1;
    }

    io_uring_prep_accept(sqe, fd, NULL, NULL, 0);
    token->type = ACCEPT;
    token->fd = fd;
    token->cb = (Cb)cb;
    token->data = data;
    io_uring_sqe_set_data(sqe, (void *)token);
    return 1;
}

int request_read(struct IOContext *ioc, int fd, void *buffer, size_t size,
                 read_cb cb, void *data)
{
    struct Token *token = get_token(ioc);
    if (unlikely(token == NULL))
        return -1;

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ioc->ring);
    if (unlikely(sqe == NULL)) {
        release_token(ioc, token);
        return -1;
    }

    io_uring_prep_read(sqe, fd, buffer, size, 0);
    token->type = READ;
    token->fd = fd;
    token->cb = (Cb)cb;
    token->data = data;
    io_uring_sqe_set_data(sqe, (void *)token);
    return 1;
}

int request_write(struct IOContext *ioc, int fd, void *buffer, size_t size,
                  write_cb cb, void *data)
{
    struct Token *token = get_token(ioc);
    if (unlikely(token == NULL))
        return -1;

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ioc->ring);
    if (unlikely(sqe == NULL)) {
        release_token(ioc, token);
        return -1;
    }

    io_uring_prep_write(sqe, fd, buffer, size, 0);
    token->type = WRITE;
    token->fd = fd;
    token->cb = (Cb)cb;
    token->data = data;
    io_uring_sqe_set_data(sqe, (void *)token);
    return 1;
}

int process(struct IOContext *ioc, size_t batch)
{
    static struct io_uring_cqe *cqes[1024];
    if (!batch)
        return batch;

    batch = batch > 1024 ? 1024 : batch;
    struct io_uring_cqe *cqe = NULL;

    io_uring_submit(&ioc->ring);
    unsigned count = io_uring_peek_batch_cqe(&ioc->ring, &cqes[0], batch);

    if (count == 0) {
        int peek_result = io_uring_wait_cqe(&ioc->ring, &cqes[0]);
        if (peek_result != 0)
            return peek_result;

        count = 1;
    }

    for (int i = 0; i < count; ++i) {
        cqe = cqes[i];
        struct Token *token = (struct Token *)cqe->user_data;
        if (token->type == ACCEPT) {
            ((accept_cb)token->cb)(cqe->res, token->data);
            release_token(ioc, token);
        } else if (token->type == READ) {
            ((read_cb)token->cb)(cqe->res, token->data);
            release_token(ioc, token);
        } else if (token->type == WRITE) {
            ((write_cb)token->cb)(cqe->res, token->data);
            release_token(ioc, token);
        }
    }

    io_uring_cq_advance(&ioc->ring, count);
    return count;
}
