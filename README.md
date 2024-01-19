# Cring

Cring is a lightweight and efficient event loop library written in pure C, designed to simplify asynchronous programming using the io-uring interface. It provides a clean and straightforward API for building scalable network applications.

Moreover, Cring is designed with a thread-per-core model in mind.

## Features

- **Pure C:** Cring is written in pure C, making it easy to integrate into your C projects.
- **Efficient IO:** Utilizes the io-uring interface for high-performance asynchronous IO operations.
- **Simple API:** Provides a minimalistic and easy-to-use API for handling asynchronous tasks.

## Examples

Here's a simple example of using Cring to create an echo server:

```c
void client_handler(struct Executor *executor, void *data)
    int fd = *(int *)data;
    char buffer[PACKET_SIZE];

    ssize_t r_len = async_read(executor, fd, (void *)buffer, PACKET_SIZE);
    ...
    ssize_t w_len = async_write(executor, fd, (void *)buffer, r_len);
    ...

void echo_server(struct Executor *executor, void *data)
    while (true) {
        int fd = async_accept(executor, *(int *)data);
        async_exec(executor, &client_handler, &fd);
    }

int main()
    struct Executor executor;
    init_executor(&executor, 40, 1000);
    async_exec(&executor, &pingpong_server, &server_fd);
    run(&executor);

```

This example demonstrates the simplicity and elegance of using Cring to build an echo server that efficiently handles asynchronous IO operations.

For more usage examples, explore the [examples](examples) folder.

# Installation Instructions

## Prerequisites

Before building Cring, ensure that you have the following prerequisites installed on your system:

- Linux 5.6 or newer (support io_uring)
- CMake (version 3.16 or higher)
- A C compiler that supports C11 (e.g., GCC or Clang)

## Building Cring

1. Clone the Cring repository to your local machine:
```bash
   git clone https://github.com/your-username/Cring.git
```
2. pre-build
```bash
    cmake -B Release .
```
- Optional: Customize the build by adding CMake options. For example, to enable benchmarking, tests, and examples, run:
```bash
    cmake -B Release -DCRING_BENCHMARK=ON -DCRING_TEST=ON -DCRING_EXAMPLES=ON .
```

3. build
```bash
   cmake --build Release
```
- To perform a code quality check using Clang-Tidy (if available), run:
```bash
   cmake --build Release --target clang-tidy-check
```
## Benchmark
## Tests
## Licence
