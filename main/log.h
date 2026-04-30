#ifndef MV_LOG_H__
#define MV_LOG_H__

#include "globals.h"
extern "C" {
#include "esp_log.h"
}

#ifndef MV_SPEED_MODE
#error "Error: MV Speed Mode should be pre-defind to account for loging and verbosity"
#else

#if MV_SPEED_MODE == FAST_MODE
    #define LOGI(...)
    #define LOGW(...)
    #define LOGE(...)

namespace mv::config{

MV_ALWAYS_INLINE void setup_logging() {
    esp_log_level_set("*", ESP_LOG_NONE); // turning off verbosity
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}
} // mv::config::

#else
    #define LOGI ESP_LOGI
    #define LOGW ESP_LOGW
    #define LOGE ESP_LOGE

namespace mv::config{
MV_INLINE void setup_logging() {;}
} // mv::config::

#endif // MV_SPEED_MODE == ? 

#endif // MV_SPEED_MODE 



#endif // MV_LOG_H__ 
