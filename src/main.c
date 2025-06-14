#include <esp_system.h>
#include <esp_log.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// #include <freertos/event_groups.h>
// #include "esp_mac.h"
// #include <esp_wifi.h>
// #include "esp_http_server.h"
// #include <esp_netif.h>
// #include "lwip/err.h"
// #include "lwip/sys.h"
// #include <lwip/sockets.h>
// #include "lwip/netdb.h"
#include "wifi/main.h"
#include "wifi/tcp_transceive.h"
#include "wifi/http/server.h"
#include "wifi/https/server.h"
#include "uart/transceive.h"
#include "uart/packet.h"

static const char *TAG = "user_main";

void app_main(void) {
    ESP_LOGI(TAG, "app_main");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());
    
    wifi_init_sta();
    wifi_connect_sta();

    server_main();
    // wifi_transceive_setup();
    // uart_setup();
    // http_start_server();

    while (1) {
        // ESP_LOGI(TAG, "Running main loop...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
