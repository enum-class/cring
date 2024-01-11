#ifndef UTILS_H_
#define UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sched.h>

int setup_listen(const char *addr, int port)
{
    int enable = 1;
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    struct sockaddr_in srv_addr;
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(port);
    srv_addr.sin_addr.s_addr = inet_addr(addr);

    bind(sock, (const struct sockaddr *)(&srv_addr), sizeof(srv_addr));
    listen(sock, 10);
    return sock;
}

#ifdef __cplusplus
}
#endif

#endif
