#ifndef MV_SD_SPI_H__
#define MV_SD_SPI_H__

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

namespace mv::config::spi {

MV_INLINE sdmmc_card_t* mount_sd() {
    LOGI(TAG, "SPI SD Defined...");
    spi_bus_config_t bus_cfg = {};
    bus_cfg.mosi_io_num = SPI_PIN_MOSI;
    bus_cfg.miso_io_num = SPI_PIN_MISO;
    bus_cfg.sclk_io_num = SPI_PIN_SCLK;
    bus_cfg.quadwp_io_num = -1;
    bus_cfg.quadhd_io_num = -1;

    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        LOGE(TAG, "Failed to init SPI bus");
        return NULL;
    }
    
    LOGI("SPI DEBUG", "Using Pins MOSI: %d, MISO: %d, SCLK: %d, CS: %d", SPI_PIN_MOSI, SPI_PIN_MISO, SPI_PIN_SCLK, (int)SPI_PIN_CS);
    LOGI(TAG, "Mounting SD card...");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI2_HOST;
    // host.max_freq_khz = 2000; 

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SPI_PIN_CS;
    slot_config.host_id = SPI2_HOST;

    
    esp_vfs_fat_sdmmc_mount_config_t mount_config;
    mount_config.format_if_mount_failed = false;
    mount_config.max_files = 5;
    mount_config.allocation_unit_size = 16 * 1024;

    sdmmc_card_t *card;
    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        LOGE(TAG, "Failed to mount filesystem");
        return NULL;
    }

    LOGI(TAG, "SD card mounted");
    return card;
}

}

#endif // MV_SD_SPI_H__ 
