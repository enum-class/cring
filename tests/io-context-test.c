#include <assert.h>
#include <pthread.h>

#include <IOContext.h>

#define THREADS_NO 4

int invalid_init()
{
    size_t valid_capacity = 10;
    size_t invalid_capacity = 0;
    struct IOContext valid_ioc;

    assert(init_io_context(NULL, invalid_capacity) == -1);
    assert(init_io_context(NULL, valid_capacity) == -1);
    assert(init_io_context(&valid_ioc, invalid_capacity) == -1);
    return 0;
}

int capacity_check()
{
    struct IOContext ioc;
    size_t capacity = 10;
    size_t expected_capacity = 16;

    assert(init_io_context(&ioc, capacity) == 0);
    assert(ioc.capacity == expected_capacity);
    return 0;
}

int invalid_free()
{
    struct IOContext uninitialized_ioc;

    assert(free_io_context(NULL) == -1);

    assert(free_io_context(&uninitialized_ioc) == 0);
    assert(uninitialized_ioc.tail == 0);
    assert(uninitialized_ioc.capacity == 0);
    assert(uninitialized_ioc.available_tokens == NULL);
    return 0;
}

int valid_free()
{
    struct IOContext ioc;
    size_t capacity = 10;

    assert(init_io_context(&ioc, capacity) == 0);
    assert(free_io_context(&ioc) == 0);
    assert(ioc.tail == 0);
    assert(ioc.capacity == 0);
    assert(ioc.available_tokens == NULL);
    return 0;
}

int evaluate_tokens()
{
    struct IOContext ioc;
    size_t capacity = 40;
    size_t expected_capacity = 64;

    assert(init_io_context(&ioc, capacity) == 0);
    assert(ioc.capacity == expected_capacity);

    struct Token *token = NULL;
    for (size_t i = 0; i < expected_capacity; ++i) {
        token = get_token(&ioc);
        assert(token != NULL);
    }

    assert(ioc.tail == 0);

    for (size_t i = 0; i < expected_capacity; ++i) {
        token = get_token(&ioc);
        assert(token == NULL);
    }

    assert(ioc.tail == 0);

    for (size_t i = 0; i < expected_capacity; ++i) {
        release_token(&ioc, ioc.available_tokens[i]);
    }

    assert(ioc.tail == expected_capacity);

    for (size_t i = 0; i < expected_capacity; ++i) {
        release_token(&ioc, ioc.available_tokens[i]);
    }

    assert(ioc.tail == expected_capacity);
    assert(free_io_context(&ioc) == 0);

    return 0;
}

void *run(void *data)
{
    (void)data;
    printf("invalid_init %d\n", invalid_init());
    printf("capacity_check %d\n", capacity_check());
    printf("invalid_free %d\n", invalid_free());
    printf("valid_free %d\n", valid_free());
    printf("evaluate_tokens %d\n", evaluate_tokens());

    pthread_exit(NULL);
}

int main()
{
    pthread_t threads[THREADS_NO];
    for (int t = 0; t < THREADS_NO; ++t)
        pthread_create(&threads[t], NULL, &run, NULL);

    for (int t = 0; t < THREADS_NO; ++t)
        pthread_join(threads[t], NULL);

    printf("%s done\n", __FILE__);
}
