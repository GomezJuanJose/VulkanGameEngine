#pragma once

#include "defines.h"

// Returns the length of the given string.
TAPI u64 string_length(const char* str);

TAPI char* string_duplicate(const char* str);

// Case-sensitive string comparison. True if the same, otherwise false.
TAPI b8 strings_equals(const char* str0, const char* str1);

// Performs string formatting to dest given format string and parameters.
TAPI i32 string_format(char* dest, const char* format, ...);

/**
 * Performs variadic string formatting to dest given format string and va_list.
 * @param dest The destination for the formatted string.
 * @param format The string to be formatted.
 * @param va_list The variadic argument list.
 * @returns The size of the data written.
 */
TAPI i32 string_format_v(char* dest, const char* format, void* va_list);