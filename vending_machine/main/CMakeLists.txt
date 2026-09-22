idf_component_register(
    SRCS
        "main.cpp"
        "dispenser_task.cpp"
        "mf_rc522.cpp"
        "ration_data.cpp"
        "rfid_task.cpp"
        "tft_ili9341.cpp"
        "touch_xpt2046.cpp"
        "ui_task.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        driver
        esp_lcd
        esp_wifi
        esp_event
        esp_netif
        esp_http_client
        nvs_flash
        esp_timer
)
