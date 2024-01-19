#include "IOContext.h"

#include <stdlib.h>
#include <string.h>

int free_io_context(struct IOContext *ioc)
{
    if (!ioc)
        return -1;

    io_uring_queue_exit(&ioc->ring);

    if (ioc->available_tokens[0])
        free(ioc->available_tokens[0]);
    if (ioc->available_tokens)
        free(ioc->available_tokens);

    memset(ioc, 0, sizeof(*ioc));
    return 0;
}

int init_io_context(struct IOContext *ioc, size_t capacity)
{
    if (!ioc || !capacity)
        return -1;

    memset(ioc, 0, sizeof(*ioc));
    ioc->capacity = align32pow2(capacity + 1);
    ioc->tail = ioc->capacity;

    ioc->available_tokens =
        (struct Token **)calloc(ioc->capacity, sizeof(struct Token *));
    struct Token *tokens =
        (struct Token *)calloc(ioc->capacity, sizeof(struct Token));

    if (!tokens || !ioc->available_tokens) {
        if (tokens)
            free(tokens);
        if (ioc->available_tokens)
            free(ioc->available_tokens);
        return -1;
    }

    for (uint32_t i = 0; i < ioc->capacity; ++i)
        ioc->available_tokens[i] = &tokens[i];

    struct io_uring_params params;
    memset(&params, 0, sizeof(params));

    int ret = io_uring_queue_init_params(ioc->capacity, &ioc->ring, &params);

    if (ret < 0) {
        free_io_context(ioc);
        return -1;
    }

    return 0;
}

int request_wait(struct IOContext *ioc, struct __kernel_timespec *ts,
                 wait_cb cb, void *data)
{
    struct Token *token = get_token(ioc);
    if (unlikely(token == NULL))
        return -1;

    struct io_uring_sqe *sqe = io_uring_get_sqe(&ioc->ring);
    if (unlikely(sqe == NULL)) {
        release_token(ioc, token);
        return -1;
    }

    io_uring_prep_timeout(sqe, ts, 0, 0);
    token->type = WAIT;
    token->cb = (Cb)cb;
    token->data = data;
    io_uring_sqe_set_data(sqe, (void *)token);

    return 0;
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
    return 0;
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
    return 0;
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
    return 0;
}

int process(struct IOContext *ioc, size_t batch)
{
    static struct io_uring_cqe *cqes[MAX_BATCH_SIZE];
    if (unlikely(batch == 0))
        return batch;

    batch = batch > MAX_BATCH_SIZE ? MAX_BATCH_SIZE : batch;
    struct io_uring_cqe *cqe = NULL;

    int ret = io_uring_submit(&ioc->ring);
    if (unlikely(ret < 0))
        return ret;

    unsigned count = io_uring_peek_batch_cqe(&ioc->ring, cqes, batch);
    if (count == 0) {
        int peek_result = io_uring_wait_cqe(&ioc->ring, cqes);
        if (unlikely(peek_result != 0))
            return peek_result;

        count = 1;
    }

    for (unsigned i = 0; i < count; ++i) {
        cqe = cqes[i];
        struct Token *token = (struct Token *)cqe->user_data;
        if (unlikely(token == NULL))
            continue;

        switch (token->type) {
        case ACCEPT:
            ((accept_cb)token->cb)(cqe->res, token->data);
            break;
        case READ:
            ((read_cb)token->cb)(cqe->res, token->data);
            break;
        case WRITE:
            ((write_cb)token->cb)(cqe->res, token->data);
            break;
        case WAIT:
            ((wait_cb)token->cb)(token->data);
            break;
        default:
            break;
        }
        // TODO: release token in batch
        release_token(ioc, token);
    }

    io_uring_cq_advance(&ioc->ring, count);
    return count;
}
