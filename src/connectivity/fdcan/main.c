#include "connectivity/fdcan/main.h"
#include <string.h>
#include "driver/twai.h"
#include "connectivity/fdcan/trcv_buffer.h"

static const char *TAG = "user_fdcan_main";

static TimerHandle_t recovery_timer;

FncState fdacn_data_trsm_ready = FNC_DISABLE;

static VecByte fdcan_tr_buf;
static twai_message_t fdcan_trsm_msg = {
    .data_length_code = 8,
};
static VecByte fdcan_rv_buf;

FdcanByteTrcvBuf fdcan_tr_pkt_buf;
FdcanByteTrcvBuf fdcan_rv_pkt_buf;

#define BUTTON_GPIO  GPIO_NUM_34
gpio_config_t io_conf = {
    .pin_bit_mask = 1ULL << BUTTON_GPIO,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
};

static void recovery_timer_cb(TimerHandle_t xTimer) {
    twai_initiate_recovery();
}

static UNUSED_FNC void fdcan_init(void)
{
    recovery_timer = xTimerCreate("TWAI_RECOVERY", pdMS_TO_TICKS(10), pdFALSE, NULL, recovery_timer_cb);
    ERROR_CHECK_FNS_HANDLE(vec_byte_new(&fdcan_tr_buf, 8));
    ERROR_CHECK_FNS_HANDLE(fdcan_trcv_buf_setup(&fdcan_tr_pkt_buf, FDCAN_TRCV_BUF_CAP, FDCAN_VEC_BYTE_CAP));
    ERROR_CHECK_FNS_HANDLE(vec_byte_new(&fdcan_rv_buf, 8));
    ERROR_CHECK_FNS_HANDLE(fdcan_trcv_buf_setup(&fdcan_rv_pkt_buf, FDCAN_TRCV_BUF_CAP, FDCAN_VEC_BYTE_CAP));
}

static UNUSED_FNC void fdcan_alerts(void)
{
    uint32_t alerts;
    while (1)
    {
        if (twai_read_alerts(&alerts, 0) != ESP_OK)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        // if (alerts & TWAI_ALERT_BUS_OFF) {
        //     xTimerStart(recovery_timer, 0);
        // }
        // if (alerts & TWAI_ALERT_BUS_RECOVERED) {
        //     twai_start();
        // }
    }
}

static UNUSED_FNC void fdcan_recv_task(void *arg)
{
    twai_message_t msg;
    while (1)
    {
        esp_err_t err = twai_receive(&msg, portMAX_DELAY);
        if (err == ESP_ERR_TIMEOUT)
        {
            continue;
        }
        else if (err != ESP_OK) {
            ESP_LOGE(TAG, "Message reception failed: %s", esp_err_to_name(err));
            continue;
        }
        vec_rm_all(&fdcan_rv_buf);
        vec_byte_push(&fdcan_rv_buf, msg.data, msg.data_length_code);
        ESP_LOGI(TAG, "Msg recv: ID=0x%lX, LEN=%02X, Data=[%02X %02X %02X %02X %02X %02X %02X %02X]",
                msg.identifier, fdcan_rv_buf.len,
                fdcan_rv_buf.data[0], fdcan_rv_buf.data[1], fdcan_rv_buf.data[2], fdcan_rv_buf.data[3],
                fdcan_rv_buf.data[4], fdcan_rv_buf.data[5], fdcan_rv_buf.data[6], fdcan_rv_buf.data[7]);
        fdcan_trcv_buf_push(&fdcan_rv_pkt_buf, &fdcan_rv_buf, msg.identifier);
        ESP_LOGI(TAG, "Buf len: %d", fdcan_rv_pkt_buf.trcv_buf.len);
    }
}

static UNUSED_FNC FnState pkt_transmit(void)
{
    vec_rm_all(&fdcan_tr_buf);
    ERROR_CHECK_FNS_RETURN(fdcan_trcv_buf_pop(&fdcan_tr_pkt_buf, &fdcan_tr_buf, &fdcan_trsm_msg.identifier));
    memcpy(fdcan_trsm_msg.data, fdcan_tr_buf.data, fdcan_tr_buf.len);
    fdcan_trsm_msg.data_length_code = fdcan_tr_buf.len;
    ESP_LOGI(TAG, "Msg trsm: ID=0x%lX, LEN=%02X, Data=[%02X %02X %02X %02X %02X %02X %02X %02X]",
                        fdcan_trsm_msg.identifier, fdcan_trsm_msg.data_length_code,
                        fdcan_trsm_msg.data[0], fdcan_trsm_msg.data[1], fdcan_trsm_msg.data[2], fdcan_trsm_msg.data[3],
                        fdcan_trsm_msg.data[4], fdcan_trsm_msg.data[5], fdcan_trsm_msg.data[6], fdcan_trsm_msg.data[7]);
    esp_err_t err = twai_transmit(&fdcan_trsm_msg, pdMS_TO_TICKS(1));
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Message transmission failed: %s", esp_err_to_name(err));
        return FNS_FAIL;
    }
    ESP_LOGI(TAG, "Message sent successfully");
    return FNS_OK;
}

static UNUSED_FNC FnState trsm_pkt_proc(void)
{
    if (gpio_get_level(BUTTON_GPIO) == 0) return FNS_INVALID;
    VecByte vec_byte;
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&vec_byte, 8));
    ERROR_CHECK_FNS_CLEAN(vec_byte_push(&vec_byte, (uint8_t[]){0x00, 0x10}, 2), vec_byte_free(&vec_byte));
    ERROR_CHECK_FNS_CLEAN(fdcan_trcv_buf_push(&fdcan_tr_pkt_buf, &vec_byte, 0x24), vec_byte_free(&vec_byte));
    ERROR_CHECK_FNS_RETURN(vec_byte_free(&vec_byte));
    return FNS_OK;
}

static UNUSED_FNC void fdcan_data_task(void *arg)
{
    size_t tick = 0;
    while (1)
    {
        pkt_transmit();
        // recv_pkt_proc(5);
        if (tick % 100 == 0)
        {
            tick = 0;
            trsm_pkt_proc();
            twai_status_info_t status;
            twai_get_status_info(&status);
            ESP_LOGI(TAG, "State=%d, TEC=%lu, REC=%lu", status.state, status.tx_error_counter, status.rx_error_counter);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        tick++;
    }
}

void fdcan_setup(void)
{
    fdcan_init();
    gpio_config(&io_conf);
    const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_25, GPIO_NUM_26, TWAI_MODE_NORMAL);
    const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI: %s", esp_err_to_name(err));
        return;
    }
    twai_reconfigure_alerts(TWAI_ALERT_RX_DATA | TWAI_ALERT_BUS_OFF | TWAI_ALERT_BUS_RECOVERED, NULL);
    err = twai_start();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI start successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI: %s", esp_err_to_name(err));
        return;
    }

    xTaskCreate(fdcan_recv_task, "fdcan_recv_task", 1024, NULL, 5, NULL);
    // xTaskCreate(fdcan_data_task, "fdcan_data_task", 3072, NULL, 5, NULL);
}
