#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Executor.h>

#include "utils.h"

#define PACKET_SIZE 1024

void client_handler(struct Executor *executor, void *data)
{
    int fd = *(int *)data;
    char buffer[PACKET_SIZE];

    ssize_t r_len = async_read(executor, fd, (void *)buffer, PACKET_SIZE);
    if (r_len <= 0) {
        fprintf(stderr, "Error in reading from socket\n");
        return;
    }

    printf("Recieved %s\n", buffer);

    ssize_t w_len = async_write(executor, fd, (void *)buffer, r_len);
    if (w_len != r_len) {
        fprintf(stderr, "Error in writing in socket\n");
        return;
    }

    close(fd);
}

void pingpong_server(struct Executor *executor, void *data)
{
    printf("Started ...\n");
    while (true) {
        int fd = async_accept(executor, *(int *)data);
        async_exec(executor, &client_handler, &fd);
    }
}

int main()
{
    int server_fd = setup_listen("127.0.0.1", 40000);

    struct Executor executor;
    if (init_executor(&executor, 40, 1000) < 0) {
        fprintf(stderr, "Error in init_executor\n");
        exit(EXIT_FAILURE);
    }

    async_exec(&executor, &pingpong_server, &server_fd);

    run(&executor);
    return 0;
}
