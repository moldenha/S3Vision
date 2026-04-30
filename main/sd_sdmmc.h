#ifndef MV_SD_SDMMC_H__
#define MV_SD_SDMMC_H__

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>


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
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "pins.h"

}

#include "globals.h"
#include "log.h"

namespace mv::config::sdmmc {

// Currently only configured for 1-bit mode
// Just have to add the pins to accomodate 4 bit mode

MV_INLINE sdmmc_card_t* mount_sd() {
    sdmmc_host_t host = SDMMC_HOST_DEFAULT(); 
    host.flags = SDMMC_HOST_FLAG_1BIT; 
    host.max_freq_khz = 1000; 
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    slot_config.flags = 0; // CURRENT FLAGS FOR SDMMC Mode 1: 
    slot_config.clk = GPIO_NUM_7; 
    slot_config.cmd = GPIO_NUM_9; 
    slot_config.d0 = GPIO_NUM_8; 
    slot_config.width = 1; 
    // slot_config.clk = GPIO_NUM_36; 
    // slot_config.cmd = GPIO_NUM_35; 
    // slot_config.d0 = GPIO_NUM_37; 
    /// slot_config.width = 1; 
    /// // If using 4-bit mode (recommended if wired) 
    // slot_config.d1 = GPIO_NUM_38; 
    // slot_config.d2 = GPIO_NUM_33; 
    // slot_config.d3 = GPIO_NUM_34; // may be 39? 
    // slot_config.width = 1; 
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP; 
    esp_vfs_fat_sdmmc_mount_config_t mount_config = { 
            .format_if_mount_failed = false, 
            .max_files = 5, 
            .allocation_unit_size = 16 * 1024 
    }; 
    sdmmc_card_t *card; 
    vTaskDelay(pdMS_TO_TICKS(500));
    LOGI(TAG, "Mounting SD card..."); 
    esp_err_t ret = esp_vfs_fat_sdmmc_mount( 
            "/sdcard", 
            &host, 
            &slot_config, 
            &mount_config, 
            &card 
    ); 
    if (ret != ESP_OK) { 
        LOGE(TAG, 
                "Failed to mount SD card: %s", 
                esp_err_to_name(ret)
        ); 
        return NULL; 
    } 
    LOGI(TAG, "SD card mounted successfully!"); 
    // sdmmc_card_print_info(stdout, card); 
    return card;
}

}

#endif // MV_SD_SDMMC_H__ 
