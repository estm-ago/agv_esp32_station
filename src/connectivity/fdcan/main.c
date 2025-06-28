#include "connectivity/fdcan/main.h"
#include "driver/twai.h"

static const char *TAG = "TWAI_SIMPLE";

twai_message_t msg_tx = {
    .identifier = 0x024,
    .data_length_code = 8,
    .data = {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};
twai_message_t msg_rx;

#define BUTTON_GPIO  GPIO_NUM_34
static void twai_tx_task(void *arg)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        twai_status_info_t status;
        twai_get_status_info(&status);
        ESP_LOGI(TAG, "State=%d, TEC=%lu, REC=%lu", status.state, status.tx_error_counter, status.rx_error_counter);
        if (gpio_get_level(BUTTON_GPIO) == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        esp_err_t err = twai_transmit(&msg_tx, pdMS_TO_TICKS(1));
        msg_tx.data[0]++;
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Message sent successfully");
        } else
        {
            ESP_LOGE(TAG, "Message transmission failed: %s", esp_err_to_name(err));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void twai_recv_task(void *arg)
{
    esp_err_t err;
    uint32_t alerts;
    while (1)
    {
        if (twai_read_alerts(&alerts, pdMS_TO_TICKS(10)) != ESP_OK)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (alerts & TWAI_ALERT_RX_DATA)
        {
            while (1)
            {
                err = twai_receive(&msg_rx, 0);
                if (err == ESP_ERR_TIMEOUT) break;
                if (err == ESP_OK) {
                    ESP_LOGI(TAG, "Message received: ID=0x%lX, Data=[%02X %02X %02X %02X %02X %02X %02X %02X]",
                        msg_rx.identifier,
                        msg_rx.data[0], msg_rx.data[1], msg_rx.data[2], msg_rx.data[3],
                        msg_rx.data[4], msg_rx.data[5], msg_rx.data[6], msg_rx.data[7]);
                }
                else
                {
                    ESP_LOGE(TAG, "Message reception failed: %s", esp_err_to_name(err));
                }
            }
        }
        if (alerts & TWAI_ALERT_BUS_OFF) {
            ESP_LOGW(TAG, "Bus-Off detected, restarting TWAI...");
            twai_initiate_recovery();
        }
        if (alerts & TWAI_ALERT_BUS_RECOVERED) {
            ESP_LOGI(TAG, "Bus-Recovered, restarting TWAI...");
            twai_start();
        }
    }
}

void fdcan_setup(void)
{
    const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_25, GPIO_NUM_26, TWAI_MODE_NORMAL);
    const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to install TWAI driver: %s", esp_err_to_name(err));
        return;
    }
    twai_reconfigure_alerts(TWAI_ALERT_RX_DATA
        | TWAI_ALERT_BUS_OFF
        | TWAI_ALERT_BUS_RECOVERED, NULL);
    err = twai_start();
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver started successfully");
    } else {
        ESP_LOGE(TAG, "Failed to start TWAI driver: %s", esp_err_to_name(err));
        return;
    }

    xTaskCreate(twai_recv_task, "twai_recv_task", 4096, NULL, 5, NULL);
    xTaskCreate(twai_tx_task, "twai_tx_task", 4096, NULL, 5, NULL);
}
