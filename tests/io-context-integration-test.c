#include <assert.h>
#include <stdlib.h>
#include <pthread.h>

#include <IOContext.h>

#include "io-context-test.h"
#include "utils.h"

#define PACKET_SIZE 1024
#define MESSAGE "radnom message"

int valid_request_wait(void)
{
    struct IOContext ioc;
    MAYBE_UNUSED size_t capacity = 1;
    MAYBE_UNUSED uint64_t idx = rand();

    unsigned int msec = 200;
    struct __kernel_timespec ts;
    msec_to_ts(&ts, msec);

    assert(init_io_context(&ioc, capacity) == 0);
    assert(request_wait(&ioc, &ts, NULL, &idx) == 0);
    assert(io_uring_submit(&ioc.ring) == 1);

    struct timeval start;
    gettimeofday(&start, NULL);

    struct io_uring_cqe *cqe = NULL;
    assert(io_uring_wait_cqe(&ioc.ring, &cqe) == 0);
    assert(check_elapced_time_msec(&start, msec) == 0);

    MAYBE_UNUSED struct Token *token = (struct Token *)cqe->user_data;
    assert(token->cb == NULL);
    assert(*(uint64_t *)token->data == idx);

    io_uring_cqe_seen(&ioc.ring, cqe);
    return 0;
}

static void *do_write(void *data)
{
    (void)data;
    MAYBE_UNUSED char buffer[PACKET_SIZE] = { 0 };
    strncpy(buffer, MESSAGE, sizeof(buffer));

    struct IOContext ioc;
    MAYBE_UNUSED size_t capacity = 1;
    MAYBE_UNUSED uint64_t idx = rand();
    MAYBE_UNUSED int fd = resolve_connect("127.0.0.1", 40000);

    assert(init_io_context(&ioc, capacity) == 0);
    assert(request_write(&ioc, fd, buffer, PACKET_SIZE, NULL, &idx) == 0);
    assert(io_uring_submit(&ioc.ring) == 1);

    struct io_uring_cqe *cqe = NULL;
    assert(io_uring_wait_cqe(&ioc.ring, &cqe) == 0);

    MAYBE_UNUSED struct Token *token = (struct Token *)cqe->user_data;
    assert(cqe->res == PACKET_SIZE);
    assert(token->cb == NULL);
    assert(*(uint64_t *)token->data == idx);
    assert(token->type == WRITE);

    io_uring_cqe_seen(&ioc.ring, cqe);
    return 0;
}

int request_read_write(void)
{
    MAYBE_UNUSED char buffer[PACKET_SIZE] = { 0 };
    struct IOContext ioc;
    MAYBE_UNUSED size_t capacity = 1;
    MAYBE_UNUSED uint64_t idx = rand();
    MAYBE_UNUSED int fd = bind_to("127.0.0.1", 40000);
    assert(fd > 0);

    assert(init_io_context(&ioc, capacity) == 0);
    assert(request_read(&ioc, fd, buffer, PACKET_SIZE, NULL, &idx) == 0);
    assert(io_uring_submit(&ioc.ring) == 1);

    pthread_t write_thread;
    pthread_create(&write_thread, NULL, &do_write, NULL);

    struct io_uring_cqe *cqe = NULL;
    assert(io_uring_wait_cqe(&ioc.ring, &cqe) == 0);

    MAYBE_UNUSED struct Token *token = (struct Token *)cqe->user_data;
    assert(cqe->res == PACKET_SIZE);
    assert(token->cb == NULL);
    assert(*(uint64_t *)token->data == idx);
    assert(token->type == READ);
    assert(strcmp(buffer, MESSAGE) == 0);

    io_uring_cqe_seen(&ioc.ring, cqe);

    pthread_join(write_thread, NULL);

    do_write(NULL);
    return 0;
}

void run_io_context_integeration_tests(void)
{
    printf("valid_request_wait %d\n", valid_request_wait());
    printf("request_read_write %d\n", request_read_write());
}
