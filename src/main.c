#include <esp_system.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// #include <freertos/event_groups.h>
// #include "esp_mac.h"
// #include <esp_wifi.h>
// #include "esp_http_server.h"
// #include "lwip/err.h"
// #include "lwip/sys.h"
// #include <lwip/sockets.h>
// #include "lwip/netdb.h"
#include "connectivity/wifi/main.h"
#include "connectivity/wifi/https/main.h"
#include "connectivity/uart/main.h"

static const char *TAG = "user_main";

void app_main(void) {
    ESP_LOGI(TAG, "app_main");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());
    
    uart_setup();
    wifi_init_sta();
    
    wifi_connect_sta();
    // wifi_https_main();
    https_server_start();

    while (1) {
        // ESP_LOGI(TAG, "Running main loop...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
