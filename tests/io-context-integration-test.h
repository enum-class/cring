#ifndef IO_CONTEXT_INTEGRATION_TEST_H
#define IO_CONTEXT_INTEGRATION_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

int valid_request_wait(void);

void run_io_context_integeration_tests(void);

int request_read_write(void);

#ifdef __cplusplus
}
#endif

#endif
