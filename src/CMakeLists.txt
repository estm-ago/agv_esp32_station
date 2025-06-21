file(GLOB_RECURSE SRC_SRCS "vec.c" "config.c" "fn_state.c" "global_variable.c" "main.c" "${CMAKE_CURRENT_LIST_DIR}/*.c")

idf_component_register(
    SRCS ${SRC_SRCS}
    INCLUDE_DIRS "../include"
    REQUIRES
        nvs_flash
        driver
        esp_wifi
        esp_netif
        esp_http_server
        esp_https_server
        esp_timer
        esp_eth
    EMBED_TXTFILES 
        "connectivity/wifi/https/certs/servercert.pem"
        "connectivity/wifi/https/certs/prvtkey.pem"
)
