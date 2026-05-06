/*
 * The below code is not really concerned with life time
 * This is meant to be a power on and off type of thing,
 *  For that reason freeing memory is not handled properly and how it normally would be
 *  Not good C practice normally
 */

#ifndef MV_FILE_HPP__
#define MV_FILE_HPP__
// #define MV_FORCE_BYTE_COPY 

#define WRITE_MODE_FAST 1 // uses the write function instead of fwrite -> bypasses internal buffer, makes a faster write
#define WRITE_MODE_SAFE 0 // uses the fwrite command and a dedicated FILE* descriptor, slow, don't use
#define MV_WRITE_MODE WRITE_MODE_FAST

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <unistd.h>
#include "globals.h"
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

extern "C"{
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "freertos/queue.h"
}

#define SD_WRITE_CHUNK_SIZE (112 * 1024)
#define WRITE_BUFFER_SIZE (SD_WRITE_CHUNK_SIZE * 2)
#define ALIGN 4096

namespace mv{

template<size_t I>
class SubFile;

class File{
    uint8_t* buffer_; // 512 * 1024 * 2 <- SD card has 512 sized sectors
#if MV_WRITE_MODE == WRITE_MODE_SAFE
    FILE* file_;
#endif
    int fd_;
    uint8_t idx;
    uint32_t idxa_len;
    uint32_t idxb_len;
    bool idxa_full;
    bool idxb_full;
    template<size_t I>
    friend class SubFile;
    // SemaphoreHandle_t buffera_mutex;
    // SemaphoreHandle_t bufferb_mutex;
    MV_ALWAYS_INLINE uint8_t* get_buffer() noexcept { return idx == 0 ? buffer_ : &buffer_[SD_WRITE_CHUNK_SIZE]; }
    MV_ALWAYS_INLINE const uint8_t* get_buffer() const noexcept { return idx == 0 ? buffer_ : &buffer_[SD_WRITE_CHUNK_SIZE]; }  
    MV_ALWAYS_INLINE uint32_t& get_len() noexcept { return idx == 0 ? idxa_len : idxb_len; } // length
    MV_ALWAYS_INLINE const uint32_t& get_len() const noexcept { return idx == 0 ? idxa_len : idxb_len; } // length
    MV_ALWAYS_INLINE void switch_buffer() noexcept { idx = (idx == 0 ? 1 : 0); }
    MV_ALWAYS_INLINE void mark_full() noexcept { 
        if(idx == 0) idxa_full = true;
        else idxb_full = true;
    }

    inline void memcpy__(void* ram_dst, const void* src_void, uint32_t size) noexcept {
        // memcpy doesn't work from PSRAM -> RAM so have to manually implement different versions:
        if (esp_ptr_external_ram(src_void)) {
            // PSRAM
            // //ESP_LOGI("MEMCPY SEGFAULT", "PSRAM Detected");
            uint8_t* dst = (uint8_t*)ram_dst;
            const uint8_t* src = (const uint8_t*)src_void;
#ifdef MV_FORCE_BYTE_COPY
            //ESP_LOGI("MEMCPY SEGFAULT", "Doing full byte copy");
            // Fallback: byte copy (always safe)
            //ESP_LOGI("MEMCPY SEGFAULT", "src[16000] = %d", (int)src[16000]);
            //ESP_LOGI("MEMCPY SEGFAULT", "dst[16000] = %d", (int)dst[16000]);
            for (uint32_t i = 0; i < size; i++) {
                // //ESP_LOGI("MEMCPY SEGFAULT", "Writing from %d of %d", (int)i, (int)size);
                dst[i] = src[i];
            }
#else
            // Require 32-byte alignment for DMA safety
            if ((((uintptr_t)dst | (uintptr_t)src | size) & 31) == 0) {
                //ESP_LOGI("MEMCPY SEGFAULT", "Doing aligned copy");
                uint32_t* d32 = (uint32_t*)dst;
                const uint32_t* s32 = (const uint32_t*)src;

                for (uint32_t i = 0; i < size / 4; i++) {
                    d32[i] = s32[i];
                }
            } else {
                //ESP_LOGI("MEMCPY SEGFAULT", "Doing full byte copy");
                // Fallback: byte copy (always safe)
                for (uint32_t i = 0; i < size; i++) {
                    dst[i] = src[i];
                }
            }
#endif
        }else{
            //ESP_LOGI("MEMCPY SEGFAULT", "Normal RAM Detected");
            memcpy(ram_dst, src_void, size);
        }
    }

    // inline SemaphoreHandle_t& get_mutex() noexcept { return idx == 0 ? buffera_mutex : bufferb_mutex; }
    inline bool store__(const void* ptr, uint32_t size) noexcept {
        if((this->get_len() + size) <= SD_WRITE_CHUNK_SIZE){
            // SemaphoreHandle_t& mutex = this->get_mutex();
            // xSemaphoreTake(mutex, portMAX_DELAY);
            assert(this->get_len() < SD_WRITE_CHUNK_SIZE);
            assert(this->get_buffer() != NULL);
            assert(ptr != NULL);
            //ESP_LOGI("MEMCPY SEGFAULT", "INFO: current length: %d, size: %d, ptr: %p", (int)this->get_len(), (int)size, ptr);
            this->memcpy__(this->get_buffer() + this->get_len(), ptr, size);
            //ESP_LOGI("MEMCPY SEGFAULT", "INFO: Finished memcpy!");
            this->get_len() += size;
            // xSemaphoreGive(mutex);
            return true;
        }
        this->mark_full();
        this->switch_buffer();
        if((this->get_len() + size) >= SD_WRITE_CHUNK_SIZE)
            return false;
        // SemaphoreHandle_t& mutex = this->get_mutex();
        // xSemaphoreTake(mutex, portMAX_DELAY);
        assert(this->get_len() < SD_WRITE_CHUNK_SIZE);
        assert(this->get_buffer() != NULL);
        //ESP_LOGI("MEMCPY SEGFAULT", "INFO: current length: %d size: %d <switched>", (int)this->get_len(), (int)size);
        this->memcpy__(this->get_buffer() + this->get_len(), ptr, size);
        this->get_len() += size;
        // xSemaphoreGive(mutex);
        return true;
    }
    // think the byte can act as a header such as a 0xFF
    inline bool store_byte__(uint8_t byte, void* ptr, uint32_t size) noexcept {
        if((this->get_len() + size + 1) < SD_WRITE_CHUNK_SIZE){
            // SemaphoreHandle_t& mutex = this->get_mutex();
            // xSemaphoreTake(mutex, portMAX_DELAY);

            uint8_t* start = &this->get_buffer()[this->get_len()];
            start[0] = byte;
            start += 1;
            this->memcpy__(start, ptr, size);
            this->get_len() += (size + 1);
            // xSemaphoreGive(mutex);
            return true;
        }
        this->mark_full();
        this->switch_buffer();
        if((this->get_len() + size + 1) >= SD_WRITE_CHUNK_SIZE)
            return false;
        // SemaphoreHandle_t& mutex = this->get_mutex();
        // xSemaphoreTake(mutex, portMAX_DELAY);
        uint8_t* start = &this->get_buffer()[this->get_len()];
        start[0] = byte;
        start += 1;
        this->memcpy__(start, ptr, size);
        this->get_len() += (size + 1);
        // xSemaphoreGive(mutex);
        return true;
    }
    // NOTE: Most of the time the actual write size will just be SD_WRITE_CHUNK_SIZE
    inline void write_idxa__() noexcept {
        if(!idxb_full) this->idx = 1;
        // xSemaphoreTake(buffera_mutex, portMAX_DELAY);
        size_t aligned = idxa_len & ~(ALIGN - 1);
        aligned = aligned == idxa_len ? aligned : aligned + ALIGN; // so tail end of data not cut off
#if MV_WRITE_MODE == WRITE_MODE_SAFE
        fwrite(this->buffer_, 1, aligned, file_); // always aligned
#elif MV_WRITE_MODE == WRITE_MODE_FAST
        ::write(fd_, buffer_, aligned); // always aligned
#endif
        this->idxa_len = 0;
        this->idxa_full = false;
        // xSemaphoreGive(buffera_mutex, portMAX_DELAY);
    }

    inline void write_idxb__() noexcept {
        if(!idxa_full) this->idx = 0;
        // xSemaphoreTake(bufferb_mutex, portMAX_DELAY);
        size_t aligned = idxb_len & ~(ALIGN - 1);
        aligned = aligned == idxb_len ? aligned : aligned + ALIGN; // so tail end of data not cut off
#if MV_WRITE_MODE == WRITE_MODE_SAFE
        fwrite(&this->buffer_[SD_WRITE_CHUNK_SIZE], 1, aligned, file_); // always aligned
#elif MV_WRITE_MODE == WRITE_MODE_FAST
        ::write(fd_, &this->buffer_[SD_WRITE_CHUNK_SIZE], aligned); // always aligned
#endif
        this->idxb_len = 0;
        this->idxb_full = false;
        // xSemaphoreGive(bufferb_mutex, portMAX_DELAY);
    }
    
    // Claude Code will call this alignment a bug, however, that is not true and here is why:
    //  - Claude code will want both buffer a and buffer b aligned, and then have them added
    //  - If that were to be done, this would mean that not all the memory written to the file would get to the SD card
    //  - This would lead to a corruption of data, and a dropping of files, the DMA buffer is setup like this:
    //      [         TOTAL DMA BUFFER           ]
    //      [    BUFFER A     |      BUFFER B    ]
    //      Where, in this function that all of buffer A should be automatically written (which is already aligned)
    //      From there, the alignment of Buffer B should be taken into account so that it is aligned, making the entire write aligned
    inline void write_both__() noexcept {
        size_t aligned = idxb_len & ~(ALIGN - 1);
        aligned = aligned == idxb_len ? aligned : aligned + ALIGN; // so tail end of data not cut off
#if MV_WRITE_MODE == WRITE_MODE_SAFE
        fwrite(this->buffer_, 1, SD_WRITE_CHUNK_SIZE + aligned, file_); // always aligned
#elif MV_WRITE_MODE == WRITE_MODE_FAST
        ::write(fd_, this->buffer_, SD_WRITE_CHUNK_SIZE + aligned); // always aligned
#endif
        this->idxb_len = 0;
        this->idxa_len = 0;
        this->idxb_full = false;
        this->idxa_full = false;
    }

    inline void write__(bool force = false) noexcept {
        if(force)
            this->write_both__();
        else if(this->idxa_full && this->idxb_full)
            this->write_both__();
        else if(this->idxa_full)
            this->write_idxa__();
        else if(this->idxb_full)
            this->write_idxb__();
    }

    inline void flush__() noexcept {
#if MV_WRITE_MODE == WRITE_MODE_SAFE
        fflush(this->file_);
        fsync(this->fd_);
#elif MV_WRITE_MODE == WRITE_MODE_FAST
        fsync(this->fd_);
#endif
    }

    public:
        File()
            :buffer_(NULL), 
#if MV_WRITE_MODE == WRITE_MODE_SAFE
            file_(NULL),
#endif
            fd_(0), idx(0), idxa_len(0), 
            idxb_len(0), idxa_full(false), 
            idxb_full(false)
        {}

        File(const char* fname)
        :buffer_(NULL), 
#if MV_WRITE_MODE == WRITE_MODE_SAFE
            file_(NULL), 
#endif
            fd_(0), idx(0), idxa_len(0), idxb_len(0), 
            idxa_full(false), idxb_full(false)
            // ,buffera_mutex(xSemaphoreCreateMutex()), bufferb_mutex(xSemaphoreCreateMutex()) 
        {
#if MV_WRITE_MODE == WRITE_MODE_SAFE
            file_ = fopen(fname, "wb");
            if(!file_)
                return;
            fd_ = fileno(file_);
#elif MV_WRITE_MODE == WRITE_MODE_FAST
            fd_ = open(fname, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd_ < 0) {
                return;
            }
#endif
            // fast RAM
            buffer_ = (uint8_t*) heap_caps_aligned_alloc(
                        32,
                        WRITE_BUFFER_SIZE, 
                        MALLOC_CAP_INTERNAL | MALLOC_CAP_DMA
                    );
            if(!buffer_){
                //ESP_LOGE("XIAO_CAM", "ERROR: Buffer not allocated");
            }

            // fseek(file_, MV_MAX_FILE_SIZE - 1, SEEK_SET);
            // fputc(0, file_);
            // fseek(file_, 0, SEEK_SET);

            memset(buffer_, 0, WRITE_BUFFER_SIZE); // fills buffer with 0
        }

        ~File(){
            // Not implemented on purpose, up to the user to manually free if needed using the close() function
            // if(buffer_){
            //     heap_caps_free(buffer_);
            //     buffer_ = NULL;
            // }
            // if(file_){
            //     fclose(file_);
            //     file_ = NULL;
            // }
            // fd_ = 0;
        }
        
        MV_ALWAYS_INLINE void close() noexcept {
            this->flush__();
            if(buffer_){
                heap_caps_free(buffer_);
                buffer_ = NULL;
            }
#if MV_WRITE_MODE == WRITE_MODE_SAFE
            if(file_){
                fclose(file_);
                file_ = NULL;
                fd_ = 0;
            }
#elif MV_WRITE_MODE == WRITE_MODE_FAST
            if(fd_ != 0){
                ::close(fd_);
                fd_ = 0;
            }
#endif
        }

#if MV_WRITE_MODE == WRITE_MODE_SAFE
        MV_ALWAYS_INLINE bool validate() noexcept {
            if(buffer_ == NULL) return false;
            if(file_ == NULL) return false;
            fd_ = fileno(file_);
            return true;
        }
#elif MV_WRITE_MODE == WRITE_MODE_FAST
        MV_ALWAYS_INLINE bool validate() noexcept {
            if(buffer_ == NULL) return false;
            if(fd_ < 0) return false;
            return true;
        }
#endif
        MV_ALWAYS_INLINE bool write(void* ptr, uint32_t size, bool fix_size = false, bool file_write_full = false){
            if(fix_size){
                // this basically makes (size % 4 == 0) so that alignment can be used for fast copy
                uint32_t aligned = size & ~(4 - 1);
                size = aligned == size ? size : aligned + 4;
            }
            uint8_t store = this->idx;
            bool out = this->store__(ptr, size);
            if(file_write_full && (store != this->idx)){
                this->write__();
            }
            return out;
        }
        MV_ALWAYS_INLINE bool write(uint8_t byte_header, void* ptr, uint32_t size, bool fix_size = false, bool file_write_full = false){
            if(fix_size){
                // this basically makes (size % 4 == 0) so that alignment can be used for fast copy
                uint32_t aligned = size & ~(4 - 1);
                size = aligned == size ? size : aligned + 4;
            }
            uint8_t store = this->idx;
            bool out = this->store_byte__(byte_header, ptr, size);
            if(file_write_full && (store != this->idx)){
                this->write__();
                // if the idx turns to full, then write
            }
            return out;
        }

        MV_ALWAYS_INLINE bool write_able() const noexcept { return this->idxa_full || this->idxb_full; }
        MV_ALWAYS_INLINE void write_if_able() {
            if(!this->write_able()) return;
            this->write__();
        }
        MV_ALWAYS_INLINE void force_write() noexcept {
            this->write__(true);
        }
        MV_ALWAYS_INLINE void flush(){
            this->flush__();
        }
        MV_ALWAYS_INLINE void force_mark_full() noexcept { this->mark_full(); }
        MV_ALWAYS_INLINE void write_cur() noexcept {
            if(idx == 0) this->write_idxa__();
            else this->write_idxb__();
        }
        MV_ALWAYS_INLINE bool has_free_buffer() noexcept { return !(this->idxa_full && this->idxb_full); }
};


template<size_t I>
class SubFile {
    static_assert(I == 0 || I == 1, "Error: SubFile supports 1 or 0");
    File* file_;
    MV_ALWAYS_INLINE uint8_t* get_buffer() noexcept { 
        if constexpr (I == 0){
            return file_->buffer_;
        }else{
            return &file_->buffer_[SD_WRITE_CHUNK_SIZE];
        }
    }
    MV_ALWAYS_INLINE const uint8_t* get_buffer() const noexcept { 
        if constexpr (I == 0){
            return file_->buffer_;
        }else{
            return &file_->buffer_[SD_WRITE_CHUNK_SIZE];
        }
    }  
    MV_ALWAYS_INLINE uint32_t& get_len() noexcept { 
        if constexpr (I == 0){
            return file_->idxa_len;
        }else{
            return file_->idxb_len;
        }
    } // length
    MV_ALWAYS_INLINE const uint32_t& get_len() const noexcept { 
        if constexpr (I == 0){
            return file_->idxa_len;
        }else{
            return file_->idxb_len;
        }
    } // length
    MV_ALWAYS_INLINE void mark_full() noexcept {
        if constexpr (I == 0){
            file_->idxa_full = true;
        }else{
            file_->idxb_full = true;
        }
    }
    MV_ALWAYS_INLINE const bool& is_full() const noexcept {
        if constexpr (I == 0){
            return file_->idxa_full;
        }else{
            return file_->idxb_full;
        }
    }
    MV_ALWAYS_INLINE void write__(bool force = false) noexcept {
        if(this->is_full() || force){
            if constexpr (I == 0){
                this->file_->write_idxa__();
            }else{
                this->file_->write_idxb__();
            }
        }
    }
    MV_ALWAYS_INLINE void flush__() noexcept {
        this->file_->flush__();
    }



    public:
        SubFile() = delete;
        SubFile(File& file)
            :file_(&file)
        {}
        SubFile(File* file)
            :file_(file)
        {}
        
        MV_ALWAYS_INLINE bool write_able() const noexcept {
            if constexpr (I == 0){
                return this->file_->idxa_full;
            }else{
                return this->file_->idxb_full;
            }
        }
        MV_ALWAYS_INLINE void write_if_able() {
            if(!this->write_able()) return;
            this->write__();
        }
        MV_ALWAYS_INLINE void force_write() noexcept {
            this->write__(true);
        }
        MV_ALWAYS_INLINE void flush(){
            this->flush__();
        }
        MV_ALWAYS_INLINE void force_mark_full() noexcept { this->mark_full(); }

};

} // mv::

#endif // MV_FILE_HPP__ 
