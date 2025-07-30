#include <nvs_flash.h>
#include <esp_netif.h>
// #include "lwip/err.h"
// #include "lwip/sys.h"
// #include <lwip/sockets.h>
// #include "lwip/netdb.h"
#include <esp_heap_caps.h>
#include "main/config.h"
#include "connectivity/uart/main.h"
#include "connectivity/fdcan/main.h"
#include "connectivity/wifi/main.h"
#include "connectivity/wifi/https/main.h"
#include "storage/main.h"

static const char *TAG = "user_main";

void memory_monitor_task(void* pvParameters) {
    while (1) {
        // 剩餘堆積 (bytes)
        size_t free_heap       = esp_get_free_heap_size();
        // 自啟動以來的最小剩餘堆積 (bytes)
        size_t min_free_heap   = esp_get_minimum_free_heap_size();
        // 最大連續可分配區块 (bytes)
        size_t largest_block   = heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
        // PSRAM（若有接外部 PSRAM）剩餘
        // size_t free_psram      = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);

        ESP_LOGI(TAG, "Heap free: %u | Min free: %u | Largest: %u",
                 free_heap, min_free_heap, largest_block);
        // ESP_LOGI(TAG, "PSRAM free: %u", free_psram);

        vTaskDelay(pdMS_TO_TICKS(1000));  // 每秒更新一次
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Start");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());

    // uart_setup();
    fdcan_setup();
    storage_setup();

    wifi_setup_sta();
    https_server_setup();

    xTaskCreate(memory_monitor_task, "mem_monitor", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);

    while (1) {
        storage_loop();
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
