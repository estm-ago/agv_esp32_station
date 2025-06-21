#include "connectivity/wifi/https/main.h"
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <esp_netif.h>
#include <esp_eth.h>
#include <esp_wifi.h>
#include <lwip/sockets.h>
#include "sdkconfig.h"
#include "connectivity/wifi/https/keep_alive.h"
#include "connectivity/wifi/https/url.h"

static const char *TAG = "user_https_server";
static const size_t max_clients = 4;

static httpd_handle_t https_server = NULL;

WSByteTrcvBuf https_tr_pkt_buf;
WSByteTrcvBuf https_rv_pkt_buf;

FnState https_ws_setup(void)
{
    FNS_ERROR_CHECK(https_trcv_buf_setup(&https_tr_pkt_buf, 10, WIFI_HTTPS_WS_VEC_MAX));
    FNS_ERROR_CHECK(https_trcv_buf_setup(&https_rv_pkt_buf, 10, WIFI_HTTPS_WS_VEC_MAX));
    return FNS_OK;
}

/**
 * @brief 當客戶端不存活時的回呼，關閉相應連線
 * @param alive_handle keep-alive 引擎 handle
 * @param sockfd 用戶端 socket 描述符
 * @return 返回 true 表示已成功處理並關閉，false 表示處理失敗
 */
static bool client_not_alive_cb(wss_keep_alive_t alive_handle, int sockfd)
{
    ESP_LOGE(TAG, "Client not alive, closing fd %d", sockfd);
    httpd_sess_trigger_close(wss_keep_alive_get_user_ctx(alive_handle), sockfd);
    return true;
}

/**
 * @brief 發送 WebSocket PING 封包給客戶端
 * @param arg 指向 async_resp_arg 結構，包含 httpd_handle 與 fd
 */
static void send_ping(void *arg)
{
    async_resp_arg* resp_arg = (async_resp_arg*) arg;
    httpd_ws_frame_t ws_pkt = {
        .len = 0,
        .type = HTTPD_WS_TYPE_PING,
    };
    httpd_ws_send_frame_async(resp_arg->httpd_handle, resp_arg->sockfd, &ws_pkt);
    free(resp_arg);
}

/**
 * @brief 定期檢查客戶端是否存活，並排程發送 PING
 * @param alive_handle keep-alive 引擎 handle
 * @param sockfd 用戶端 socket 描述符
 * @return 成功排程回傳 true，否則 false
 */
static bool check_client_alive_cb(wss_keep_alive_t alive_handle, int sockfd)
{
    ESP_LOGD(TAG, "Checking if client (fd=%d) is alive", sockfd);
    async_resp_arg *resp_arg = malloc(sizeof(async_resp_arg));
    assert(resp_arg != NULL);
    resp_arg->httpd_handle = wss_keep_alive_get_user_ctx(alive_handle);
    resp_arg->sockfd = sockfd;

    if (httpd_queue_work(resp_arg->httpd_handle, send_ping, resp_arg) != ESP_OK)
    {
        return false;
    }
    return true;
}

/**
 * @brief 在新連線建立時呼叫，將客戶端加入 keep-alive 名單
 * @param httpd_handle HTTPD server handle
 * @param sockfd 客戶端 socket 檔案描述符
 * @return esp_err_t, ESP_OK 表示加入成功
 */
static esp_err_t client_connect(httpd_handle_t httpd_handle, int sockfd) {
    ESP_LOGI(TAG, "New client connected %d", sockfd);
    wss_keep_alive_t h = httpd_get_global_user_ctx(httpd_handle);
    return wss_keep_alive_add_client(h, sockfd);
}

/**
 * @brief 在連線關閉時呼叫，將客戶端從 keep-alive 名單移除，並關閉 socket
 * @param httpd_handle HTTPD server handle
 * @param sockfd 客戶端 socket 描述符
 */
static void client_disconnect(httpd_handle_t httpd_handle, int sockfd) {
    ESP_LOGI(TAG, "Client disconnected %d", sockfd);
    wss_keep_alive_t h = httpd_get_global_user_ctx(httpd_handle);
    wss_keep_alive_remove_client(h, sockfd);
    close(sockfd);
}

/**
 * @brief 啟動內部 HTTPS server，包括 keep-alive 引擎與 URI handler 註冊
 * @return esp_err_t, ESP_OK 表示啟動成功，ESP_ERR_INVALID_STATE 表示已啟動
 */
static esp_err_t https_server_start_inner(void)
{
    if (https_server != NULL) return ESP_ERR_INVALID_STATE;
    // Prepare keep-alive engine
    wss_keep_alive_config_t keep_alive_config = KEEP_ALIVE_CONFIG_DEFAULT();
    keep_alive_config.max_clients = max_clients;
    keep_alive_config.client_not_alive_cb = client_not_alive_cb;
    keep_alive_config.check_client_alive_cb = check_client_alive_cb;
    wss_keep_alive_t keep_alive = wss_keep_alive_start(&keep_alive_config);

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server");

    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
    conf.httpd.max_open_sockets = max_clients;
    conf.httpd.global_user_ctx = keep_alive;
    conf.httpd.open_fn = client_connect;
    conf.httpd.close_fn = client_disconnect;

    extern const unsigned char servercert_start[] asm("_binary_servercert_pem_start");
    extern const unsigned char servercert_end[]   asm("_binary_servercert_pem_end");
    conf.servercert = servercert_start;
    conf.servercert_len = servercert_end - servercert_start;

    extern const unsigned char prvtkey_pem_start[] asm("_binary_prvtkey_pem_start");
    extern const unsigned char prvtkey_pem_end[]   asm("_binary_prvtkey_pem_end");
    conf.prvtkey_pem = prvtkey_pem_start;
    conf.prvtkey_len = prvtkey_pem_end - prvtkey_pem_start;

    if (httpd_ssl_start(&https_server, &conf) != ESP_OK)
    {
        ESP_LOGI(TAG, "Error starting server!");
        return ESP_FAIL;
    }

    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(https_server, &ws);

    wss_keep_alive_set_user_ctx(keep_alive, https_server);
    return ESP_OK;
}

/**
 * @brief 停止內部 HTTPS server，並關閉 keep-alive 引擎
 * @return esp_err_t, ESP_OK 表示停止成功，ESP_ERR_INVALID_STATE 表示未啟動
 */
static esp_err_t https_server_stop_inner(void) {
    if (https_server == NULL) return ESP_ERR_INVALID_STATE;
    wss_keep_alive_stop(httpd_get_global_user_ctx(https_server));

    return httpd_ssl_stop(https_server);
}

/**
 * @brief 當獲得 IP 事件時呼叫，負責啟動 server
 * @param arg 指向 https_server handle 的指標
 * @param event_base 事件類別
 * @param event_id 事件 ID
 * @param event_data 事件資料
 */
static void wifi_connect_handler(
    void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data
) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL)
    {
        https_server_start_inner();
        *server = https_server;
    }
}

/**
 * @brief 當 WiFi 斷線事件時呼叫，負責停止 server
 * @param arg 指向 https_server handle 的指標
 * @param event_base 事件類別
 * @param event_id 事件 ID
 * @param event_data 事件資料
 */
static void wifi_disconnect_handler(
    void* arg, esp_event_base_t event_base,
    int32_t event_id, void* event_data
) {
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL)
    {
        return;
    }
    if (https_server_stop_inner() == ESP_OK) {
        *server = NULL;
    } else {
        ESP_LOGE(TAG, "Failed to stop https server");
    }
}

/**
 * @brief 對外接口：註冊 IP 及 WiFi 事件處理器並啟動 HTTPS server
 * @return esp_err_t, ESP_OK 表示成功啟動，否則返回錯誤碼
 */
esp_err_t https_server_start(void)
{
    esp_err_t result = https_server_start_inner();
    if (result != ESP_OK) return result;
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_connect_handler, &https_server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_disconnect_handler, &https_server));
    return ESP_OK;
}

/**
 * @brief 對外接口：註銷事件處理器並停止 HTTPS server
 * @return esp_err_t, ESP_OK 表示成功停止
 */
esp_err_t https_server_stop(void)
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_connect_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, wifi_disconnect_handler));
    return https_server_stop_inner();
}

/*
static void send_hello(void *arg) {
    static const char * data = "Hello client";
    async_resp_arg *resp_arg = arg;
    httpd_handle_t httpd_handle = resp_arg->httpd_handle;
    int sockfd = resp_arg->sockfd;
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    httpd_ws_send_frame_async(httpd_handle, sockfd, &ws_pkt);
    free(resp_arg);
}

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
                    async_resp_arg *resp_arg = malloc(sizeof(async_resp_arg));
                    assert(resp_arg != NULL);
                    resp_arg->httpd_handle = *server;
                    resp_arg->sockfd = sock;
                    if (httpd_queue_work(resp_arg->httpd_handle, send_hello, resp_arg) != ESP_OK) {
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
*/

void wifi_https_main(void) {
    https_ws_setup();
    https_server_start();
    // wss_server_send_messages(&https_server);
}
