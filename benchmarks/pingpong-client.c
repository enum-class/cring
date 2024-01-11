#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include <Executor.h>

#include "utils.h"

#define PACKET_SIZE 1024

int port = -1;
char *address = NULL;
int threads = 1;
int connections = 1;
int qps = -1;

void pingpong_client(struct Executor *executor, void *data)
{
    struct __kernel_timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;

    int fd = connect_to_server(address, port);
    if (fd < 0) {
        fprintf(stderr, "Connection to server %s:%d intrrupted\n", address,
                port);
        return;
    }

    srand(time(NULL));
    char buffer[PACKET_SIZE] = { 0 };

    int counter = 0;
    while (++counter <= 10) {
        ssize_t w_len = async_write(executor, fd, (void *)buffer, PACKET_SIZE);
        if (w_len <= 0)
            break;

        ssize_t r_len = async_read(executor, fd, (void *)buffer, w_len);
        if (r_len <= 0)
            break;

        async_wait(executor, &ts);
    }
}

void *run_thread(void *)
{
    struct Executor executor;
    init_executor(&executor, 400, 1000);

    for (int i = 0; i < connections; ++i)
        async_exec(&executor, &pingpong_client, &i);

    run(&executor);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "p:a:t:c:q:")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 'a':
            address = optarg;
            break;
        case 't':
            threads = atoi(optarg);
            break;
        case 'c':
            connections = atoi(optarg);
            break;
        case 'q':
            qps = atoi(optarg);
            break;
        default:
            fprintf(
                stderr,
                "Usage: %s [-p port] [-a address] [-t threads] [-c connections per thread] [-q query per second limit]\n",
                argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    pthread_t *thread_holder = malloc(threads * sizeof(pthread_t));
    if (thread_holder == NULL)
        exit(EXIT_FAILURE);

    for (int t = 0; t < threads; ++t)
        pthread_create(&thread_holder[t], NULL, &run_thread, NULL);

    for (int t = 0; t < threads; ++t)
        pthread_join(thread_holder[t], NULL);

    free(thread_holder);

    return 0;
}
