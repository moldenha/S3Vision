#ifndef MV_CAMERA_TASK_H__
#define MV_CAMERA_TASK_H__


extern "C" {
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "esp_camera.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "pins.h"
}

#include "globals.h"
#include "log.h"

extern "C" {


#ifdef MV_MAX_FRAME_DROP
// Set like this for VGA mode
static size_t max_len = 20000; // 2000 bytes -> drop frame
#endif // MV_MAX_FRAME_DROP

void camera_task(void *pv) {
    // int frame_count = 0;
    while (true) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) continue;
        
        uint8_t idx;

#ifdef MV_MAX_FRAME_DROP
        if (fb->len >= max_len || xQueueReceive(free_frame_queue, &idx, 0) != pdTRUE) {
            esp_camera_fb_return(fb);
            // LOGE(TAG, "Error: No free buffer to use");
            continue;
        }
#else
        if (xQueueReceive(free_frame_queue, &idx, 0) != pdTRUE) {
            esp_camera_fb_return(fb);
            // LOGE(TAG, "Error: No free buffer to use");
            continue;
        }
#endif // MV_MAX_FRAME_DROP 
        frame_t* buf = &frame_pool[idx];
        buf->fb = fb;
        buf->timestamp_us = esp_timer_get_time();
        xQueueSend(frame_queue, &idx_pool[idx], portMAX_DELAY);
    }
}
}

#endif // MV_CAMERA_TASK_H__ 
