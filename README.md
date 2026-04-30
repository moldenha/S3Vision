# S3Vision

This is a library designed for extremely fast captures of video to an SD card, working at almost max FPS on both SPI and SDMMC mode. 
This library is currently only made/tested for the OV3660 on the XIAO ESP32S3 sense board. It is meant to be run with the ESP-IDF compiler.


## Overview

S3Vision is a library designed to enable extremely fast, continuous video recording on the ESP32-S3. It is optimized for MJPEG capture using cameras such as the OV3660, with a focus on maximizing throughput, minimizing latency, and preventing dropped frames.

S3Vision is built to sustain real-time video recording directly to SD storage, even when using SPI mode, which is typically bandwidth-limited.

## Features

-  Real-time MJPEG video capture
-  High FPS performance (~25 FPS at VGA on SPI)
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

## Installation
- [**Phenomenal guide creating much of the basis for this**](https://medium.com/@manjotkhangura/getting-esp32-s3-sense-ov3660-camera-working-a-weekend-deep-dive-941d9c1a05d8) (will help with setup)

- Install [ESP-IDF](https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/installation.html) for building and running
    - It is reccomended to use neovim, much easier from the terminal. Can be installed into MSVC though

## Build
- just run `idf.py reconfigure` and then `idf.py build` and finally `idf.py flash`

## Why S3Vision?

- Most ESP32 camera libraries prioritize ease of use.

- S3Vision is a pain in the ass to use **prioritizes performance and reliability under load.**

## Understanding S3Vision
 - Under the `main/setup.h` file, is the initial setup (which goes through setting up buffers and taks)
 - Each of the 3 tasks is under `main/camera_task.h`, `main/sd_task.h`, and `main/cpy_task.h` which are the tasks ever running in parallel
 - `main/File.hpp` shows the custom file writing logic
 - `main/circular_malloc.h` shows the custom circular malloc buffer used to enhance speed and reliability

