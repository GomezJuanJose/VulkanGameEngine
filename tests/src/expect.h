#pragma once

#include <core/logger.h>
#include <math/tmath.h>

/**
 * @brief Expects expected to be equal to actual.
 */
#define expect_should_be(expected, actual)                                                              \
    if(actual != expected){                                                                             \
        TERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return FALSE;                                                                                   \
    }

/**
 * @brief Expects expected to NOT be equal to actual.
 */
#define expect_should_not_be(expected, actual)                                                                      \
    if(actual == expected){                                                                                         \
        TERROR("--> Expected %d != %d, but they are equal. File: %s:%d.", expected, actual, __FILE__, __LINE__);    \
        return FALSE;                                                                                               \
    }

/**
 * @brief Expects expected to be actual given a tolerance of T_FLOAT_EPSILON
 */
#define expect_float_to_be(expected, actual)                                                        \
    if(tabs(expected-actual) > 0.001f){                                                             \
        TERROR("--> Expected %f, but got: %f. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return FALSE;                                                                               \
    }

/**
 * @brief Expects actual to be true
 */
#define expect_to_be_true(actual)                                                                           \
    if(actual != TRUE){                                                                                     \
        TERROR("--> Expected TRUE, but got: FALSE. File: %s:%d.", __FILE__, __LINE__);    \
        return FALSE;                                                                                       \
    }

/**
 * @brief Expects actual to be false
 */
#define expect_to_be_false(actual)                                                                          \
    if(actual != FALSE){                                                                                    \
        TERROR("--> Expected FALSE, but got: TRUE. File: %s:%d.", __FILE__, __LINE__);    \
        return FALSE;                                                                                       \
    }