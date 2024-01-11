#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <Executor.h>

#include "utils.h"

#define PACKET_SIZE 1024
#define FRAME_COUNT 400
#define RING_SIZE 1000

int server_fd = -1;

void client_handler(struct Executor *executor, void *data)
{
    int fd = *(int *)data;
    uint8_t buffer[PACKET_SIZE];

    while (true) {
        ssize_t r_len = async_read(executor, fd, (void *)buffer, PACKET_SIZE);
        if (r_len <= 0)
            break;
        ssize_t w_len = async_write(executor, fd, (void *)buffer, r_len);
        if (w_len != r_len)
            break;
    }
}

void pingpong_server(struct Executor *executor, void *data)
{
    while (true) {
        int fd = async_accept(executor, *(int *)data);
        async_exec(executor, &client_handler, &fd);
    }
}

int main(int argc, char *argv[])
{
    char *address = "127.0.0.1";
    int port = 40000;
    int core = 1;

    int opt;
    while ((opt = getopt(argc, argv, "p:a:c:")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'a':
            address = optarg;
            break;
        case 'c':
            core = atoi(optarg);
            break;
        default:
            fprintf(stderr, "Usage: %s [-p port] [-a address] [-c core]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    server_fd = setup_listen(address, port);

    bind_cpu(core);

    struct Executor executor;
    init_executor(&executor, FRAME_COUNT, RING_SIZE);
    async_exec(&executor, &pingpong_server, &server_fd);

    run(&executor);
    return 0;
}
