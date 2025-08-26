#pragma once

#include "defines.h"

/**
 * A mutex to be used for synchronization purposes. A mutex (or
 * mutual exclusion) is used to limit access to a resource when
 * there are multiple threads of execution around that resource.
 */
typedef struct tmutex {
    void *internal_data;
} tmutex;

/**
 * Creates a mutex.
 * @param out_mutex A pointer to hold the created mutex.
 * @returns True if created successfully; otherwise false.
 */
b8 tmutex_create(tmutex* out_mutex);

/**
 * @brief Destroys the provided mutex.
 * 
 * @param mutex A pointer to the mutex to be destroyed.
 */
void tmutex_destroy(tmutex* mutex);

/**
 * Creates a mutex lock.
 * @param mutex A pointer to the mutex.
 * @returns True if locked successfully; otherwise false.
 */
b8 tmutex_lock(tmutex *mutex);

/**
 * Unlocks the given mutex.
 * @param mutex The mutex to unlock.
 * @returns True if unlocked successfully; otherwise false.
 */
b8 tmutex_unlock(tmutex *mutex);