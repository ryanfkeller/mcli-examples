idf_component_register(
    SRCS "esp32_wifi_sta_mcli.cpp"
    INCLUDE_DIRS "." "../../libs/mcli/include/mcli" "../../libs/mcli/include/adapters"
    REQUIRES nvs_flash esp_netif esp_wifi lwip
)