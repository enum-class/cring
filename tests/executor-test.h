#ifndef EXECUTOR_TEST_H
#define EXECUTOR_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test case for initializing an executor with invalid parameters.
 *
 * This test verifies the behavior of the executor initialization function
 * when invalid parameters are provided. It checks if the function returns
 * the expected error code or exhibits the expected behavior in such cases.
 *
 * @return 0 on success, non-zero on failure.
 */
int executor_invalid_init(void);

/**
 * @brief Test case for initializing an executor with valid parameters.
 *
 * This test verifies the correct initialization of an executor with valid
 * parameters. It checks if the initialization process completes successfully
 * and the executor is ready for use after the function call.
 *
 * @return 0 on success, non-zero on failure.
 */
int executor_valid_init(void);

/**
 * @brief Test case for freeing an executor with invalid parameters.
 *
 * This test verifies the behavior of the executor freeing function when
 * invalid parameters are provided. It checks if the function returns the
 * expected error code or exhibits the expected behavior in such cases.
 *
 * @return 0 on success, non-zero on failure.
 */
int executor_invalid_free(void);

/**
 * @brief Run all executor-related tests.
 *
 * This function serves as a container for executing all the test cases
 * related to the executor module. It calls each individual test case
 * and reports the overall result.
 */
void run_executor_tests(void);

#ifdef __cplusplus
}
#endif

#endif
