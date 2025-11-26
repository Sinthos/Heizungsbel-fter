# Main component CMakeLists.txt
# ESP32-C6 Zigbee Fan Switch

idf_component_register(
    SRCS
        "app_main.c"
        "relay.c"
        "zigbee_handler.c"
    INCLUDE_DIRS
        "."
    PRIV_REQUIRES
        nvs_flash
        esp_zigbee_lib
        esp_radio_spinel
)
