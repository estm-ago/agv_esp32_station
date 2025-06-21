#include "connectivity/wifi/https/url.h"
#include <string.h>
#include <esp_log.h>
#include "config.h"
#include "fn_state.h"
#include "connectivity/trcv_buffer.h"
#include "connectivity/wifi/https/keep_alive.h"

static const char *TAG = "user_https_ws";

static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    uint8_t *buf = NULL;

    // First receive the full ws message
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        buf = malloc((ws_pkt.len + 1) * sizeof(char));
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to alloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT)
    {
        buf[ws_pkt.len] = '\0';
        if (ws_pkt.payload != NULL) ESP_LOGI(TAG, "Received message: %s", ws_pkt.payload);
        VecByte vec_byte;
        vec_byte_new(&vec_byte, ws_pkt.len);
        if (vec_byte_push(&vec_byte, ws_pkt.payload, ws_pkt.len) == FNS_OK)
        {
            https_trcv_buf_push(&https_rv_pkt_buf, &vec_byte, httpd_req_to_sockfd(req));
            ESP_LOGI(TAG, "Buf count: %d", https_rv_pkt_buf.trcv_buf.len);
        }
        vec_byte_free(&vec_byte);
        
        esp_err_t ret = httpd_ws_send_frame(req, &ws_pkt);
        free(buf);
        return ret;
        // free(buf);
        // return ESP_OK;
    }
    free(buf);
    if (ws_pkt.type == HTTPD_WS_TYPE_PING)
    {
        ESP_LOGI(TAG, "Got a WS PING frame, Replying PONG");
        ws_pkt.type = HTTPD_WS_TYPE_PONG;
        return httpd_ws_send_frame(req, &ws_pkt);
    }
    else if (ws_pkt.type == HTTPD_WS_TYPE_PONG)
    {
        ESP_LOGD(TAG, "Received PONG message");
        return wss_keep_alive_client_is_active(httpd_get_global_user_ctx(req->handle), httpd_req_to_sockfd(req));
    }
    else  if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE)
    {
        ws_pkt.len = 0;
        ws_pkt.payload = NULL;
        return httpd_ws_send_frame(req, &ws_pkt);
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
