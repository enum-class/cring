#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <cring.h>

#define SIZE 1024

void client_handler(Executor* executor, Socket fd) {
    uint8_t buffer[SIZE];

    while (true) {
        ssize_t r_len = async_read(executor, fd, buffer, SIZE);
        if (r_len <= 0)
            break;

        ssize_t w_len = async_write(executor, fd, buffer, len);
        if (w_len != r_len)
            break;
    }
}

void echo_server(Executor* executor, Socket server_fd) {
    while (true) {
        int fd = async_accept(executor, server_fd);
        async_exec(executor, client_handler, fd);
    }
}

int main() {
    Socket server_fd = setup_listen("127.0.0.1", 40000);
    Executor executor;

    init(&executor);
    async_exec(executor, echo_server, server_fd);
    
    run(executor);
    return 0;
}
