#include <assert.h>

#include <IOContext.h>

#include "io-context-test.h"
#include "utils.h"

int ioc_invalid_init(void)
{
    MAYBE_UNUSED size_t valid_capacity = 10;
    MAYBE_UNUSED size_t invalid_capacity = 0;
    MAYBE_UNUSED struct IOContext valid_ioc;

    assert(init_io_context(NULL, invalid_capacity) == -1);
    assert(init_io_context(NULL, valid_capacity) == -1);

    assert(init_io_context(&valid_ioc, invalid_capacity) == -1);
    return 0;
}

int ioc_capacity_check(void)
{
    MAYBE_UNUSED struct IOContext ioc;
    MAYBE_UNUSED size_t capacity = 10;
    MAYBE_UNUSED size_t expected_capacity = 16;

    assert(init_io_context(&ioc, capacity) == 0);
    assert(ioc.capacity == expected_capacity);
    assert(ioc.tail == expected_capacity);

    assert(free_io_context(&ioc) == 0);
    assert(is_memory_set_zero(&ioc, sizeof(struct IOContext)) == 1);

    return 0;
}

int ioc_invalid_free(void)
{
    assert(free_io_context(NULL) == -1);
    return 0;
}

int ioc_valid_free(void)
{
    MAYBE_UNUSED struct IOContext ioc;
    MAYBE_UNUSED size_t capacity = 10;

    assert(init_io_context(&ioc, capacity) == 0);
    assert(free_io_context(&ioc) == 0);
    assert(ioc.tail == 0);
    assert(ioc.capacity == 0);
    assert(ioc.available_tokens == NULL);
    return 0;
}

int ioc_evaluate_tokens(void)
{
    struct IOContext ioc;
    MAYBE_UNUSED size_t capacity = 40;
    size_t expected_capacity = 64;

    assert(init_io_context(&ioc, capacity) == 0);
    assert(ioc.capacity == expected_capacity);

    MAYBE_UNUSED struct Token *token = NULL;
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

void run_io_context_tests(void)
{
    printf("ioc_invalid_init %d\n", ioc_invalid_init());
    printf("ioc_capacity_check %d\n", ioc_capacity_check());
    printf("ioc_invalid_free %d\n", ioc_invalid_free());
    printf("ioc_valid_free %d\n", ioc_valid_free());
    printf("ioc_evaluate_tokens %d\n", ioc_evaluate_tokens());
}
