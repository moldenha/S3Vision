#ifndef MV_SD_TASK_H__
#define MV_SD_TASK_H__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include "file.hpp"
static mv::File mjpegFile;

extern "C" {

void sd_task(void *pv){ 
    int frame_collection = 0;
    mv::SubFile<0> fa_(&mjpegFile);
    mv::SubFile<1> fb_(&mjpegFile);
    // bool flushable = false;
    while (true) { 
        uint8_t* idx; 
        if (mjpegFile.has_free_buffer() && xQueueReceive(free_queue, &idx, portMAX_DELAY)) {
            if(circular_is_free(idx)){
                continue;    
            }
            // LOGI(TAG, "Allocating on RAM from a circular buffer size of %d", (int)cget_circular_buffer_size(idx));
            mjpegFile.write(get_circular_buffer(idx), cget_circular_buffer_size(idx), /*fix_size=*/true); 
            mjpegFile.force_mark_full();
            circular_free(idx);
        }
        else if(!mjpegFile.has_free_buffer()){
            mjpegFile.write_if_able(); // will write whole/multiple buffers
            ++frame_collection;
        }

        if(frame_collection == 50){
            mjpegFile.flush();
            frame_collection = 0;
        }
    } 
}


}


#endif // MV_SD_TASK_H__ 
