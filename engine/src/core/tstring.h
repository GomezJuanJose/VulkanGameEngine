#pragma once

#include "defines.h"
#include "math/math_types.h"

// Returns the length of the given string.
TAPI u64 string_length(const char* str);

TAPI char* string_duplicate(const char* str);

// Case-sensitive string comparison. True if the same, otherwise false.
TAPI b8 strings_equal(const char* str0, const char* str1);

// Case-insensitive string comparasion. True if the same, otherwise false.
TAPI b8 strings_equali(const char* str0, const char* str1);

/**
 * @brief Case-sensitive string comparison for a number of characters.
 *
 * @param str0 The first string to be compared.
 * @param str1 The second string to be compared.
 * @param length The maximum number of characters to be compared.
 * @return True if the same, otherwise false.
 */
TAPI b8 strings_nequal(const char* str0, const char* str1, u64 length);

/**
 * @brief Case-insensitive string comparison for a number of characters.
 *
 * @param str0 The first string to be compared.
 * @param str1 The second string to be compared.
 * @param length The maximum number of characters to be compared.
 * @return True if the same, otherwise false.
 */
TAPI b8 strings_nequali(const char* str0, const char* str1, u64 length);

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

/**
 * @brief Empties the provided string by setting the first character to 0.
 * 
 * @param str The string to be emptied.
 * @return A pointer to str.
 */
TAPI char* string_empty(char* str);

TAPI char* string_copy(char* dest, const char* source);

TAPI char* string_ncopy(char* dest, const char* source, i64 length);

TAPI char* string_trim(char* str);

TAPI void string_mid(char* dest, const char* source, i32 start, i32 length);

/**
 * @brief Returns the index of the first occurance of C in str; otherwise -1.
 * 
 * @param str The string to be scanned.
 * @param c The character to search for.
 * @return The index of the first occurance of C; otherwise -1 if not found.
 */
TAPI i32 string_index_of(char* str, char c);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space-delimited. (i.e. "1.0 2.0 3.0 4.0")
 * @param out_vector A pointer to the vector to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_vec4(char* str, vec4* out_vector);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space-delimited. (i.e. "1.0 2.0 3.0")
 * @param out_vector A pointer to the vector to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_vec3(char* str, vec3* out_vector);

/**
 * @brief Attempts to parse a vector from the provided string.
 * 
 * @param str The string to parse from. Should be space-delimited. (i.e. "1.0 2.0")
 * @param out_vector A pointer to the vector to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_vec2(char* str, vec2* out_vector);


/**
 * @brief Attempts to parse a 32-bit floating-point number from the provided string.
 * 
 * @param str The string to parse from. Should *not* be postfixed with 'f'.
 * @param f A pointer to the float to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_f32(char* str, f32* f);

/**
 * @brief Attempts to parse a 64-bit floating-point number from the provided string.
 * 
 * @param str The string to parse from.
 * @param f A pointer to the float to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_f64(char* str, f64* f);

/**
 * @brief Attempts to parse a 8-bit signed interger from the provided string.
 * 
 * @param str The string to parse from.
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_i8(char* str, i8* i);

/**
 * @brief Attempts to parse a 16-bit signed interger from the provided string.
 * 
 * @param str The string to parse from.
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_i16(char* str, i16* i);

/**
 * @brief Attempts to parse a 32-bit signed interger from the provided string.
 * 
 * @param str The string to parse from.
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_i32(char* str, i32* i);

/**
 * @brief Attempts to parse a 64-bit signed interger from the provided string.
 * 
 * @param str The string to parse from.
 * @param i A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_i64(char* str, i64* i);

/**
 * @brief Attempts to parse a 8-bit unsigned interger from the provided string.
 * 
 * @param str The string to parse from.
 * @param u A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_u8(char* str, u8* u);

/**
 * @brief Attempts to parse a 16-bit unsigned interger from the provided string.
 * 
 * @param str The string to parse from.
 * @param u A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_u16(char* str, u16* u);

/**
 * @brief Attempts to parse a 32-bit unsigned interger from the provided string.
 * 
 * @param str The string to parse from.
 * @param u A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_u32(char* str, u32* u);

/**
 * @brief Attempts to parse a 64-bit unsigned interger from the provided string.
 * 
 * @param str The string to parse from.
 * @param u A pointer to the int to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_u64(char* str, u64* u);

/**
 * @brief Attempts to parse a boolean from the provided string.
 * "true" or "1" are considered true; anything else is false.
 * 
 * @param str The string to parse from. "true" or "1" are considered true; anything else is false.
 * @param b A pointer to the boolean to write to.
 * @return True if parsed successfully; otherwise false.
 */
TAPI b8 string_to_bool(char* str, b8* b);

/**
 * @brief Splits the given string by the delimiter provided and stores in the
 * provided darray. Optionally trims each entry. NOTE: A string allocation
 * occurs for each entry, and must be freed by the caller.
 * 
 * @param str The string to be split.
 * @param delimiter The character to split by.
 * @param str_darray A pointer to a darray to char arrays to hold the entries. NOTE: must be a darray.
 * @param trim_entries Trims each entry if true.
 * @param include_empty Indicates if empty entries should be included.
 * @return The number of entries yielded by the split operation.
 */
TAPI u32 string_split(const char* str, char delimiter, char*** str_darray, b8 trim_entries, b8 include_empty);

/**
 * @brief Cleans up string allocations in str_darray, but does not
 * free the darray itself.
 * 
 * @param str_darray The darray to be cleaned up.
 */
TAPI void string_cleanup_split_array(char** str_darray);

/**
 * Appends append to source and returns a new string.
 * @param dest The destination string.
 * @param source The string to be appended to.
 * @param append The string to append to source.
 * @returns A new string containing the concatenation of the two strings.
 */
TAPI void string_append_string(char* dest, const char* source, const char* append);

/**
 * @brief Appends the supplied integer to source and outputs to dest.
 * 
 * @param dest The destination for the string.
 * @param source The string to be appended to.
 * @param i The integer to be appended.
 */
TAPI void string_append_int(char* dest, const char* source, i64 i);

/**
 * @brief Appends the supplied float to source and outputs to dest.
 * 
 * @param dest The destination for the string.
 * @param source The string to be appended to.
 * @param f The float to be appended.
 */
TAPI void string_append_float(char* dest, const char* source, f32 f);

/**
 * @brief Appends the supplied boolean (as either "true" or "false") to source and outputs to dest.
 * 
 * @param dest The destination for the string.
 * @param source The string to be appended to.
 * @param b The boolean to be appended.
 */
TAPI void string_append_bool(char* dest, const char* source, b8 b);

/**
 * @brief Appends the supplied character to source and outputs to dest.
 * 
 * @param dest The destination for the string.
 * @param source The string to be appended to.
 * @param c The character to be appended.
 */
TAPI void string_append_char(char* dest, const char* source, char c);

/**
 * @brief Extracts the directory from a full file path.
 * 
 * @param dest The destination for the path.
 * @param path The full path to extract from.
 */
TAPI void string_directory_from_path(char* dest, const char* path);

/**
 * @brief Extracts the filename (including file extension) from a full file path.
 * 
 * @param dest The destination for the filename.
 * @param path The full path to extract from.
 */
TAPI void string_filename_from_path(char* dest, const char* path);

/**
 * @brief Extracts the filename (excluding file extension) from a full file path.
 * 
 * @param dest The destination for the filename.
 * @param path The full path to extract from.
 */
TAPI void string_filename_no_extension_from_path(char* dest, const char* path);