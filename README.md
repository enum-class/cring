# Cring

Cring is a lightweight and efficient event loop library written in pure C, designed to simplify asynchronous programming using the io-uring interface. It provides a clean and straightforward API for building scalable network applications.

## Features

- **Pure C:** Cring is written in pure C, making it easy to integrate into your C projects.
- **Efficient IO:** Utilizes the io-uring interface for high-performance asynchronous IO operations.
- **Simple API:** Provides a minimalistic and easy-to-use API for handling asynchronous tasks.

## Example

Here's a simple example of using Cring to create an echo server:

```c
void client_handler(Executor* executor, Socket fd) {
    uint8_t buffer[SIZE];

    while (true) {
        ssize_t r_len = async_read(executor, fd, buffer, SIZE);
        if (r_len <= 0)
            break;

        ssize_t w_len = async_write(executor, fd, buffer, r_len);
        if (w_len != r_len)
            break;
    }   
}

void echo_server(Executor* executor, Socket server_fd) {
    while (true) {
        int fd = async_accept(executor, server_fd);
        async_exec(executor, client_handler, fd);
    }   
}

int main() {
    Socket server_fd = setup_listen("127.0.0.1", 40000);
    Executor executor;

    init(&executor);
    async_exec(executor, echo_server, server_fd);
        
    run(executor);
    return 0;
}

```

This example demonstrates the simplicity and elegance of using Cring to build an echo server that efficiently handles asynchronous IO operations.

## Getting Started
