// Globals are meant to be more programatic/software definitions that aid in program speed
// these are meant to differ from Pin definitions, or hardware definitions

#ifndef MV_GLOBALS_H__
#define MV_GLOBALS_H__

// Specifically for TAG variable when on FAST_MODE
#pragma GCC diagnostic ignored "-Wunused-variable"

extern "C" {
#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "freertos/queue.h"
#include "esp_timer.h"
}


#ifdef _MSC_VER
    #define MV_ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define MV_ALWAYS_INLINE inline __attribute__((always_inline))
#else
    #define MV_ALWAYS_INLINE inline 
    //otherwise default to just inline
#endif


// No inline should be used specifically when there are functions that will be rarely called (such as error handling in testing)
// The reason for this is to reduce binary sizes, allowing a greater accumulation of static memory
#if defined(__GNUC__) || defined(__clang__)
    #define MV_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
    #define MV_NOINLINE __declspec(noinline)
#else
    #define MV_NOINLINE
#endif

#define MV_INLINE inline

#define MV_MAX_FILE_SIZE 50000 * 30 * 30 // (45 MB, just for inital allocation of file)
#define MV_MAX_PSRAM_FRAME 112 * 1024
#define MV_RAW_BUF_SIZE 112 * 1024 // 112 KB (~ 0.114 MB)
#define MV_CIRCULAR_BUFFERS 25 // allow 25 circular buffers (~2.8 MB)
// In total allocating 2.5 MB (out of 4 MB possible after camera) for circular buffer
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define FAST_MODE 1
#define VERBOSE_MODE 0

#define SD_SDMMC 1
#define SD_SPI 0

#ifndef MV_SD_MODE
#define MV_SD_MODE SD_SPI
#endif
#ifndef MV_SPEED_MODE
#define MV_SPEED_MODE FAST_MODE // or verbose mode
// #define MV_SPEED_MODE VERBOSE_MODE // or fast mode
#endif

#define FRAME_MAGIC 0xDEADBEEF


#ifdef ADAFRUIT_MICROSD_BREAKOUT_
// XIAO ESP32-S3 Pins to use on adafruit breakout micro sd board

#define SPI_PIN_MISO  9
#define SPI_PIN_MOSI  8
#define SPI_PIN_SCLK  7
#define SPI_PIN_CS    (gpio_num_t)44

#else
// ESP32-S3 Sense board pins to use
#define SPI_PIN_CS (gpio_num_t)21 // GPIO21
#define SPI_PIN_SCLK 7 //  GPIO7
#define SPI_PIN_MISO 8 // GPIO8
#define SPI_PIN_MOSI 9 // GPIO10

#endif // ADAFRUIT_MICROSD_BREAKOUT_

#define FRAME_POOL_SIZE 25



extern "C" {
static const char *TAG = "XIAO_CAM";
static uint32_t frame_header = FRAME_MAGIC;
// Creates a queue to send frames to the SD card and create a lack of buffering until the next frame is ready to be saved
//frame_queue -> frame buffers from the camera that are ready to be saved into circular memory, and released back to the camera
//free_queue -> memory that needs to be written to the SD card and freed
QueueHandle_t frame_queue;
QueueHandle_t free_frame_queue;
QueueHandle_t free_queue;
TaskHandle_t sd_task_handle;
// SemaphoreHandle_t file_mutex; // to make sure file isn't written to at the same time



}

#endif // MV_GLOBALS_H__ 


// IMPORTANT CONFIG NOTES:
// - These are notes related to either the globals here, or the sdkconfig's
//
// WiFi:
//  - Obviously WiFi is not used in this project, for that reason we want the absolute minimum amount of power/memory
//      dedicated to WiFi. Currently, the minimum amount of iRAM and memory is dedicated to WiFi. While the memory is not
//      needed for this project, that (very fast) memory being taken up results in cache misses for the critical path, reducing
//      the memory used helps to eliminate that.
//
//  - Current evidence suggests that CONFIG_ESP32_WIFI_ENABLED and CONFIG_ESP_WIFI_ENABLED can't be configured to off, however,
//      as long as wifi isn't turned on it shouldn't matter
//  
//      CONFIG_SOC_WIFI_SUPPORTED=n # <- Saves power by preventing the hardware's WiFi module from initiating
// Logging:
//  - Logging (to a terminal) is a huge performance drop in any application. However, it is useful for debugging.
//
//  - When debugging, you can turn logging on with this define in the main file: #define MV_SPEED_MODE VERBOSE_MODE 
//
//  - When not debugging (like on battery running):
//      - When this is happening you want everything related to debugging to be disabled so that no power or memory is dedicated to it
//      - First, add this at the beggining of the main function: #define MV_SPEED_MODE FAST_MODE 
//      - Then, these are the sdkconfig lines to change:
//          CONFIG_USJ_ENABLE_USB_SERIAL_JTAG=n # <- doing this will save 5-10 mA of constant draining
//          CONFIG_ESP_ROM_CONSOLE_OUTPUT_SECONDARY=n # <- Disabling saves speed (and a tiny fraction of power) by preventing the CPU from processing and pushing secondary UART data to the debug pins
//          CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG=n # <- Saves power by preventing clocks from running during sleep modes pre-created during code running
//          CONFIG_ESP_CONSOLE_SECONDARY_NONE=n # <- Saves iRAM and DRAM preventing cache misses from hotpath (saving speed and power)
//          CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG_ENABLED=n # <- Saves power by allowing APB clock to be turned off during run
//
//          change: # CONFIG_BOOTLOADER_LOG_LEVEL_NONE is not set
//          to: CONFIG_BOOTLOADER_LOG_LEVEL_NONE=y
//
//          and change: CONFIG_BOOTLOADER_LOG_LEVEL_INFO=y
//          to: # CONFIG_BOOTLOADER_LOG_LEVEL_INFO is not set
//
//          CONFIG_BOOTLOADER_LOG_LEVEL=0 # <- make 0
