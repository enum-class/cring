#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <Executor.h>
#include "utils.h"

#define PACKET_SIZE 1024
#define JOIN_MESSAGE "Someone joined"
#define LEFT_MESSAGE "Someone left"

struct Message {
    char buffer[PACKET_SIZE];
    size_t length;
    struct Message *next;
};

struct ChatSession;

struct Participant {
    struct ChatSession *session;
    struct Participant *next;
};

struct ChatRoom {
    struct Participant *participants;
};

struct ChatSession {
    int fd;
    struct ChatRoom *room;
    struct Message *write_messages;
};

int push(struct Message **messages, const char *msg, size_t len)
{
    if (messages == NULL || msg == NULL || len == 0)
        return 0;

    struct Message *new_msg = (struct Message *)malloc(sizeof(struct Message));
    if (new_msg == NULL)
        return 0;

    strncpy(new_msg->buffer, msg, len);
    new_msg->next = NULL;
    new_msg->length = len;

    if (*messages == NULL) {
        *messages = new_msg;
    } else {
        struct Message *current = *messages;
        while (current->next != NULL)
            current = current->next;

        current->next = new_msg;
    }

    return 1;
}

int pop(struct Message **messages, char *dst)
{
    if (messages == NULL || dst == NULL)
        return 0;

    struct Message *current = *messages;
    if (!current)
        return 0;

    if (current->next)
        *messages = current->next;
    else
        *messages = NULL;

    size_t length = current->length;
    strncpy(dst, current->buffer, length);
    free(current);

    return length;
}

void sendto_room(struct ChatRoom *room, struct ChatSession *session,
                 const char *msg, size_t length)
{
    if (!room || !msg)
        return;

    struct Participant *current = room->participants;
    while (current) {
        if (current->session != session)
            push(&current->session->write_messages, msg, length);
        current = current->next;
    }
}

int join(struct ChatRoom *room, struct ChatSession *session)
{
    if (!room || !session)
        return 0;

    struct Participant *new_par =
        (struct Participant *)malloc(sizeof(struct Participant));
    if (!new_par)
        return 0;

    new_par->next = NULL;
    new_par->session = session;

    if (room->participants == NULL) {
        room->participants = new_par;
    } else {
        struct Participant *current = room->participants;
        while (current->next)
            current = current->next;
        current->next = new_par;
    }

    sendto_room(room, session, JOIN_MESSAGE, strlen(JOIN_MESSAGE));
    return 1;
}

int leave(struct ChatRoom *room, struct ChatSession *session)
{
    if (!room || !session)
        return 0;

    struct Participant *current = room->participants;
    if (current->session == session) {
        room->participants = current->next;
    } else {
        while (current->next) {
            if (current->next->session == session)
                break;
            current = current->next;
        }
        struct Participant *tmp = current->next;
        current->next = current->next->next;
        current = tmp;
    }

    free(current);
    sendto_room(room, NULL, LEFT_MESSAGE, strlen(LEFT_MESSAGE));
    return 1;
}

void stop(struct ChatSession *session)
{
    if (!session)
        return;
    leave(session->room, session);
    close(session->fd);
    free(session);
    session = NULL;
}

void reader(struct Executor *executor, void *data)
{
    struct ChatSession *session = (struct ChatSession *)data;

    char buffer[PACKET_SIZE];

    while (session) {
        ssize_t r_len = async_read(executor, session->fd, buffer, PACKET_SIZE);
        if (r_len <= 0) {
            fprintf(stderr, "Error in reading from socket\n");
            stop(session);
            return;
        }

        sendto_room(session->room, session, buffer, r_len);
    }
}

void writer(struct Executor *executor, void *data)
{
    char buffer[PACKET_SIZE];
    struct ChatSession *session = (struct ChatSession *)data;

    while (session) {
        ssize_t len = pop(&session->write_messages, buffer);
        if (len <= 0) {
            struct __kernel_timespec ts;
            ts.tv_sec = 1;
            ts.tv_nsec = 0;
            async_wait(executor, &ts);
            continue;
        }

        ssize_t w_len = async_write(executor, session->fd, buffer, len);
        if (w_len != len) {
            stop(session);
            return;
        }
    }
}

void start(struct Executor *executor, void *data)
{
    struct ChatSession *session = (struct ChatSession *)data;

    join(session->room, session);

    async_exec(executor, &reader, session);
    async_exec(executor, &writer, session);
}

void chat_server(struct Executor *executor, void *data)
{
    struct ChatRoom room;
    int server_fd = *(int *)data;

    while (true) {
        int fd = async_accept(executor, server_fd);

        struct ChatSession *session =
            (struct ChatSession *)malloc(sizeof(struct ChatSession));
        session->fd = fd;
        session->room = &room;
        session->write_messages = NULL;

        async_exec(executor, &start, session);
    }
}

int main(void)
{
    int server_fd = setup_listen("127.0.0.1", 40000);

    struct Executor executor;
    if (init_executor(&executor, 40, 1000) < 0) {
        fprintf(stderr, "Error in init_executor\n");
        exit(EXIT_FAILURE);
    }

    async_exec(&executor, &chat_server, &server_fd);

    run(&executor);
    free_executor(&executor);
}
