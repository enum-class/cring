#include <pthread.h>
#include <stdio.h>

#include "io-context-test.h"
#include "executor-test.h"
#include "utils.h"

#define THREADS_NO 4

void *run_tests(void *data MAYBE_UNUSED)
{
    run_io_context_tests();
    run_executor_tests();
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t threads[THREADS_NO];
    for (int t = 0; t < THREADS_NO; ++t)
        pthread_create(&threads[t], NULL, &run_tests, NULL);

    for (int t = 0; t < THREADS_NO; ++t)
        pthread_join(threads[t], NULL);

    printf("%s done\n", __FILE__);
}
