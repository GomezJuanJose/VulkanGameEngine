#include "ring_queue.h"

#include "core/tmemory.h"
#include "core/logger.h"

b8 ring_queue_create(u32 stride, u32 capacity, void* memory, ring_queue* out_queue){
    if(!out_queue){
        TERROR("ring_queue_create requires a valid pointer to hold the queue.");
        return FALSE;
    }

    out_queue->length = 0;
    out_queue->capacity = capacity;
    out_queue->stride = stride;
    out_queue->head = 0;
    out_queue->tail = -1;
    if(memory){
        out_queue->owns_memory = FALSE;
        out_queue->block = memory;
    }else{
        out_queue->owns_memory = TRUE;
        out_queue->block = tallocate(capacity * stride, MEMORY_TAG_RING_QUEUE);
    }

    return TRUE;
}

void ring_queue_destroy(ring_queue* queue){
    if(queue){
        if(queue->owns_memory){
            tfree(queue->block, queue->capacity * queue->stride, MEMORY_TAG_RING_QUEUE);
        }
        tzero_memory(queue, sizeof(ring_queue));
    }
}

b8 ring_queue_enqueue(ring_queue* queue, void* value){
    if(queue && value){
        if(queue->length == queue->capacity){
            TERROR("ring_queue_enqueue - Attempted to enqueue value in full ring queue: %p", queue);
            return FALSE;
        }

        queue->tail = (queue->tail + 1) % queue->capacity;

        tcopy_memory(queue->block + (queue->tail * queue->stride), value, queue->stride);
        queue->length++;
        return TRUE;
    }

    TERROR("ring_queue_enqueue requires valid pointer to queue and value.");
    return FALSE;
}

b8 ring_queue_dequeue(ring_queue* queue, void* out_value){
    if(queue && out_value){
        if(queue->length == 0){
            TERROR("ring_queue_dequeue - Attempted to dequeue value in empty ring queue: %p", queue);
            return FALSE;
        }

        tcopy_memory(out_value, queue->block + (queue->head * queue->stride), queue->stride);
        queue->head = (queue->head + 1) % queue->capacity;
        queue->length--;
        return TRUE;
    }

    TERROR("ring_queue_dequeue requires valid pointers to queue and out_value.");
    return FALSE;
}

b8 ring_queue_peek(const ring_queue* queue, void* out_value){
    if(queue && out_value){
        if(queue->length == 0){
            TERROR("ring_queue_peek - Attempted to peek value in empty ring queue: %p", queue);
            return FALSE;
        }

        tcopy_memory(out_value, queue->block + (queue->head * queue->stride), queue->stride);
        return TRUE;
    }

    TERROR("ring_queue_peek requires valid pointers to queue and out_value.");
    return FALSE;
}