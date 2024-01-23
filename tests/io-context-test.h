#ifndef IO_CONTEXT_TEST_H
#define IO_CONTEXT_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test case for an invalid initialization of the IO context.
 *
 * This test checks the behavior when attempting to initialize the IO context
 * with invalid parameters or conditions. It ensures that the initialization
 * fails as expected and returns an error code.
 *
 * @return 0 on success, non-zero on failure.
 */
int ioc_invalid_init(void);

/**
 * @brief Test case to check IO context capacity handling.
 *
 * This test evaluates the handling of the IO context's capacity, verifying
 * that it correctly reflects the specified capacity and behaves appropriately
 * when exceeding the defined limits.
 *
 * @return 0 on success, non-zero on failure.
 */
int ioc_capacity_check(void);

/**
 * @brief Test case for an invalid freeing of the IO context.
 *
 * This test verifies the behavior when attempting to free the IO context
 * with invalid parameters or conditions. It ensures that the freeing operation
 * fails as expected and returns an error code.
 *
 * @return 0 on success, non-zero on failure.
 */
int ioc_invalid_free(void);

/**
 * @brief Test case for a valid freeing of the IO context.
 *
 * This test checks the behavior when successfully freeing the IO context,
 * ensuring that it deallocates resources appropriately and cleans up properly.
 *
 * @return 0 on success, non-zero on failure.
 */
int ioc_valid_free(void);

/**
 * @brief Test case to evaluate the handling of IO tokens by the IO context.
 *
 * This test examines how the IO context manages tokens, ensuring that tokens
 * are properly associated and released based on the specified conditions.
 *
 * @return 0 on success, non-zero on failure.
 */
int ioc_evaluate_tokens(void);

/**
 * @brief Run tests for the IO context module.
 *
 * This function serves as a container for executing all IO context tests.
 * It calls each individual test case and reports the overall result.
 */
void run_io_context_tests(void);

#ifdef __cplusplus
}
#endif

#endif
