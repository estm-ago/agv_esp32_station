#include "wifi/http/server.h"
#include "wifi/http/base.h"
#include <esp_log.h>
#include <string.h>

static const char *TAG = "http_server";

httpd_handle_t http_start_server(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }

    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &ws_uri);
    httpd_register_uri_handler(server, &hello_get_uri);
    httpd_register_uri_handler(server, &hello_post_uri);
    httpd_register_uri_handler(server, &echo_uri);

    ESP_LOGI(TAG, "Web Server Started");
    return server;
}

esp_err_t http_stop_server(httpd_handle_t server) {
    return httpd_stop(server);
}

static void connect_handler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data
) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = http_start_server();
    }
}

static void disconnect_handler(
    void* arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void* event_data
) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (http_stop_server(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}
