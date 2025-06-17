#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_netif.h>
#include <esp_eth.h>
#include <esp_wifi.h>
#include <lwip/sockets.h>
#include <esp_https_server.h>
#include "wifi/https/server.h"
#include "wifi/https/keep_alive.h"
#include "wifi/https/url.h"
#include "sdkconfig.h"

struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

static const char *TAG = "user_wss_echo_server";
static const size_t max_clients = 4;

static httpd_handle_t https_server = NULL;

static bool client_not_alive_cb(wss_keep_alive_t h, int fd) {
    ESP_LOGE(TAG, "Client not alive, closing fd %d", fd);
    httpd_sess_trigger_close(wss_keep_alive_get_user_ctx(h), fd);
    return true;
}

static void send_ping(void *arg) {
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = NULL;
    ws_pkt.len = 0;
    ws_pkt.type = HTTPD_WS_TYPE_PING;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

static bool check_client_alive_cb(wss_keep_alive_t h, int fd) {
    ESP_LOGD(TAG, "Checking if client (fd=%d) is alive", fd);
    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
    assert(resp_arg != NULL);
    resp_arg->hd = wss_keep_alive_get_user_ctx(h);
    resp_arg->fd = fd;

    if (httpd_queue_work(resp_arg->hd, send_ping, resp_arg) != ESP_OK) {
        return false;
    }
    return true;
}

static esp_err_t wss_open_fd(httpd_handle_t hd, int sockfd) {
    ESP_LOGI(TAG, "New client connected %d", sockfd);
    wss_keep_alive_t h = httpd_get_global_user_ctx(hd);
    return wss_keep_alive_add_client(h, sockfd);
}

static void wss_close_fd(httpd_handle_t hd, int sockfd) {
    ESP_LOGI(TAG, "Client disconnected %d", sockfd);
    wss_keep_alive_t h = httpd_get_global_user_ctx(hd);
    wss_keep_alive_remove_client(h, sockfd);
    close(sockfd);
}

static esp_err_t https_server_start_inner(void) {
    if (https_server != NULL) return ESP_ERR_INVALID_STATE;
    // Prepare keep-alive engine
    wss_keep_alive_config_t keep_alive_config = KEEP_ALIVE_CONFIG_DEFAULT();
    keep_alive_config.max_clients = max_clients;
    keep_alive_config.client_not_alive_cb = client_not_alive_cb;
    keep_alive_config.check_client_alive_cb = check_client_alive_cb;
    wss_keep_alive_t keep_alive = wss_keep_alive_start(&keep_alive_config);

    // Start the httpd server
    https_server = NULL;
    ESP_LOGI(TAG, "Starting server");

    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
    conf.httpd.max_open_sockets = max_clients;
    conf.httpd.global_user_ctx = keep_alive;
    conf.httpd.open_fn = wss_open_fd;
    conf.httpd.close_fn = wss_close_fd;

    extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
    extern const unsigned char servercert_end[]   asm("_binary_servercert_pem_end");
    conf.servercert = servercert_start;
    conf.servercert_len = servercert_end - servercert_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
    conf.prvtkey_pem = prvtkey_pem_start;
    conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    esp_err_t ret = httpd_ssl_start(&https_server, &conf);
    if (ESP_OK != ret) {
        ESP_LOGI(TAG, "Error starting server!");
        return ESP_FAIL;
    }

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(https_server, &ws);

    wss_keep_alive_set_user_ctx(keep_alive, https_server);
    return ESP_OK;
}

static esp_err_t https_server_stop_inner(void) {
    if (https_server == NULL) return ESP_ERR_INVALID_STATE;
    wss_keep_alive_stop(httpd_get_global_user_ctx(https_server));
    // Stop the httpd server
    return httpd_ssl_stop(https_server);
}

static void connect_handler(
    void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data
) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        https_server_start_inner();
        *server = https_server;
    }
}

static void disconnect_handler(
    void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data
) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        return;
    }
    if (https_server_stop_inner() == ESP_OK) {
        *server = NULL;
    } else {
        ESP_LOGE(TAG, "Failed to stop https server");
    }
}

esp_err_t https_server_start(void) {
    esp_err_t result = https_server_start_inner();
    if (result != ESP_OK) return result;
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &https_server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &https_server));
    return ESP_OK;
}

esp_err_t https_server_stop(void) {
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, connect_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, disconnect_handler));
    return https_server_stop_inner();
}

static void send_hello(void *arg) {
    static const char * data = "Hello client";
    struct async_resp_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg);
}

// Get all clients and send async message
static void wss_server_send_messages(httpd_handle_t* server) {
    ESP_LOGI(TAG, "wss_server_send_messages");
    bool send_messages = true;

    // Send async message to all connected clients that use websocket protocol every 10 seconds
    while (send_messages) {
        vTaskDelay(10000 / portTICK_PERIOD_MS);

        if (!*server) { // httpd might not have been created by now
            continue;
        }
        size_t clients = max_clients;
        int    client_fds[max_clients];
        if (httpd_get_client_list(*server, &clients, client_fds) == ESP_OK) {
            for (size_t i=0; i < clients; ++i) {
                int sock = client_fds[i];
                if (httpd_ws_get_fd_info(*server, sock) == HTTPD_WS_CLIENT_WEBSOCKET) {
                    ESP_LOGI(TAG, "Active client (fd=%d) -> sending async message", sock);
                    struct async_resp_arg *resp_arg = malloc(sizeof(struct async_resp_arg));
                    assert(resp_arg != NULL);
                    resp_arg->hd = *server;
                    resp_arg->fd = sock;
                    if (httpd_queue_work(resp_arg->hd, send_hello, resp_arg) != ESP_OK) {
                        ESP_LOGE(TAG, "httpd_queue_work failed!");
                        send_messages = false;
                        break;
                    }
                }
            }
        } else {
            ESP_LOGE(TAG, "httpd_get_client_list failed!");
            return;
        }
    }
}

void wifi_https_main(void) {

    https_server_start();

    wss_server_send_messages(&https_server);
}
