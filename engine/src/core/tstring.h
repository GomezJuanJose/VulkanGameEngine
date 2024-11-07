#pragma once

#include "defines.h"

// Returns the length of the given string.
TAPI u64 string_length(const char* str);

TAPI char* string_duplicate(const char* str);

// Case-sensitive string comparison. True if the same, otherwise false.
TAPI b8 strings_equals(const char* str0, const char* str1);