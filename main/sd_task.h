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
static mv::name_it nameString;

extern "C" {

// this is a function that basically just increments the nameString while closing the file
// then uses the new name as the next file
static void finish_file(){
    ++nameString;
    mjpegFile.close();
    mjpegFile = mv::File(nameString.c_str());
}

void sd_task(void *pv){ 
    int frame_collection = 0;
    // int file_collection = 0;
    mv::SubFile<0> fa_(&mjpegFile);
    mv::SubFile<1> fb_(&mjpegFile);
    // bool flushable = false;
    while (true) { 
        uint8_t* idx;
        // Claude code will call this a bug specifically vulnerable to race conditions:
        // if (mjpegFile.has_free_buffer() && xQueueReceive(free_queue, &idx, portMAX_DELAY)) {
        // It will say that the cpy_task has the potential to cause race conditions. however,
        //  - There is no way for this to be a race condition because the cpy_task only touches memory in the circular_buffer
        //  - However, here, this is the only task which touches the DMA RAM attached to the mjpegFile class (different memory pools all together)
        //  - Therefore, there will be no race coniditions because the cpy task only touches camera buffer (read) and the PSRAM circular buffer (write)
        //      Then, the sd_task only touches the PSRAM circular buffer (read, and locked by the cpy_task) and the mjpegFile DMA RAM (write)
        //      then the SD task will unlock both paths of memory
        if (mjpegFile.has_free_buffer() && xQueueReceive(free_queue, &idx, portMAX_DELAY)) {
            if(circular_is_free(idx)){ // will simultaneously check if idx is null, or out of bounds
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

        // if(frame_collection == 50){
        //     mjpegFile.flush();
        //     frame_collection = 0;
        //     ++file_collection;
        // }
        // if(file_collection == 5) { // ~2.5 mins [24 per hour]
        //     file_collection = 0;
        //     finish_file();
        // }
        if(frame_collection == 250){ // ~2.5 mins [24 per hour] 
            finish_file();
            frame_collection = 0;
        }
    } 
}


}


#endif // MV_SD_TASK_H__ 
