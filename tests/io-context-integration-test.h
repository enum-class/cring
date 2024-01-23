#ifndef IO_CONTEXT_INTEGRATION_TEST_H
#define IO_CONTEXT_INTEGRATION_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test case for a valid request wait operation.
 *
 * This test verifies the behavior of the request wait operation when a valid
 * request is made. It checks if the wait operation completes successfully and
 * the expected result is obtained.
 *
 * @return 0 on success, non-zero on failure.
 */
int valid_request_wait(void);

/**
 * @brief Run integration tests for the IO context module.
 *
 * This function serves as a container for executing integration tests
 * related to the IO context module. It calls each individual test case
 * and reports the overall result.
 */
void run_io_context_integeration_tests(void);

/**
 * @brief Test case for the request read and write operations.
 *
 * This test verifies the correctness of the request read and write operations
 * in the IO context module. It checks if these operations behave as expected
 * and handle the specified scenarios appropriately.
 *
 * @return 0 on success, non-zero on failure.
 */
int request_read_write(void);

#ifdef __cplusplus
}
#endif

#endif
