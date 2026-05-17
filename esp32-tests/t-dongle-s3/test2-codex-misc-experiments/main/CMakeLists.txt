idf_component_register(
    SRCS
        "main.cpp"
        "common_ui.cpp"
        "mode_matrix.cpp"
        "mode_mandelbrot.cpp"
        "mode_scope.cpp"
        "mode_wifi.cpp"
    INCLUDE_DIRS "."
    REQUIRES lvgl esp_timer esp_lcd esp_driver_gpio esp_driver_spi esp_wifi esp_event esp_netif nvs_flash waveshare__esp_lcd_st7735
)
