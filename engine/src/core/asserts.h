#pragma once

#include "defines.h"

// Disable assertions by commenting out the below line
#define TASSERTIONS_ENABLED

#ifdef TASSERTIONS_ENABLED
    #if _MSC_VER
        #include <intrin.h>
        #define debugbreak() __debugbreak()
    #else
        #define debugbreak() __builtin_trap()
    #endif

    TAPI void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

    #define TASSERT(expr)                                                   \
        {                                                                   \
            if(expr){                                                       \
            }else {                                                         \
                report_assertion_failure(#expr, "", __FILE__, __LINE__);    \
                debugbreak();                                               \
            }                                                               \
        }                                                                   

    #define TASSERT_MSG(expr, message)                                          \
        {                                                                       \
            if(expr){                                                           \
            }else {                                                             \
                report_assertion_failure(#expr, message, __FILE__, __LINE__);   \
                debugbreak();                                                   \
            }                                                                   \
        }                                                                       

    #ifdef _DEBUG
        #define TASSERT_DEBUG(expr)                                         \
        {                                                                   \
            if(expr){                                                       \
            }else {                                                         \
                report_assertion_failure(#expr, "", __FILE__, __LINE__);    \
                debugbreak();                                               \
            }                                                               \
        }                                                                   
    #else
        #define TASSERT_DEBUG(expr)
    #endif
#else
    #define TASSERT(expr)
    #define TASSERT_MSG(expr, message)
    #define TASSERT_DEBUG(expr)
#endif