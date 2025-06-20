#pragma once

#include "vulkan_types.inl"

b8 vulkan_buffer_create(
    vulkan_context* context,
    u64 size,
    VkBufferUsageFlagBits usage,
    u32 memory_property_flags,
    b8 bind_on_create,
    b8 use_freelist,
    vulkan_buffer* out_buffer
);

void vulkan_buffer_destroy(vulkan_context* context, vulkan_buffer* buffer);

b8 vulkan_buffer_resize(
    vulkan_context* context,
    u64 new_size,
    vulkan_buffer* buffer,
    VkQueue queue,
    VkCommandPool pool
);

void vulkan_buffer_bind(vulkan_context* context, vulkan_buffer* buffer, u64 offset);

void* vulkan_buffer_lock_memory(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags);
void vulkan_buffer_unlock_memory(vulkan_context* context, vulkan_buffer* buffer);

/**
 * @brief Allocates space from a vulkan buffer. Provides the offset at which the
 * allocation occurred. This will be required for data copying and freeing.
 * 
 * @param buffer A pointer to the buffer from which to allocate.
 * @param size The size in bytes to be allocated.
 * @param out_offset A pointer to hold the offset in bytes from the beginning of the buffer.
 * @return True on success; otherwise false.
 */
b8 vulkan_buffer_allocate(vulkan_buffer* buffer, u64 size, u64* out_offset);

/**
 * @brief Frees space in the vulkan buffer.
 * 
 * @param buffer A pointer to the buffer to free data from.
 * @param size The size in bytes to be freed.
 * @param offset The offset in bytes from the beginning of the buffer.
 * @return True on success; otherwise false.
 */
b8 vulkan_buffer_free(vulkan_buffer* buffer, u64 size, u64 offset);

void vulkan_buffer_load_data(vulkan_context* context, vulkan_buffer* buffer, u64 offset, u64 size, u32 flags, const void* data);

void vulkan_buffer_copy_to(
    vulkan_context* context,
    VkCommandPool pool,
    VkFence fence,
    VkQueue queue,
    VkBuffer source,
    u64 source_offset,
    VkBuffer dest,
    u64 dest_offset,
    u64 size
);