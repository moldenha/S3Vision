#ifndef MV_SD_H__
#define MV_SD_H__

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
#include "sd_spi.h"
#include "sd_sdmmc.h"

namespace mv::config {

#ifndef MV_SD_MODE
#error "ERROR: SD mode for SPI or SDMMC needs to be pre-defined"
#else
MV_INLINE sdmmc_card_t* initialize_sd() {
#if MV_SD_MODE == SD_MMC
    sdmmc_card_t* card = sdmmc::mount_sd();
#elif MV_SD_MODE == SD_SPI
    sdmmc_card_t* card = spi::mount_sd();
#endif // MV_SD_MODE == SD_SPI || SD_MMC 
    if(card == NULL){ return NULL; }
    int ret_mk = mkdir("/sdcard/video", 0777);   
    if (ret_mk != 0) { 
        LOGW(TAG, "mkdir failed errno=%d [probably already made]", errno); 
    }
    
    return card;
}
#endif // MV_SD_MODE 

}

#endif // MV_SD_H__ 
