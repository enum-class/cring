#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <Executor.h>

#include "utils.h"

#define PACKET_SIZE 1024
#define MESSAGES_COUNT 150

int port = 40000;
char *address = "127.0.0.1";
int threads = 1;
int connections = 1;
int qps = -1;

void pingpong_client(struct Executor *executor, void *data)
{
    (void)data;
    int fd = connect_to_server(address, port);
    if (fd < 0) {
        fprintf(stderr, "Connection to server %s:%d intrrupted\n", address,
                port);
        return;
    }

    char buffer[PACKET_SIZE] = { 0 };

    int counter = 0;
    while (++counter <= MESSAGES_COUNT) {
        ssize_t w_len = async_write(executor, fd, (void *)buffer, PACKET_SIZE);
        if (w_len <= 0) {
            fprintf(stderr, "Error in sending message %zd\n", w_len);
            break;
        }

        ssize_t r_len = async_read(executor, fd, (void *)buffer, w_len);
        if (r_len <= 0) {
            fprintf(stderr, "Error in reading message %zd\n", r_len);
            break;
        }
    }

    close(fd);
}

void *run_thread(void *data)
{
    (void)data;
    struct Executor executor;
    init_executor(&executor, 400, 1000);
    printf("stating ...\n");
    for (int i = 0; i < connections; ++i)
        async_exec(&executor, &pingpong_client, &i);

    run(&executor);

    free_executor(&executor);
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
            printf("QPS not supported");
            // TODO: qps = atoi(optarg);
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

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int t = 0; t < threads; ++t)
        pthread_create(&thread_holder[t], NULL, &run_thread, NULL);

    for (int t = 0; t < threads; ++t)
        pthread_join(thread_holder[t], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    free(thread_holder);

    printf("Real QPS: %.4f\n",
           ((double)threads * (double)connections * MESSAGES_COUNT) /
               elapsed_time);
    printf("AVG RTT: %.4f us\n",
           elapsed_time * 1e6 /
               ((double)threads * (double)connections * MESSAGES_COUNT));

    return 0;
}
