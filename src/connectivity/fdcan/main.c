#include "connectivity/fdcan/main.h"
#include "driver/twai.h"
#include "connectivity/cmds.h"
#include "connectivity/wifi/https/main.h"
#include "storage/main.h"

static const char *TAG = "user_fdcan_main";

twai_status_info_t twai_state;

FncState fdacn_data_trsm_ready = FNC_DISABLE;

static VecByte fdcan_trsm_buf;
static twai_message_t fdcan_trsm_msg = {
    .data_length_code = 8,
};
static VecByte fdcan_recv_buf;

FdcanByteTrcvBuf fdcan_trsm_pkt_buf;
FdcanByteTrcvBuf fdcan_recv_pkt_buf;

static UNUSED_FNC void fdcan_init(void)
{
    ERROR_CHECK_FNS_HANDLE(vec_byte_new(&fdcan_trsm_buf, 8));
    ERROR_CHECK_FNS_HANDLE(fdcan_trcv_buf_setup(&fdcan_trsm_pkt_buf, FDCAN_TRCV_BUF_CAP, FDCAN_VEC_BYTE_CAP));
    ERROR_CHECK_FNS_HANDLE(vec_byte_new(&fdcan_recv_buf, 8));
    ERROR_CHECK_FNS_HANDLE(fdcan_trcv_buf_setup(&fdcan_recv_pkt_buf, FDCAN_TRCV_BUF_CAP, FDCAN_VEC_BYTE_CAP));
}

static UNUSED_FNC void fdcan_alerts_task(void *arg)
{
    esp_err_t err;
    uint32_t alerts;
    while (1)
    {
        err = twai_read_alerts(&alerts, pdMS_TO_TICKS(10));
        if (err == ESP_ERR_TIMEOUT) continue;
        else if (err == ESP_ERR_INVALID_STATE)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        else if (err != ESP_OK) {
            ESP_LOGE(TAG, "Alert failed: %s", esp_err_to_name(err));
            continue;
        }
        if (alerts & TWAI_ALERT_BUS_OFF) {
            // xTimerStart(recovery_timer, 0);
            twai_initiate_recovery();
        }
        if (alerts & TWAI_ALERT_BUS_RECOVERED) {
            twai_start();
        }
    }
}

static UNUSED_FNC void fdcan_recv_task(void *arg)
{
    esp_err_t err;
    twai_message_t msg;
    while (1)
    {
        err = twai_receive(&msg, pdMS_TO_TICKS(10));
        if (err == ESP_ERR_TIMEOUT) continue;
        else if (err == ESP_ERR_INVALID_STATE)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        else if (err != ESP_OK) {
            ESP_LOGE(TAG, "Msg recv failed: %s", esp_err_to_name(err));
            continue;
        }
        vec_rm_all(&fdcan_recv_buf);
        vec_byte_push(&fdcan_recv_buf, msg.data, msg.data_length_code);
        ESP_LOGI(TAG, "Msg recv ID: 0x%lX  LEN: %02X >>>", msg.identifier, fdcan_recv_buf.len);
        ESP_LOG_BUFFER_HEXDUMP(TAG, fdcan_recv_buf.data, fdcan_recv_buf.len, ESP_LOG_INFO);
        fdcan_trcv_buf_push(&fdcan_recv_pkt_buf, &fdcan_recv_buf, msg.identifier);
        ESP_LOGI(TAG, "Buf len: %d", fdcan_recv_pkt_buf.trcv_buf.len);
    }
}

static FnState pkt_transmit(void)
{
    vec_rm_all(&fdcan_trsm_buf);
    ERROR_CHECK_FNS_RETURN(fdcan_trcv_buf_pop(&fdcan_trsm_pkt_buf, &fdcan_trsm_buf, &fdcan_trsm_msg.identifier));
    memcpy(fdcan_trsm_msg.data, fdcan_trsm_buf.data, fdcan_trsm_buf.len);
    fdcan_trsm_msg.data_length_code = fdcan_trsm_buf.len;
    ESP_LOGI(TAG, "Msg trsm ID: 0x%lX  LEN: %02X >>>", fdcan_trsm_msg.identifier, fdcan_trsm_msg.data_length_code);
    ESP_LOG_BUFFER_HEXDUMP(TAG, fdcan_trsm_msg.data, fdcan_trsm_msg.data_length_code, ESP_LOG_INFO);
    esp_err_t err = twai_transmit(&fdcan_trsm_msg, pdMS_TO_TICKS(1));
    twai_get_status_info(&twai_state);
    ESP_LOGI(TAG, "State=%d, TEC=%lu, REC=%lu", twai_state.state, twai_state.tx_error_counter, twai_state.rx_error_counter);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Msg trsm failed: %s", esp_err_to_name(err));
        return FNS_FAIL;
    }
    return FNS_OK;
}

static FnState trsm_pkt_proc(void)
{
    VecByte vec_byte;
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&vec_byte, 8));
    if (fdacn_data_trsm_ready == FNC_ENABLE)
    {
        #ifdef ENABLE_CON_PKT_TEST
        ERROR_CHECK_FNS_WRI_PUSH(vec_byte_push(&vec_byte, (uint8_t[]){0x00, 0x10}, 2),
            fdcan_trcv_buf_push(&fdcan_trsm_pkt_buf, &vec_byte, 0x40), vec_byte_free(&vec_byte));
        #endif
    }
    vec_byte_free(&vec_byte);
    return FNS_OK;
}

static FnState recv_pkt_proc_inner(VecByte* vec_byte)
{
    uint8_t code;
    ERROR_CHECK_FNS_RETURN(vec_byte_get_byte(vec_byte, 0, &code));
    switch (code)
    {
        case CMD_DATA_B0_CONTROL:
        {
            ERROR_CHECK_FNS_RETURN(vec_byte_get_byte(vec_byte, 1, &code));
            switch (code)
            {
                case CMD_DATA_B1_LEFT_SPEED:
                {
                    uint32_t value;
                    ERROR_CHECK_FNS_RETURN(vec_byte_pop_u32(vec_byte, 2, &value));
                    // Todo vec_byte_check_len(vec_byte, 0);
                    vec_byte_push_u32(&stg_m_left_speed.buffer, value);
                    return FNS_OK;
                }
                case CMD_DATA_B1_RIGHT_SPEED:
                {
                    uint32_t value;
                    ERROR_CHECK_FNS_RETURN(vec_byte_pop_u32(vec_byte, 2, &value));
                    vec_byte_push_u32(&stg_m_right_speed.buffer, value);
                    return FNS_OK;
                }
                case CMD_DATA_B1_LEFT_DUTY:
                {
                    uint8_t value;
                    ERROR_CHECK_FNS_RETURN(vec_byte_pop_byte(vec_byte, 2, &value));
                    vec_byte_push_byte(&stg_m_left_duty.buffer, value);
                    return FNS_OK;
                }
                case CMD_DATA_B1_RIGHT_DUTY:
                {
                    uint8_t value;
                    ERROR_CHECK_FNS_RETURN(vec_byte_pop_byte(vec_byte, 2, &value));
                    vec_byte_push_byte(&stg_m_right_duty.buffer, value);
                    return FNS_OK;
                }
                default: break;
            }
            break;
        }
        default: break;
    }
    last_error = FNS_NOT_FOUND;
    return FNS_NOT_FOUND;
}

static FnState recv_pkt_proc(size_t count)
{
    VecByte vec_byte;
    uint32_t id;
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&vec_byte, HTTPS_RECV_VEC_MAX));
    for (size_t i = 0; i < count; i++)
    {
        if (ERROR_CHECK_FNS_RAW(fdcan_trcv_buf_pop(&fdcan_recv_pkt_buf, &vec_byte, &id))) break;
        last_error = recv_pkt_proc_inner(&vec_byte);

        // https_trcv_buf_push(&https_trsm_pkt_buf, &vec_byte, (int)vec_byte.data[0]);
    }
    vec_byte_free(&vec_byte);
    return FNS_OK;
}

static UNUSED_FNC void fdcan_data_task(void *arg)
{
    size_t tick = 0;
    while (1)
    {
        last_error = pkt_transmit();
        last_error = recv_pkt_proc(5);
        if (tick % 100 == 0)
        {
            tick = 0;
            last_error = trsm_pkt_proc();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        tick++;
    }
}

void fdcan_setup(void)
{
    fdcan_init();
    const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(FDCAN_GPIO_TX, FDCAN_GPIO_RX, TWAI_MODE_NORMAL);
    const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI: %s", esp_err_to_name(err));
        return;
    }
    twai_reconfigure_alerts(TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED, NULL);
    err = twai_start();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI start successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI: %s", esp_err_to_name(err));
        return;
    }

    xTaskCreate(fdcan_alerts_task, "fdcan_alerts_task", 1024, NULL, 6, NULL);
    xTaskCreate(fdcan_recv_task, "fdcan_recv_task", 4096, NULL, 5, NULL);
    xTaskCreate(fdcan_data_task, "fdcan_data_task", 4096, NULL, 5, NULL);
}
