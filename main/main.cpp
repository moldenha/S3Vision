// #define MV_MAX_FRAME_DROP 
// #define ADAFRUIT_MICROSD_BREAKOUT_ 
#include "setup.h"

extern "C" void app_main() {
    ESP_LOGI("DEBUG", "RUNNING MAIN APP");
    if(!mv::config::setup()){
        return;
    }
    mv::config::setup_tasks();
}
