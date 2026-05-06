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

#define MV_SD_MODE SD_SPI
#define MV_SPEED_MODE FAST_MODE // or verbose mode


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
SemaphoreHandle_t file_mutex; // to make sure file isn't written to at the same time



}

#endif // MV_GLOBALS_H__ 

