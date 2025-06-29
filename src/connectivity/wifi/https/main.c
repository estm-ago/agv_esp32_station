#include "connectivity/wifi/https/main.h"
#include "connectivity/wifi/https/server.h"
#include "connectivity/wifi/https/keep_alive.h"
#include "connectivity/wifi/https/trcv_buffer.h"
#include "connectivity/cmds.h"
#include "connectivity/fdcan/main.h"

static const char *TAG = "user_https_main";
httpd_handle_t https_server = NULL;
FncState https_data_trsm_ready = FNC_DISABLE;

static VecByte https_trsm_buf;
static VecByte https_recv_buf;

static httpd_ws_frame_t trsm_pkt = {
    .type       = HTTPD_WS_TYPE_BINARY,
    .fragmented = false,
    .final      = true,
};
static httpd_ws_frame_t recv_pkt;

WSByteTrcvBuf https_trsm_pkt_buf;
WSByteTrcvBuf https_recv_pkt_buf;

static UNUSED_FNC void https_init(void)
{
    ERROR_CHECK_FNS_HANDLE(vec_byte_new(&https_trsm_buf, HTTPS_TRSM_VEC_MAX));
    ERROR_CHECK_FNS_HANDLE(https_trcv_buf_setup(&https_trsm_pkt_buf, HTTPS_TRCV_BUF_MAX, HTTPS_TRSM_VEC_MAX));
    trsm_pkt.payload = https_trsm_buf.data;
    ERROR_CHECK_FNS_HANDLE(vec_byte_new(&https_recv_buf, HTTPS_RECV_VEC_MAX));
    ERROR_CHECK_FNS_HANDLE(https_trcv_buf_setup(&https_recv_pkt_buf, HTTPS_RECV_BUF_MAX, HTTPS_RECV_VEC_MAX));
}

static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    memset(&recv_pkt, 0, sizeof(httpd_ws_frame_t));
    recv_pkt.payload = https_recv_buf.data;
    esp_err_t ret = httpd_ws_recv_frame(req, &recv_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    https_recv_buf.len = recv_pkt.len;
    if (recv_pkt.len) {
        ESP_LOGI(TAG, "frame len is %d", https_recv_buf.len);
        ret = httpd_ws_recv_frame(req, &recv_pkt, recv_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            return ret;
        }
    }
    switch (recv_pkt.type)
    {
        case HTTPD_WS_TYPE_PING:
        {
            ESP_LOGI(TAG, "Got a WS PING frame, Replying PONG");
            recv_pkt.type = HTTPD_WS_TYPE_PONG;
            return httpd_ws_send_frame(req, &recv_pkt);
        }
        case HTTPD_WS_TYPE_PONG:
        {
            ESP_LOGD(TAG, "Received PONG message");
            return wss_keep_alive_client_is_active(httpd_get_global_user_ctx(req->handle), httpd_req_to_sockfd(req));
        }
        case HTTPD_WS_TYPE_CLOSE:
        {
            recv_pkt.len = 0;
            recv_pkt.payload = NULL;
            return httpd_ws_send_frame(req, &recv_pkt);
        }
        case HTTPD_WS_TYPE_TEXT:
        case HTTPD_WS_TYPE_BINARY:
        {
            int sockfd = httpd_req_to_sockfd(req);
            ESP_LOGI(TAG, "Msg recv FD: %d len: %d >>>", sockfd, https_recv_buf.len);
            ESP_LOG_BUFFER_HEXDUMP(TAG, https_recv_buf.data, https_recv_buf.len, ESP_LOG_INFO);
            https_trcv_buf_push(&https_recv_pkt_buf, &https_recv_buf, sockfd);
            ESP_LOGI(TAG, "Buf count: %d", https_recv_pkt_buf.trcv_buf.len);
            recv_pkt.payload[0] = 0x31;
            recv_pkt.len = 1;
            return httpd_ws_send_frame(req, &recv_pkt);
        }
        default:
        {
            return ESP_ERR_NOT_FOUND;
        }
    }
    return ESP_OK;
}

const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = ws_handler,
        .user_ctx   = NULL,
        .is_websocket = true,
        .handle_ws_control_frames = true
};

static FnState pkt_transmit(void)
{
    vec_rm_all(&https_trsm_buf);
    int sockfd;
    ERROR_CHECK_FNS_RETURN(https_trcv_buf_pop(&https_trsm_pkt_buf, &https_trsm_buf, &sockfd));
    trsm_pkt.len = https_trsm_buf.len;
    ESP_LOGI(TAG, "Msg trsm FD: %d  LEN: %02X >>>", sockfd, trsm_pkt.len);
    ESP_LOG_BUFFER_HEXDUMP(TAG, trsm_pkt.payload, trsm_pkt.len, ESP_LOG_INFO);
    esp_err_t err = httpd_ws_send_frame_async(https_server, sockfd, &trsm_pkt);
    if (err != ESP_OK) {
        https_data_trsm_ready = FNC_DISABLE;
        ESP_LOGE(TAG, "Msg trsm failed: %s", esp_err_to_name(err));
    }
    return FNS_OK;
}

static UNUSED_FNC FnState trsm_pkt_proc(void)
{
    VecByte vec_byte;
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&vec_byte, HTTPS_TRSM_VEC_MAX));
    if (https_data_trsm_ready == FNC_ENABLE)
    {
        #ifdef ENABLE_CON_PKT_TEST
        uint8_t msg[] = "Hello";
        ERROR_CHECK_FNS_WRI_PUSH(vec_byte_push(&vec_byte, msg, sizeof(msg)),
            https_trcv_buf_push(&https_trsm_pkt_buf, &vec_byte, 57), vec_byte_free(&vec_byte));
        ESP_LOGI(TAG, "Buf count: %d", https_trsm_pkt_buf.trcv_buf.len);
        #endif
    }
    ERROR_CHECK_FNS_RETURN(vec_byte_free(&vec_byte));
    return FNS_OK;
}

static FnState recv_pkt_proc0(VecByte* vec_byte)
{
    uint8_t code = vec_byte->data[vec_byte->head];
    switch (code)
    {
        case CMD_B0_VECH_CONTROL:
        {
            if (vec_byte->len > FDCAN_VEC_BYTE_CAP) return FNS_NO_MATCH;
            fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, vec_byte, 0x22);
            break;
        }
        case CMD_B0_ARM_CONTROL:
        {
            if (vec_byte->len > FDCAN_VEC_BYTE_CAP) return FNS_NO_MATCH;
            fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, vec_byte, 0x32);
            break;
        }
        default:
        {
            last_error = FNS_NO_MATCH;
            return FNS_NO_MATCH;
        }
    }
    return FNS_OK;
}

static UNUSED_FNC FnState recv_pkt_proc(size_t count)
{
    VecByte vec_byte;
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&vec_byte, HTTPS_RECV_VEC_MAX));
    for (size_t i = 0; i < count; i++)
    {
        int sockfd;
        ERROR_CHECK_FNS_CLEAN(https_trcv_buf_pop(&https_recv_pkt_buf, &vec_byte, &sockfd), vec_byte_free(&vec_byte));
        // recv_pkt_proc0(&vec_byte);

        if (vec_byte.data[0] == 0x74)
        {
            if (vec_byte.data[1] == 0x73) https_data_trsm_ready = FNC_ENABLE;
            else if (vec_byte.data[1] == 0x65) https_data_trsm_ready = FNC_DISABLE;
        }
        https_trcv_buf_push(&https_trsm_pkt_buf, &vec_byte, sockfd);
        // Todo
        vec_byte.len = 8;
        fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, &vec_byte, 0x0F);
    }
    vec_byte_free(&vec_byte);
    return FNS_OK;
}

static void https_data_task(void *arg)
{
    static const char *TASK_TAG = "user_https_DATA";
    esp_log_level_set(TASK_TAG, ESP_LOG_INFO);
    size_t tick = 0;
    for(;;)
    {
        pkt_transmit();
        recv_pkt_proc(5);
        if (tick % 20 == 0)
        {
            tick = 0;
            trsm_pkt_proc();
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        tick++;
    }
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

FnState https_server_setup(void) {
    https_init();
    https_server_start();
    xTaskCreate(https_data_task, "https_data_task", 4096, NULL, HTTPS_DATA_TASK_PRIO_SEQU, NULL);

    // wss_server_send_messages(&https_server);
    return FNS_OK;
}

