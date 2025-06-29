#include <arpa/inet.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_netif.h>
#include <lwip/sockets.h>
#include "main/config.h"
#include "connectivity/wifi/main.h"

#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

static const char *TAG = "user_wifi_main";

static EventGroupHandle_t wifi_event_group;
static int wifi_connect_retry_count = 0;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id,void* event_data)
{
    ESP_LOGI(TAG, "wifi_event_handler");

    // On disconnection, retry or signal failure
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (wifi_connect_retry_count < CONNECT_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            wifi_connect_retry_count++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            // Exceeded max retries, set failure bit
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
    }
    // Obtained IP, reset retry count and set success bit
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connect_retry_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_connect_sta(void)
{
    ESP_LOGI(TAG, "wifi_connect_sta");

    xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    // Configure Wi-Fi connection parameters
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    strcpy((char *)wifi_config.sta.ssid,     WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PSWD);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    // Set Wi-Fi mode to Station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // Set Station configuration
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // Start Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait for connection result: success or fail
    ESP_LOGI(TAG, "Connecting to WIFI_SSID: %s ...", WIFI_SSID);
    esp_wifi_connect();
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE, portMAX_DELAY);
    
    // Connected successfully
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "Connect SUCCESS >> WIFI_SSID: %s / WIFI_PSWD: %s", WIFI_SSID, WIFI_PSWD);
    }
    // Failed to connect
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Connect FAILED  >> WIFI_SSID: %s / WIFI_PSWD: %s", WIFI_SSID, WIFI_PSWD);
    }
}

void wifi_setup_sta(void)
{
    ESP_LOGI(TAG, "wifi_setup_sta");

    wifi_event_group = xEventGroupCreate();
    // Create default Wi-Fi station
    esp_netif_t* sta_netif = esp_netif_create_default_wifi_sta();
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(sta_netif));
    esp_netif_ip_info_t ip_info;
    inet_pton(AF_INET, WIFI_DHCP, &ip_info.ip);
    inet_pton(AF_INET, "255.255.255.0", &ip_info.netmask);
    inet_pton(AF_INET, "192.168.0.1", &ip_info.gw);
    ESP_ERROR_CHECK(esp_netif_set_ip_info(sta_netif, &ip_info));

    // Initialize Wi-Fi driver
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    
    // Register Wi-Fi and IP event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_connect_sta();
}
