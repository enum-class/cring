#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <Executor.h>

#include "utils.h"

#define PACKET_SIZE 1024

int server_fd = -1;

void client_handler(struct Executor* executor, void* data) {
    printf("client_handler\n");
    int fd = *(int*)data;
    uint8_t buffer[PACKET_SIZE];

    while (true) {
        ssize_t r_len = async_read(executor, fd, (void*)buffer, PACKET_SIZE);
        if (r_len <= 0)
            break;
        ssize_t w_len = async_write(executor, fd, (void*)buffer, r_len);
        if (w_len != r_len)
            break;
    }
}

void pingpong_server(struct Executor* executor, void* data) {
    while (true) {
        printf("pingpong_server\n");
        int fd = async_accept(executor, *(int*)data);
        async_exec(executor, &client_handler, &fd);
    }
}

int main() {
    server_fd = setup_listen("127.0.0.1", 40000);

    struct Executor executor;
    init_executor(&executor, 10);
    async_exec(&executor, &pingpong_server, &server_fd);
    
    run(&executor);
    return 0;
}
