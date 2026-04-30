// With holds to c-standard (hence h header)
// makes a frame_t struct
// Allocates the frame pool on the stack for faster access
//  and transfer to initial memcpy task
#ifndef MV_FRAME_POOL_H__
#define MV_FRAME_POOL_H__

#include <unistd.h>


extern "C" {

#include "esp_camera.h"

typedef struct {
    camera_fb_t* fb;
    uint64_t timestamp_us;
} frame_t;

static frame_t frame_pool[FRAME_POOL_SIZE];
static uint8_t idx_pool[FRAME_POOL_SIZE];

}

#endif //  MV_FRAME_POOL_H__
