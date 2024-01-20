#ifndef EXECUTOR_TEST_H
#define EXECUTOR_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

int executor_invalid_init(void);

int executor_valid_init(void);

int executor_invalid_free(void);

void run_executor_tests(void);

#ifdef __cplusplus
}
#endif

#endif
