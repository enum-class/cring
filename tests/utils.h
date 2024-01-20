#ifndef UTILS_H_
#define UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#define MAYBE_UNUSED __attribute__((unused))

static inline int is_memory_set_zero(void *data, size_t size)
{
    unsigned char *mm = (unsigned char *)data;
    return (*mm == 0) && memcmp(mm, mm + 1, size - 1) == 0;
}

#ifdef __cplusplus
}
#endif

#endif
