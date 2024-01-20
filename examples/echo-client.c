#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Executor.h>

#include "utils.h"

#define CLIENT_NO 10
#define PACKET_SIZE 1024

void echo_client(struct Executor *executor, void *data)
{
    (void)data;
    int client_id = rand();
    printf("Client %d started ...\n", client_id);
    int fd = connect_to_server("127.0.0.1", 40000);
    if (fd < 0) {
        fprintf(stderr, "Connection to server intrrupted");
        return;
    }

    char send_buffer[PACKET_SIZE] = { 0 };
    snprintf(send_buffer, sizeof(send_buffer), "Hello from client %d",
             client_id);
    ssize_t length = strlen(send_buffer);

    ssize_t w_len = async_write(executor, fd, (void *)send_buffer, length);
    if (w_len != length) {
        fprintf(
            stderr,
            "Error in sending message from client %d, expected length : %zd, real length : %ld\n",
            client_id, length, w_len);
        return;
    }

    struct __kernel_timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    async_wait(executor, &ts);

    char recv_buffer[PACKET_SIZE] = { 0 };
    ssize_t r_len = async_read(executor, fd, (void *)recv_buffer, length);
    if (r_len != length) {
        fprintf(
            stderr,
            "Error in recieving message from client %d, expected length : %zd, real length : %ld\n",
            client_id, length, r_len);
        return;
    }

    close(fd);
    printf("Bye Bye %d\n", client_id);
}

int main(void)
{
    struct Executor executor;
    init_executor(&executor, 400, 1000);

    for (int i = 0; i < CLIENT_NO; ++i)
        async_exec(&executor, &echo_client, NULL);

    run(&executor);
}
