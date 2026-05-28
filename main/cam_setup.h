#ifndef MV_CAM_SETUP_H__
#define MV_CAM_SETUP_H__


extern "C" {
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "esp_camera.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "pins.h"
}

#include "globals.h"
#include "config.h"

namespace mv::config{

extern "C" {
static esp_err_t init_camera(void) {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) return err;
    sensor_t *s = esp_camera_sensor_get();
    if (!s) return ESP_FAIL;

    // ==== Potential settings to play around with ====

/*
    s->set_brightness(s, 3);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_sharpness(s, 3);
    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_wb_mode(s, 0);
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 0);
    s->set_ae_level(s, 0);
    s->set_aec_value(s, 300);
    s->set_gain_ctrl(s, 1);
    s->set_agc_gain(s, 0);
    s->set_gainceiling(s, (gainceiling_t)0);
    s->set_bpc(s, 0);
    s->set_wpc(s, 1);
    s->set_raw_gma(s, 1);
    s->set_lenc(s, 1);
    s->set_hmirror(s, 0);
    s->set_vflip(s, 0);
    s->set_dcw(s, 1);
    s->set_colorbar(s, 0);

    // Better exposure for dark scenes
    s->set_exposure_ctrl(s, 1);     // Enable auto-exposure
    s->set_aec2(s, 1);              // Enable advanced exposure control
    s->set_ae_level(s, 2);          // Increase exposure level (range: -2 to 2)
    s->set_aec_value(s, 600);       // Longer exposure time (default: 300)
    
    // Better gain for low light
    s->set_gain_ctrl(s, 1);         // Enable auto gain
    s->set_agc_gain(s, 15);         // Increase gain (range: 0-30)
    s->set_gainceiling(s, GAINCEILING_128X);  // Higher gain ceiling
    
    // White balance for better colors
    s->set_whitebal(s, 1);          // Enable auto white balance
    s->set_awb_gain(s, 1);          // Enable AWB gain
    
    // Other improvements
    s->set_lenc(s, 1);              // Enable lens correction
    s->set_raw_gma(s, 1);           // Enable gamma correction
    s->set_dcw(s, 1);               // Enable downsize

*/
    
    // Give sensor a moment to stabilize after init
    vTaskDelay(pdMS_TO_TICKS(100));
    
    s->set_framesize(s, camera_config.frame_size);

    LOGI(TAG, "[*] Camera initialized ");
    return ESP_OK;

    

}}


} // mv::config 

#endif //MV_CAM_SETUP_H__ 

