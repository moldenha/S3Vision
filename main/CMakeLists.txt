idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS ""
    PRIV_REQUIRES esp_driver_spi sdmmc esp_driver_gpio fatfs esp32-camera nvs_flash esp_timer
)

