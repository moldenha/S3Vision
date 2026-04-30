#include "setup.h"

extern "C" void app_main() {
    if(!mv::config::setup()){
        return;
    }
    mv::config::setup_tasks();
}
