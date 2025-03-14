#pragma once

#include "defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for release builds.
#if TRELEASE == 1
    #define LOG_DEBUG_ENABLED 0
    #define LOG_TRACE_ENABLED 0
#endif

typedef enum log_level{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5
} log_level;

/**
 * @brief Initializes logging system. Call twice; once with state = 0 to get required memory size,
 * then a second time passing allocated memory to state.
 * 
 * @param memory_requirement A pointer to hold the required memory size of internal state.
 * @param state 0 if just requesting memory requirement, otherwise allocated block of memory.
 * @return b8 TRUE on success; otherwise FALSE.
 */
b8 initialize_logging(u64* memory_requirement, void* state);
void shutdown_logging(void* state);

TAPI void log_output(log_level level, const char* message, ...);


#ifndef TFATAL
    // Logs a fatal-level message.
    #define TFATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);
#endif

#ifndef TERROR
    // Logs an error-level message.
    #define TERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
    // Logs a warning-level message.
    #define TWARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
    #define TWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
    // Logs a warning-level message.
    #define TINFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
    #define TINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
    // Logs a warning-level message.
    #define TDEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
    #define TDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
    // Logs a warning-level message.
    #define TTRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
    #define TTRACE(message, ...)
#endif