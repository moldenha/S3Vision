#ifndef MV_CONFIG_H__
#define MV_CONFIG_H__

extern "C"{
#include "driver/gpio.h"
#include "esp_camera.h"
#include "pins.h"

// Working OV3660 config
static camera_config_t camera_config = {
    .pin_pwdn     = -1,
    .pin_reset    = -1,
    .pin_xclk     = XIAO_CAM_PIN_XCLK,
    .pin_sccb_sda = XIAO_CAM_PIN_SIOD,
    .pin_sccb_scl = XIAO_CAM_PIN_SIOC,
    
    .pin_d7       = XIAO_CAM_PIN_D7,
    .pin_d6       = XIAO_CAM_PIN_D6,
    .pin_d5       = XIAO_CAM_PIN_D5,
    .pin_d4       = XIAO_CAM_PIN_D4,
    .pin_d3       = XIAO_CAM_PIN_D3,
    .pin_d2       = XIAO_CAM_PIN_D2,
    .pin_d1       = XIAO_CAM_PIN_D1,
    .pin_d0       = XIAO_CAM_PIN_D0,
    .pin_vsync    = XIAO_CAM_PIN_VSYNC,
    .pin_href     = XIAO_CAM_PIN_HREF,
    .pin_pclk     = XIAO_CAM_PIN_PCLK,
    
    .xclk_freq_hz = 20000000, // increased for higher fps (increasing past this will cause pinkness)
    .ledc_timer   = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    
    .pixel_format = PIXFORMAT_JPEG,     // Try JPEG first
    // .pixel_format = PIXFORMAT_RGB565,
    //WORKING .frame_size   = FRAMESIZE_QVGA,     // 800x600
    //WORKING .frame_size   = FRAMESIZE_HD,     // 800x600
    .frame_size = FRAMESIZE_VGA, // 800x600
    .jpeg_quality = 15,
    .fb_count     = 4,
    .fb_location  = CAMERA_FB_IN_PSRAM,
    .grab_mode    = CAMERA_GRAB_LATEST,  // important
    // .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
    .sccb_i2c_port = 0
};

}

#endif // MV_CONFIG_H__ 
