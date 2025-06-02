#pragma once

#include "defines.h"

typedef enum memory_tag{
    MEMORY_TAG_UNKNOWN, // For temporary use. Should be assigned one of the below or have a new tag created.
    MEMORY_TAG_ARRAY,
    MEMORY_TAG_LINEAR_ALLOCATOR,
    MEMORY_TAG_DARRAY,
    MEMORY_TAG_DICT,
    MEMORY_TAG_RING_QUEUE,
    MEMORY_TAG_BST,
    MEMORY_TAG_STRING,
    MEMORY_TAG_APPLICATION,
    MEMORY_TAG_JOB,
    MEMORY_TAG_TEXTURE,
    MEMORY_TAG_MATERIAL_INSTANCE,
    MEMORY_TAG_RENDERER,
    MEMORY_TAG_GAME,
    MEMORY_TAG_TRANSFORM,
    MEMORY_TAG_ENTITY,
    MEMORY_TAG_ENTITY_NODE,
    MEMORY_TAG_SCENE,
    MEMORY_TAG_RESOURCE,
    

    MEMORY_TAG_MAX_TAGS
} memory_tag;

/** @brief The configuration for the memory system. */
typedef struct memory_system_configuration {
    /** @brief The total memory size in bytes used by the internal allocator for this system. */
    u64 total_alloc_size;
} memory_system_configuration;

/**
 * @brief Initializes the memory system.
 * @param config The configuration for this system.
 */
TAPI b8 memory_system_initialize(memory_system_configuration config);

/**
 * @brief Shuts down the memory system.
 */
TAPI void memory_system_shutdown();

TAPI void* tallocate(u64 size, memory_tag tag);

TAPI void tfree(void* block, u64 size, memory_tag tag);

TAPI void* tzero_memory(void* block, u64 size);

TAPI void* tcopy_memory(void* dest, const void* source, u64 size);

TAPI void* tset_memory(void* dest, i32 value, u64 size);

TAPI char* get_memory_usage_str();

TAPI u64 get_memory_alloc_count();