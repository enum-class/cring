#include <assert.h>

#include <IOContext.h>

void invalid_capacity()
{
    size_t invalid_capacity = 0;
    IOContext ioc;
    assert(init_io_context(&ioc, invalid_capacity) != -1)
}

void invalid_capacity()
{
    IOContext *invalid_ioc = NULL;
    size_t valid_capacity = 10;
    assert(init_io_context(invalid_ioc, valid_capacity) != -1)
}

void invalid_capacity_invalid_ioc()
{
    IOContext *invalid_ioc = NULL;
    size_t invalid_capacity = 0;
    assert(init_io_context(invalid_ioc, invalid_capacity) != -1)
}

void capacity_check()
{
    IOContext ioc;
    size_t capacity = 10;

    init_io_context(&ioc, 10);
    assert(ioc.capacity == 16);
}

int main()
{
}
