#pragma once

// Unsigned int types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating points types
typedef float f32;
typedef double f64;

// Boolean types
typedef int b32;
typedef _Bool b8;

/** @brief A range, typically of memory */
typedef struct range {
    /** @brief The offset in bytes. */
    u64 offset;
    /** @brief The size in bytes. */
    u64 size;
} range;

// Properly define static assertions
#if defined(__clang__) || defined(__gcc__)
    #define STATIC_ASSERT _Static_assert
#else
    #define STATIC_ASSERT static_assert
#endif

// Ensure all types are of the correct size
STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");

STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 bytes.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");

STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

#define TRUE 1
#define FALSE 0

/**
 * @brief Any id set to this should be considered invalid,
 * and not actually pointing to a real object.
 */
#define INVALID_ID 4294967295U
#define INVALID_ID_U16 65535U
#define INVALID_ID_U8 255U

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define TPLATFORM_WINDOWS 1
    #ifndef _WIN64
        #error "64-bit is required on Windows!"
    #endif
#elif defined(__linux__) || defined(__gnu_linux__)
    // Linux OS
    #define TPLATFORM_LINUX 1
    #if defined(__ANDROID__)
        #define TPLATFORM_ANDROID 1
    #endif
#elif defined (__unix__)
    // Catch anything not caught by the above
    #define TPLATFORM_UNIX 1
#elif defined(_POSIX_VERSION)
    // Posix
    #define TPLATFORM_POSIX 1
#elif __APPLE__
    #define TPLATFORM_APPLE 1
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        // iOS Simulator
        #define TPLATFORM_IOS 1
        #define TPLATFORM_IOS_SIMULATOR 1
    #elif TARGET_OS_IPHONE
        #define TPLATFORM_IOS 1
    #elif TARGET_OS_MAC
        // Other kinds of Mac OS
    #else
        #error "Unknown platform!"
    #endif
#else
    #error "Unknown platform!"
#endif

#ifdef TEXPORT
    // Exports
    #ifdef _MSC_VER
        #define TAPI __declspec(dllexport)
    #else
        #define TAPI __attribute__((visibility("default")))
    #endif
#else
    // Imports
    #ifdef _MSC_VER
        #define TAPI __declspec(dllimport)
    #else
        #define TAPI
    #endif
#endif

#define TCLAMP(value, min, max) (value <= min) ? min : (value >= max) ? max : value;

// Inlining
#ifdef _MSC_VER
    #define TINLINE __forceinline
    #define TNOINLINE __declspec(noinline)
#else
    #define TINLINE static inline
    #define TNOINLINE
#endif

/** @brief Gets the number of bytes from amount of gibibytes (GiB) (1024 * 1024 * 1024) */
#define GIBIBYTES(amount) amount * 1024 * 1024 * 1024
/** @brief Gets the number of bytes from amount of mebibytes (MiB) (1024 * 1024) */
#define MEBIBYTES(amount) amount * 1024 * 1024
/** @brief Gets the number of bytes from amount of kibibytes (KiB) (1024) */
#define KIBIBYTES(amount) amount * 1024

/** @brief Gets the number of bytes from amount of gigabytes (GiB) (1000 * 1000 * 1000) */
#define GIGABYTES(amount) amount * 1000 * 1000 * 1000
/** @brief Gets the number of bytes from amount of megabytes (MiB) (1000 * 1000) */
#define MEGABYTES(amount) amount * 1000 * 1000
/** @brief Gets the number of bytes from amount of kilobytes (KiB) (1000) */
#define KILOBYTES(amount) amount * 1000

TINLINE u64 get_aligned(u64 operand, u64 granularity){
    return ((operand + (granularity - 1)) & ~(granularity - 1));
}

TINLINE range get_aligned_range(offset, size, granularity){
    return (range){get_aligned(offset, granularity), get_aligned(size, granularity)};
}