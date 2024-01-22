#ifndef UTILS_H_
#define UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/time.h>

#include <Executor.h>

#define MAYBE_UNUSED __attribute__((unused))

static inline int resolve_connect(const char *addr, int port)
{
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, addr, &address.sin_addr) <= 0)
        return -1;

    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return -1;

    if (connect(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        return -1;

    return fd;
}

static inline int bind_to(const char *addr, int port)
{
    int enable = 1;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(addr);

    bind(sock, (const struct sockaddr *)(&address), sizeof(address));

    return sock;
}

static inline void msec_to_ts(struct __kernel_timespec *ts, unsigned int msec)
{
    ts->tv_sec = msec / 1000;
    ts->tv_nsec = (msec % 1000) * 1000000;
}

static inline int check_elapced_time_msec(const struct timeval *start,
                                          unsigned int duration_ms)
{
    struct timeval now;
    gettimeofday(&now, NULL);

    long long sec = now.tv_sec - start->tv_sec;
    long long usec = now.tv_usec - start->tv_usec;

    sec = now.tv_sec - start->tv_sec;
    usec = (now.tv_usec - start->tv_usec);
    if (sec > 0 && usec < 0) {
        sec--;
        usec += 1000000;
    }

    sec *= 1000;
    usec /= 1000;

    long long msec = sec + usec;

    if (msec >= duration_ms / 2 && msec <= (duration_ms * 3) / 2)
        return 0;

    return -1;
}

static inline int is_memory_set_zero(void *data, size_t size)
{
    unsigned char *mm = (unsigned char *)data;
    return (*mm == 0) && memcmp(mm, mm + 1, size - 1) == 0;
}

#ifdef __cplusplus
}
#endif

#endif
