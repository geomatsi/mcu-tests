idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES lvgl esp_timer esp_lcd esp_driver_gpio esp_driver_i2c espressif__esp_lcd_st7796 espressif__esp_lcd_touch_ft5x06
)
