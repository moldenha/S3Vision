#ifndef MV_CIRCULAR_MALLOC_H__
#define MV_CIRCULAR_MALLOC_H__


// The point of this file is to make a circular logging-like malloc
// Basically, there will be a register of multiple different buffers allocated on the PSRAM
// When the camera buffer is done writing -> this will take in a camera buffer and 
//
// NOTE: This is not to be used in future projects as a self-contained system, this is meant to be used hollistically

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include "globals.h"

extern "C"{
#include "esp_camera.h"
#include "esp_heap_caps.h"
}


namespace malloc_internal{

// The reason these buffers are not marked as volatile:
//  - The buffers themselves never interact with extrenuous hardware
//  - The buffers are also subjected to Core 1 (SD_task and cpy_task)
//      * If this changes, consider making these buffers volatile *

static uint8_t* raw_buffers[MV_CIRCULAR_BUFFERS];
static uint32_t buffer_sizes[MV_CIRCULAR_BUFFERS];
static uint8_t raw_buffer_idxs[MV_CIRCULAR_BUFFERS];
// **IMPORTANT**
// The locks here act as kind of like a pseudo-atomic buffer, 
// After a buffer has been used up, it is locked until free
// This guards against a very specific edge case, where if for example, you fill up Buffer A with a large image,
//      and then you fill up Buffer B with a few more images, and then you get a small image too large for 
//      Buffer B, but barely enough memory left over in Buffer A to be able to be used, but Buffer A has been queued for
//      storing and writing to the SD card in the SD task on core 1, then it could happen where you get a memory corruption
// However, this is also, just irrelevant and would never be used, and here is why:
//      The locking mechanism would only be used in the allocation and the freeing function
//      However, the face that the buffer_sizes[*Buffer A] would already be greater than 0
//      And, in the for loop, you have:
//          if(malloc_internal::buffer_sizes[i] != 0) continue;
//      Making it so that the buffer won't be returned to begin with
//      With the locks here is how that would change:
//          if(malloc_internal::locks[i] || malloc_internal::buffer_sizes[i] != 0) continue;
//      Further, the freeing function would change to:
//          malloc_internal::buffer_sizes[*idx] = 0;
//          malloc_internal::locks[*idx] = false;
//      Making the locks buffer below a redundency not needed that would never be used
//          
// static bool locks[MV_CIRCULAR_BUFFERS];


uint8_t* cur_idx;
uint32_t* cur_buf_size;
uint8_t* cur_buffer;
} // malloc_internal::

// Only meant to be called once as such:
/*
if(!initialize_circular_buffers__()){
    ESP_LOGE(TAG, "ERROR: Unable to allocate memory for buffers");
    return;
}
 */
MV_INLINE bool initialize_circular_buffers__(){
    malloc_internal::raw_buffers[0] = 
            (uint8_t*) heap_caps_aligned_alloc(
                    32,
                    MV_RAW_BUF_SIZE * MV_CIRCULAR_BUFFERS, 
                    MALLOC_CAP_SPIRAM
    );
    if(!malloc_internal::raw_buffers[0])
        return false;

    malloc_internal::buffer_sizes[0] = 0;
    malloc_internal::raw_buffer_idxs[0] = 0;
    malloc_internal::cur_idx = 
        &malloc_internal::raw_buffer_idxs[0];
    malloc_internal::cur_buf_size = 
        &malloc_internal::buffer_sizes[0];
    malloc_internal::cur_buffer =
        malloc_internal::raw_buffers[0];

    for(uint32_t i = 1; i < MV_CIRCULAR_BUFFERS; ++i){
        malloc_internal::raw_buffers[i] = 
            &(malloc_internal::raw_buffers[0][MV_RAW_BUF_SIZE * i]); 
        malloc_internal::buffer_sizes[i] = 0;
        malloc_internal::raw_buffer_idxs[i] = i;
        // malloc_internal::locks[i] = false;
    }
    return true;
}

// Returns a pointer to the memory
// Also gives an idx that needs to be potentially freed
MV_ALWAYS_INLINE void* circular_malloc(uint32_t bytes, uint8_t** idx){
    if((*malloc_internal::cur_buf_size + bytes) < MV_RAW_BUF_SIZE){
        void* returning = (void*)&malloc_internal::cur_buffer[*malloc_internal::cur_buf_size]; 
        *malloc_internal::cur_buf_size += bytes;
        *idx = NULL;
        return returning;
    }
    *idx = malloc_internal::cur_idx;
    // malloc_internal::locks[*idx] = true
    for(uint8_t i = 0; i < *malloc_internal::cur_idx; ++i){
        if(malloc_internal::buffer_sizes[i] != 0) continue;
        malloc_internal::cur_idx = &malloc_internal::raw_buffer_idxs[i];
        malloc_internal::cur_buffer = malloc_internal::raw_buffers[i];
        malloc_internal::cur_buf_size = &malloc_internal::buffer_sizes[i];
        *malloc_internal::cur_buf_size += bytes; // <- used for marking the current size
        
        // Claude code will say that this is a potential bug because I am just returning the beggining of the buffer,
        // Here is why claude is wrong:
        //  - This is only run on one task, meaning I do not have to worry about mutex's, or the changing of sizes
        //      When returning
        //  - Secondly, the if statement at the top, has a continue on it if the buffer size is not 0
        //      Therefore, I already know the beggining of the buffer is empty, and should be returned as such
        //      There is no reason not to just return this
        return malloc_internal::cur_buffer;
    }
    // Claude code will call this a bug: for(uint8_t i = *malloc_internal::cur_idx + 1; i < MV_CIRCULAR_BUFFERS; ++i){
    // However, if the *malloc_internal::cur_idx index actually changes at any point, this function will stop due to a returning statement
    // It would be a waste of performance, and memory to store a local index, while minimal performance hit, if any, calls for cognitive disidance
    for(uint8_t i = *malloc_internal::cur_idx + 1; i < MV_CIRCULAR_BUFFERS; ++i){
        if(malloc_internal::buffer_sizes[i] != 0) continue;
        malloc_internal::cur_idx = &malloc_internal::raw_buffer_idxs[i];
        malloc_internal::cur_buffer = malloc_internal::raw_buffers[i];
        malloc_internal::cur_buf_size = &malloc_internal::buffer_sizes[i];
        *malloc_internal::cur_buf_size += bytes;
        // Claude code will say that this is a potential bug because I am just returning the beggining of the buffer,
        // Here is why claude is wrong:
        //  - This is only run on one task, meaning I do not have to worry about mutex's, or the changing of sizes
        //      When returning
        //  - Secondly, the if statement at the top, has a continue on it if the buffer size is not 0
        //      Therefore, I already know the beggining of the buffer is empty, and should be returned as such
        //      There is no reason not to just return this
        return malloc_internal::cur_buffer;
    }
    return NULL; // No buffers available
}

MV_ALWAYS_INLINE uint8_t* get_circular_buffer(uint8_t* idx){
    return (*idx < MV_CIRCULAR_BUFFERS ? malloc_internal::raw_buffers[*idx] : NULL);
}

MV_ALWAYS_INLINE uint32_t cget_circular_buffer_size(uint8_t* idx){
    return (*idx < MV_CIRCULAR_BUFFERS ? malloc_internal::buffer_sizes[*idx] : 0); 
}

MV_ALWAYS_INLINE bool circular_is_free(uint8_t* idx){
    return malloc_internal::buffer_sizes[*idx] == 0;
}

MV_ALWAYS_INLINE void circular_free(uint8_t* idx){
    malloc_internal::buffer_sizes[*idx] = 0;
}


#endif // MV_CIRCULAR_MALLOC_H__ 
