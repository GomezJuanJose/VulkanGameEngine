/**
 * @file freelist.h
 * @author Juan José (gs.juanjose.1999@gmail.com)
 * @brief This file contains a free list, used for custom memory allocation tracking.
 * @version 0.1
 * @date 2025-05-19
 * 
 * @copyright Copyright (c) 2025
 */

 #pragma once

 #include "defines.h"

 /**
  * @brief A data structure to be used alongside an allocator for dynamic memory
  * allocation. Tracks free ranges of memory.
  */
 typedef struct freelist {
    /** @brief The internal state of the freelist. */
    void* memory;
 } freelist;

 /**
  * @brief Creates a new freelist or obtains the memory requirement for one. Call
  * twice; once passing 0 to memory to obtain memory requirement, and a second
  * time passing an allocated block to memory.
  * 
  * @param total_size The total size in bytes that the free list should track.
  * @param memory_requirement A pointer to hold memory requirement for the free list itself.
  * @param memory 0, or a pre-allocated block of memory for the free list to use.
  * @param out_list A pointer to hold the created free list.
  */
 TAPI void freelist_create(u64 total_size, u64* memory_requirement, void* memory, freelist* out_list);

 /**
  * @brief Destroys the provided list.
  * 
  * @param list The list to be destroyed.
  */
 TAPI void freelist_destroy(freelist* list);

 /**
  * @brief Attempts to find a free block of memory of the given size.
  * 
  * @param list A pointer to the list to search.
  * @param size The size to allocate.
  * @param out_offset A pointer to hold the offset to the allocated memory.
  * @return b8 True if a block of memroy was found and allocated; otherwise false. 
  */
 TAPI b8 freelist_allocate_block(freelist* list, u64 size, u64* out_offset);

 /**
  * @brief Attempts to free a block of memory at the given offset, and of the given
  * size. Can fail if invalid data is passed.
  * 
  * @param list A pointer to the list to free from.
  * @param size The size to be freed.
  * @param offset The offset to free at.
  * @return b8 True if successful; otherwise false. False should be trated as an error.
  */
 TAPI b8 freelist_free_block(freelist* list, u64 size, u64 offset);

 /**
  * @brief Attempts to resize the provided freelist to the given size. Internal data is copied to the new
  * block of memory. The old block must be freed after this call.
  * NOTE: New size must be _greater_ than the existing size of the given list.
  * NOTE: Should be called twice; once to query the memory requirement (passing new_memory=0),
  * and a second time to actually resize the list.
  * 
  * @param list A pointer to the list to be resized.
  * @param memory_requirement A pointer to hold the amount of memory required for the resize.
  * @param new_memory The new block of state memory. Set to 0 if only querying memory_requirement.
  * @param new_size The new size. Must be greater than the size of the provided list.
  * @param out_old_memory A pointer to hold the old block of memory so that it may be freed after this call.
  * @return True on success; otherwise false. 
  */
 TAPI b8 freelist_resize(freelist* list, u64* memory_requirement, void* new_memory, u64 new_size, void** out_old_memory);


 /**
  * @brief Clears the free list.
  * 
  * @param list The list to be cleared.
  */
 TAPI void freelist_clear(freelist* list);

 /**
  * @brief Returns the amount of free space in this list. NOTE: Since this has
  * to iterate the entire internal list, this can be expensive operation.
  * Use sparingly.
  * 
  * @param list A pointer to the list to obtain from.
  * @return The amount of the free space in bytes.
  */
 TAPI u64 freelist_free_space(freelist* list);