#ifndef IO_CONTEXT_TEST_H
#define IO_CONTEXT_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

int ioc_invalid_init(void);

int ioc_capacity_check(void);

int ioc_invalid_free(void);

int ioc_valid_free(void);

int ioc_evaluate_tokens(void);

void run_io_context_tests(void);

#ifdef __cplusplus
}
#endif

#endif
