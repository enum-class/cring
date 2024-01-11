#ifndef UTILS_H_
#define UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

int connect_to_server(const char *addr, int port)
{
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, addr, &server_address.sin_addr) <= 0)
        return -1;

    int client_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (client_fd < 0)
        return -1;

    if (connect(client_fd, (struct sockaddr *)&server_address,
                sizeof(server_address)) < 0)
        return -1;

    return client_fd;
}

#ifdef __cplusplus
}
#endif

#endif
