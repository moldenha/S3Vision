#ifndef MV_CPY_TASK_H__
#define MV_CPY_TASK_H__


#include "globals.h"
#include "circular_malloc.h"

#define STARTING_MALLOC_SIZE (sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t))

extern "C" {

#include "esp_timer.h"
#include "esp_camera.h"

void cpy_task(void *pv){
    while(true){
        uint8_t idx;
        if(xQueueReceive(frame_queue, &idx, portMAX_DELAY)){
            frame_t *buf = &frame_pool[idx]; 
            // LOGI(TAG, "CPY task recieved frame pool %d", (int)idx); // <- slows fps (probably)
            uint32_t size = (uint32_t)buf->fb->len;
            uint8_t* freeing_idx = NULL;
            // The size was checked in the camera_task, so that there won't be an issue with the size being too large
            //  or even close to UINT32_MAX
            uint8_t* ptr = (uint8_t*)circular_malloc(size + STARTING_MALLOC_SIZE, &freeing_idx);
            if(!ptr){
                // LOGW(TAG, "Warning: No pointer available");
                esp_camera_fb_return(buf->fb);
                LOGW(TAG, "Warning: No pointer available");
                xQueueSend(free_frame_queue, &idx, portMAX_DELAY);
                if(freeing_idx){
                    // circular_free(freeing_idx);
                    xQueueSend(free_queue, &freeing_idx, portMAX_DELAY);
                }
                continue;
            }
            // LOGI(TAG, "Starting cpy memcpy..."); // <- slows fps (probably)
#ifdef TIME_MEMCPY  // <- used for debugging at some point
            uint64_t t0 = esp_timer_get_time();
            memcpy(ptr, &frame_header, sizeof(uint32_t));
            memcpy(&ptr[sizeof(uint32_t) + sizeof(uint64_t)], &size, sizeof(uint32_t));
            memcpy(&ptr[sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t)], buf->fb->buf, size);
            uint64_t t1 = (esp_timer_get_time() - t0);
            memcpy(&ptr[sizeof(uint32_t)], &t1, sizeof(uint64_t));
#else
            memcpy(ptr, &frame_header, sizeof(uint32_t));
            memcpy(&ptr[sizeof(uint32_t)], &(buf->timestamp_us), sizeof(uint64_t));
            memcpy(&ptr[sizeof(uint32_t) + sizeof(uint64_t)], &size, sizeof(uint32_t));
            memcpy(&ptr[sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint32_t)], buf->fb->buf, size);
#endif // TIME_MEMCPY 
            // LOGI(TAG, "Ended cpy memcpy"); // <- slows fps (probably)
            esp_camera_fb_return(buf->fb);
            xQueueSend(free_frame_queue, &idx, portMAX_DELAY);
            if(freeing_idx){
                // circular_free(freeing_idx);
                xQueueSend(free_queue, &freeing_idx, portMAX_DELAY);
            }
        }
    }
}
}

#endif // MV_CPY_TASK_H__ 
