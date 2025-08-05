idf_component_register(
    SRCS "esp32_uart_mcli.cpp"
    INCLUDE_DIRS "." "../../libs/mcli/include/mcli" "../../libs/mcli/include/adapters"
    REQUIRES esp_driver_uart esp_driver_gpio
)