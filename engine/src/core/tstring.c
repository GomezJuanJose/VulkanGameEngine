#include "core/tstring.h"
#include "core/tmemory.h"

#include <string.h>

u64 string_length(const char* str){
    return strlen(str);
}

char* string_duplicate(const char* str){
    u64 length = string_length(str);
    char* copy = tallocate(length + 1, MEMORY_TAG_STRING);
    tcopy_memory(copy, str, length + 1);

    return copy;
}

// Case-sensitive string comparison. True if the same, otherwise false.
b8 strings_equals(const char* str0, const char* str1){
    return strcmp(str0, str1) == 0;
}