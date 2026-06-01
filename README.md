# S3Vision

This is a library designed for extremely fast captures of video to an SD card, working at almost max FPS on both SPI and SDMMC mode. 
This library is currently only made/tested for the OV3660 on the XIAO ESP32S3 sense board. It is meant to be run with the ESP-IDF compiler.


## Overview

S3Vision is a library designed to enable extremely fast, continuous video recording on the ESP32-S3. It is optimized for MJPEG capture using cameras such as the OV3660, with a focus on maximizing throughput, minimizing latency, and preventing dropped frames.

S3Vision is built to sustain real-time video recording directly to SD storage, even when using SPI mode, which is typically bandwidth-limited.

## Features

-  Real-time MJPEG video capture
-  High FPS performance (~26 FPS at VGA on SPI)
-  Direct-to-SD recording (SPI and SD_MMC supported)
-  Optimized buffering and pipeline design
-  Designed for asynchronous / multi-task pipelines
-  Configurable camera settings (resolution, quality, buffers)

## Design

S3Vision focuses on throughput-first design:

- Minimize blocking operations
- Decouple capture and storage
- Optimize memory movement (PSRAM-aware)
- Reduce SD write overhead
- Avoid pipeline stalls that cause dropped or black frames

The result is a system that stays stable even under sustained load.

## Hardware Requirements

- ESP32-S3 (with PSRAM recommended)
- Camera module (e.g., OV3660)
- SD card (SPI or SD_MMC)

## Core Logic and Setup

### Setup
  1. `configure_sdk.py` 
    - This should be run before starting the program and has 3 options (there is further configuration that can be done in the `main.cpp`:
    - `python3 configure_sdk.py -d` <- This is debug mode, and preps the `sdkconfig` for debugging
    - `python3 configure_sdk.py -c` <- This is console mode, and preps the `sdkconfig` for the user to read outputs from the console
    - `python3 configure_sdk.py -b` <- This is battery mode, and preps the `sdkconfig` for extended and optimal battery life
  2. Configuring `main.cpp` options
    - `MV_SD_MODE` Configures between `SD_SDMMC` and `SD_SPI` (default is `SD_SPI`)
        * To set just add the line before including in the `main.cpp` file: `#define MV_SD_MODE SD_SPI` for SPI mode in this case
    - `MV_SPEED_MODE` Configures between `FAST_MODE` and `VERBOSE_MODE` which basically just switches logging types (default is `FAST_MODE`) 
        * To set just add the line before including in the `main.cpp` file: `#define MV_SPEED_MODE FAST_MODE` for Fast mode in this case 
    - `ADAFRUIT_MICROSD_BREAKOUT_` This my default is off, but it is for projects using the breakout pins for SPI to an SD card writer on a XIAO-ESP32-S3
        * To indicate that you are using this instead of the built-in sense SD card sensor just include `#define ADAFRUIT_MICROSD_BREAKOUT_` in `main.cpp`
    - `MV_MAX_FRAME_DROP` This indicates that when a decoded image from the camera is greater than this number, to drop the frame
        * This can be good for dropping outlier frame/frame sizes that can significantly drop FPS for a short period of time
        * By default, it is not configured. However, if it is used, the recommended size is `20000`
        * Example usage: `#define MV_MAX_FRAME_DROP 20000`
  3. File System
    - The videos will be stored on the SD card under `video/v_n.raw` where `n` represents the video number
    - The initial setup will make the `video` directory, and begin the allocation for storing files
    - The initial setup will open the first video file for writing to
  4. Core Setup
    - There is an initial setup of frame pools for holding frames not written to the SD card
    - There is the initial setup and allocation of the circular buffer
    - There is a setup for automatic light sleeping during idle periods on the CPU
  5. Task Setup
    - 'camera_task`: This is the task to process frames from the camera - highest priority on Core 0
    - `cpy_task`   : This is the task to copy frames off of the camera allocated buffer onto the circular buffer - medium priority on Core 1
    - `sd_task`    : This is the task to take the frames on the circular buffer and write them to the SD card - lowest priority on Core 1

### Custom Allocator
  - There is a custom allocator deemed the 'circular_allocator` that is specifically made for this project and ensures the fastest possible copying of frames
  - The way that it works is that there is a large buffer of PSRAM allocated, the size determined by `MV_RAW_BUF_SIZE * MV_CIRCULAR_BUFFERS` that is 32-bit aligned
  - From there, it is split into `MV_CIRCULAR_BUFFERS` chunks of `MV_RAW_BUF_SIZE` bytes
  - When the `circular_malloc` function is called, it will check if the current chunk has enough space left to accomidate the requested size
    * If it does, it will just return the current buffer at the start of writing point
    * If it does not, the current buffer will be marked as full and needing to be freed, from there the next free buffer will be returned, and a pointer representing the full buffer is returned
  - When `circular_free` is called, it expects a pointer that was given by `circular_malloc` to know which buffer to free 

### File Protocol
  - Uses the normal `write` C command with a file descriptor to save overhead with `FILE` function
  - Pre-stores all memory inside of a pre-alligned DMA RAM allocated buffer before writing to the file to save space and reduce write times


## Installation
- [**Phenomenal guide to helping get started**](https://medium.com/@manjotkhangura/getting-esp32-s3-sense-ov3660-camera-working-a-weekend-deep-dive-941d9c1a05d8) (will help with setup)

- Install [ESP-IDF](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/installation.html) for building and running
    - It is reccomended to use neovim, much easier from the terminal. Can be installed into MSVC though

## Build
- just run `idf.py reconfigure` (after running the `configure_sdk.py` outlined above) and then `idf.py build` and finally `idf.py flash`

## Why S3Vision?

- Most ESP32 camera libraries prioritize ease of use.

- S3Vision on the other hand **prioritizes performance and reliability under load.**

## Understanding S3Vision
 - Under the `main/setup.h` file, is the initial setup (which goes through setting up buffers and taks)
 - Each of the 3 tasks is under `main/camera_task.h`, `main/sd_task.h`, and `main/cpy_task.h` which are the tasks ever running in parallel
 - `main/File.hpp` shows the custom file writing logic
 - `main/circular_malloc.h` shows the custom circular malloc buffer used to enhance speed and reliability

