#include "connectivity/wifi/https/main.h"
#include "connectivity/wifi/https/server.h"
#include "connectivity/wifi/https/keep_alive.h"
#include "connectivity/wifi/https/trcv_buffer.h"
#include "connectivity/cmds.h"
#include "connectivity/fdcan/main.h"
#include "storage/main.h"

static const char *TAG = "user_https_main";
httpd_handle_t https_server = NULL;
FncState https_data_trsm_ready = FNC_DISABLE;
int controller_fd = 57;

static httpd_ws_frame_t trsm_pkt = {
    .type       = HTTPD_WS_TYPE_BINARY,
    .fragmented = false,
    .final      = true,
};
static VecByte https_trsm_buf;
static httpd_ws_frame_t recv_pkt;
static VecByte https_recv_buf;

WSByteTrcvBuf https_trsm_pkt_buf;
WSByteTrcvBuf https_recv_pkt_buf;

VecByte vec_sep;
static UNUSED_FNC void https_init(void)
{
    RESULT_CHECK_HANDLE(vec_byte_new(&https_trsm_buf, HTTPS_TRSM_VEC_MAX));
    RESULT_CHECK_HANDLE(https_trcv_buf_setup(&https_trsm_pkt_buf, HTTPS_TRCV_BUF_MAX, HTTPS_TRSM_VEC_MAX));
    trsm_pkt.payload = https_trsm_buf.data;
    RESULT_CHECK_HANDLE(vec_byte_new(&https_recv_buf, HTTPS_RECV_VEC_MAX));
    RESULT_CHECK_HANDLE(https_trcv_buf_setup(&https_recv_pkt_buf, HTTPS_RECV_BUF_MAX, HTTPS_RECV_VEC_MAX));
    RESULT_CHECK_HANDLE(vec_byte_new(&vec_sep, FDCAN_VEC_BYTE_CAP));
}

static Result recv_pkt_proc_file(FileData* file_data, VecByte* in_byte, int sockfd)
{
    Result result;
    VecByte vec_byte;
    RESULT_CHECK_CLEANUP(vec_byte_new(&vec_byte, STORAGE_GET_DATA * file_data->type + 2));
    // RESULT_CHECK_CLEANUP(storage_store_data(file_data));
    vec_byte_realign(in_byte);
    RESULT_CHECK_CLEANUP(vec_byte_push(&vec_byte, in_byte->data, 2));
    RESULT_CHECK_CLEANUP(file_data_get(file_data->path, STORAGE_GET_DATA, &vec_byte));
    RESULT_CHECK_CLEANUP(https_trcv_buf_push(&https_trsm_pkt_buf, &vec_byte, sockfd));
    cleanup:
    vec_byte_free(&vec_byte);
    return result;
}

static Result instant_recv_proc(VecByte* vec_byte, int sockfd)
{
    uint8_t code;
    RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 0, &code));
    switch (code)
    {
    case CMD_DATA_B0_CONTROL:
        {
            RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 1, &code));
            switch (code)
            {
                case CMD_DATA_B1_LEFT_SPEED:
                {
                    return recv_pkt_proc_file(&stg_wheel_left_speed, vec_byte, sockfd);
                }
                case CMD_DATA_B1_RIGHT_SPEED:
                {
                    return recv_pkt_proc_file(&stg_wheel_right_speed, vec_byte, sockfd);
                }
                case CMD_DATA_B1_LEFT_DUTY:
                {
                    return recv_pkt_proc_file(&stg_wheel_left_duty, vec_byte, sockfd);
                }
                case CMD_DATA_B1_RIGHT_DUTY:
                {
                    return recv_pkt_proc_file(&stg_wheel_right_duty, vec_byte, sockfd);
                }
                default: break;
            }
            break;
        }
        case CMD_VEHI_B0_CONTROL:
        {
            return fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, vec_byte, 0x21);
        }
        case CMD_ARM_B0_CONTROL:
        {
            return fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, vec_byte, 0x31);
        }
        default: break;
    }
    return RESULT_OK(NULL);
}

static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    memset(&recv_pkt, 0, sizeof(httpd_ws_frame_t));
    vec_rm_all(&https_recv_buf);
    recv_pkt.payload = https_recv_buf.data;
    esp_err_t ret = httpd_ws_recv_frame(req, &recv_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    Result result = vec_byte_add_len(&https_recv_buf, recv_pkt.len);
    if (RESULT_CHECK_RAW(result))
    {
        ESP_LOGE(TAG, "Len OF: %d", result.result.error);
        return ESP_FAIL;
    }
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
            wss_keep_alive_t keep_alive = (wss_keep_alive_t)httpd_get_global_user_ctx(req->handle);
            return wss_keep_alive_client_is_active(keep_alive, httpd_req_to_sockfd(req));
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
            wss_keep_alive_t keep_alive = (wss_keep_alive_t)httpd_get_global_user_ctx(req->handle);
            int sockfd = httpd_req_to_sockfd(req);
            ESP_LOGI(TAG, "Msg recv FD: %d len: %d >>>", sockfd, https_recv_buf.len);
            ESP_LOG_BUFFER_HEXDUMP(TAG, https_recv_buf.data, https_recv_buf.len, ESP_LOG_INFO);
            https_trcv_buf_push(&https_recv_pkt_buf, &https_recv_buf, sockfd);
            ESP_LOGI(TAG, "Buf count: %d", https_recv_pkt_buf.trcv_buf.len);
            return wss_keep_alive_client_is_active(keep_alive, sockfd);
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

static Result pkt_transmit(void)
{
    vec_rm_all(&https_trsm_buf);
    int sockfd;
    RESULT_CHECK_RET_RES(https_trcv_buf_pop(&https_trsm_pkt_buf, &https_trsm_buf, &sockfd));
    trsm_pkt.len = https_trsm_buf.len;
    ESP_LOGI(TAG, "Msg trsm FD: %d LEN: %02X >>>", sockfd, trsm_pkt.len);
    ESP_LOG_BUFFER_HEXDUMP(TAG, trsm_pkt.payload, trsm_pkt.len, ESP_LOG_INFO);
    esp_err_t err = httpd_ws_send_frame_async(https_server, sockfd, &trsm_pkt);
    if (err != ESP_OK) {
        https_data_trsm_ready = FNC_DISABLE;
        ESP_LOGE(TAG, "Msg trsm failed: %s", esp_err_to_name(err));
    }
    return RESULT_OK(NULL);
}

static Result trsm_pkt_proc(void)
{
    VecByte vec_byte;
    RESULT_CHECK_RET_RES(vec_byte_new(&vec_byte, HTTPS_TRSM_VEC_MAX));
    if (https_data_trsm_ready == FNC_ENABLE)
    {
        #ifdef ENABLE_CON_PKT_TEST
        uint8_t msg[] = "Hello\n";
        ERROR_CHECK_FNS_WRI_PUSH(vec_byte_push(&vec_byte, msg, sizeof(msg)),
            https_trcv_buf_push(&https_trsm_pkt_buf, &vec_byte, controller_fd), vec_byte_free(&vec_byte));
        ESP_LOGI(TAG, "Buf count: %d", https_trsm_pkt_buf.trcv_buf.len);
        #endif
    }
    vec_byte_free(&vec_byte);
    return RESULT_OK(NULL);
}

static Result recv_pkt_proc_inner(VecByte* vec_byte, int sockfd)
{
    uint8_t code;
    RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 0, &code));
    ESP_LOGI(TAG, "recv_pkt_proc_inner: %02X", code);
    switch (code)
    {
        case CMD_DATA_B0_CONTROL:
        {
            RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 1, &code));
            switch (code)
            {
                case CMD_DATA_B1_LEFT_SPEED:
                {
                    return recv_pkt_proc_file(&stg_wheel_left_speed, vec_byte, sockfd);
                }
                case CMD_DATA_B1_RIGHT_SPEED:
                {
                    return recv_pkt_proc_file(&stg_wheel_right_speed, vec_byte, sockfd);
                }
                case CMD_DATA_B1_LEFT_DUTY:
                {
                    return recv_pkt_proc_file(&stg_wheel_left_duty, vec_byte, sockfd);
                }
                case CMD_DATA_B1_RIGHT_DUTY:
                {
                    return recv_pkt_proc_file(&stg_wheel_right_duty, vec_byte, sockfd);
                }
                default: break;
            }
            break;
        }
        case CMD_WHEEL_B0_CONTROL:
        {
            RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 1, &code));
            if (code == CMD_SOCKET_CHECK)
            {
                controller_fd = sockfd;
                return RESULT_OK(NULL);
            }
        }
        case CMD_VEHI_B0_CONTROL:
        {
            return fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, vec_byte, 0x21);
        }
        case CMD_MAP_B0_CONTROL:
        case CMD_ARM_B0_CONTROL:
        {
            return fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, vec_byte, 0x31);
        }
        case 0x74:  // t
        {
            RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 1, &code));
            RESULT_CHECK_RET_RES(vec_rm_range(vec_byte, 0, 2));
            ESP_LOGI(TAG, "recv_pkt_proc_inner: %02X", code);
            switch (code)
            {
                case 0x66:  // f
                {
                    RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 0, &code));
                    RESULT_CHECK_RET_RES(vec_rm_range(vec_byte, 0, 1));
                    switch (code)
                    {
                        case 0x30:  // 0
                            fdacn_data_trsm_ready = FNC_DISABLE;
                            return RESULT_OK(NULL);
                        case 0x31:  // 1
                            fdacn_data_trsm_ready = FNC_ENABLE;
                            return RESULT_OK(NULL);
                        default: break;
                    }
                    break;
                }
                case 0x68:  // h
                {
                    RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 0, &code));
                    RESULT_CHECK_RET_RES(vec_rm_range(vec_byte, 0, 1));
                    switch (code)
                    {
                        case 0x30:
                        {
                            https_data_trsm_ready = FNC_DISABLE;
                            return RESULT_OK(NULL);
                        }
                        case 0x31:
                        {
                            https_data_trsm_ready = FNC_ENABLE;
                            return RESULT_OK(NULL);
                        }
                        default: break;
                    }
                    break;
                }
                default: break;
            }
            break;
        }
        default:
        {
            // if (vec_byte->len > FDCAN_VEC_BYTE_CAP) vec_byte->len = FDCAN_VEC_BYTE_CAP;
            // RESULT_CHECK_RET_RES(fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, vec_byte, 0x0F));
            // RESULT_CHECK_RET_RES(recv_pkt_return(vec_byte, 0x32, sockfd));
            break;
        }
    };
    ESP_LOGE(TAG, "recv_pkt_proc_inner: %02X", code);
    return RESULT_ERROR(RES_ERR_NOT_FOUND);
}

static Result recv_pkt_return(uint8_t code, int sockfd)
{
    VecByte vec_byte;
    Result result;
    RESULT_CHECK_CLEANUP(vec_byte_new(&vec_byte, 2));
    RESULT_CHECK_CLEANUP(vec_byte_push_byte(&vec_byte, code));
    RESULT_CHECK_CLEANUP(vec_byte_push_byte(&vec_byte, '\n'));
    RESULT_CHECK_CLEANUP(https_trcv_buf_push(&https_trsm_pkt_buf, &vec_byte, sockfd));
    cleanup:
    vec_byte_free(&vec_byte);
    return result;
}

static Result recv_pkt_proc(size_t count)
{
    VecByte vec_https, vec_fdcan;
    int sockfd;
    Result result;
    RESULT_CHECK_CLEANUP(vec_byte_new(&vec_https, HTTPS_RECV_VEC_MAX));
    RESULT_CHECK_CLEANUP(vec_byte_new(&vec_fdcan, FDCAN_VEC_BYTE_CAP));
    for (size_t i = 0; i < count; i++)
    {
        RESULT_CHECK_CLEANUP(https_trcv_buf_pop(&https_recv_pkt_buf, &vec_https, &sockfd));
        while (!RESULT_CHECK_RAW(vec_byte_pop_can(&vec_https, &vec_fdcan)))
        {
            ESP_LOGI(TAG, "R Length: %d", vec_https.len);
            recv_pkt_proc_inner(&vec_fdcan, sockfd);
        }
        ESP_LOGI(TAG, "OR Length: %d", vec_https.len);
        recv_pkt_return(0x31, sockfd);
    }
    cleanup:
    vec_byte_free(&vec_https);
    vec_byte_free(&vec_fdcan);
    return result;
}

static UNUSED_FNC void https_data_task(void *arg)
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

Result https_server_setup(void) {
    https_init();
    https_server_start();
    xTaskCreate(https_data_task, "https_data_task", 6144, NULL, HTTPS_DATA_TASK_PRIO_SEQU, NULL);

    return RESULT_OK(NULL);
}
