#ifndef MV_SETUP_H__
#define MV_SETUP_H__

// ESP libraries
extern "C" {
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_vfs_fat.h"
#include "esp_camera.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "pins.h"
}
#include "globals.h"

#include "sd.h"
#include "log.h"
#include "cam_setup.h"

// Memory
#include "frame_pool.h"
#include "circular_malloc.h"

// tasks
#include "sd_task.h"
#include "cpy_task.h"
#include "camera_task.h"
// End tasks



namespace mv::config{

MV_INLINE bool setup(){
    ::mv::config::setup_logging();
    gpio_reset_pin(XIAO_LED_RGB_GPIO);
    gpio_set_direction(XIAO_LED_RGB_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(GPIO_NUM_7);
    gpio_reset_pin(GPIO_NUM_8);
    gpio_reset_pin(GPIO_NUM_9);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    
    LOGI(TAG, "Free PSRAM: %d", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    LOGI(TAG, "Free INTERNAL RAM: %d bytes\n",
       heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    LOGI(TAG,"Largest INTERNAL block: %d bytes\n",
       heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    LOGI(TAG,"Free DMA RAM: %d bytes\n",
       heap_caps_get_free_size(MALLOC_CAP_DMA));

    LOGI(TAG,"Largest DMA block: %d bytes\n",
           heap_caps_get_largest_free_block(MALLOC_CAP_DMA));
    // LOGI(TAG, "Initializing SPI bus...");
#if MV_SD_MODE == SD_SDMMC
    ESP_LOGI("DEBUG (A)", "Doing mount SD -> SDMMC (%d)", MV_SD_MODE);
#elif MV_SD_MODE == SD_SPI
    ESP_LOGI("DEBUG (A)", "Doing mount SD -> SPI (%d)", MV_SD_MODE);
#endif // MV_SD_MODE == SD_SPI || SD_MMC 
    sdmmc_card_t* card = ::mv::config::initialize_sd();
    if(!card) return false;
    
    if(!initialize_circular_buffers__()){
        LOGE(TAG, "ERROR: Unable to allocate memory for buffers");
        return false;
    }

    if (::mv::config::init_camera() != ESP_OK) {
        LOGE(TAG, "Camera failed!");
        return false;
    }

    free_queue  = xQueueCreate(FRAME_POOL_SIZE, sizeof(uint8_t*));
    free_frame_queue = xQueueCreate(FRAME_POOL_SIZE, sizeof(uint8_t));
    frame_queue = xQueueCreate(FRAME_POOL_SIZE, sizeof(uint8_t));
    file_mutex = xSemaphoreCreateMutex();


    for(uint8_t i = 0; i < FRAME_POOL_SIZE; ++i){
        idx_pool[i] = i;
        xQueueSend(free_frame_queue, &idx_pool[i], portMAX_DELAY);
    }

    LOGI(TAG, "Initializing file system...");
    mjpegFile = ::mv::File("/sdcard/video/video.raw");
    if(!mjpegFile.validate()){
        LOGE(TAG, "Error: Unable to validate file");
        return false;
    }
    return true;
    
}

MV_INLINE void setup_tasks(){
    LOGI(TAG, "Starting tasks...");

    // Camera (highest priority, core 0)
    xTaskCreatePinnedToCore(
        camera_task, "cam",
        8192, NULL,
        6,        // HIGH priority
        NULL,
        0
    );

    // Copy Memory to buffer (medium priority, core 1)
    xTaskCreatePinnedToCore(
        cpy_task, "cpy",
        8192, NULL,
        5,
        NULL,
        1
    );


    // SD file write task (lowest priority, core 1)
    xTaskCreatePinnedToCore(
        sd_task, "sd",
        6144, NULL,
        2,
        NULL,
        1
    );
 
}

} // mv::config::

#endif // MV_SETUP_H__ 
