#ifndef MV_CIRCULAR_MALLOC_H__
#define MV_CIRCULAR_MALLOC_H__

// The point of this file is to make a circular logging-like malloc
// Basically, there will be a register of multiple different buffers allocated on the PSRAM
// When the camera buffer is done writing -> this will take in a camera buffer and 

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
static uint8_t* raw_buffers[MV_CIRCULAR_BUFFERS];
static uint32_t buffer_sizes[MV_CIRCULAR_BUFFERS];
static uint8_t raw_buffer_idxs[MV_CIRCULAR_BUFFERS];
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
    for(uint8_t i = 0; i < *malloc_internal::cur_idx; ++i){
        if(malloc_internal::buffer_sizes[i] != 0) continue;
        malloc_internal::cur_idx = &malloc_internal::raw_buffer_idxs[i];
        malloc_internal::cur_buffer = malloc_internal::raw_buffers[i];
        malloc_internal::cur_buf_size = &malloc_internal::buffer_sizes[i];
        *malloc_internal::cur_buf_size += bytes;
        return (void*)malloc_internal::cur_buffer;
    }
    for(uint8_t i = *malloc_internal::cur_idx + 1; i < MV_CIRCULAR_BUFFERS; ++i){
        if(malloc_internal::buffer_sizes[i] != 0) continue;
        malloc_internal::cur_idx = &malloc_internal::raw_buffer_idxs[i];
        malloc_internal::cur_buffer = malloc_internal::raw_buffers[i];
        malloc_internal::cur_buf_size = &malloc_internal::buffer_sizes[i];
        *malloc_internal::cur_buf_size += bytes;
        return (void*)malloc_internal::cur_buffer;
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
