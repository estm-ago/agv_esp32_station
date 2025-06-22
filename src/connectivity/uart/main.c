#include "connectivity/uart/main.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "global_variable.h"
#include "config.h"
#include "connectivity/trcv_buffer.h"
#include "mcu_const.h"

static const char *TAG = "user_uart_main";

#define STM32_UART UART_NUM_2
#define STM32_UART_TXD GPIO_NUM_17
#define STM32_UART_RXD GPIO_NUM_16

static VecByte uart_tr_buf;
static VecByte uart_rv_buf;

ByteTrcvBuf uart_tr_pkt_buf;
ByteTrcvBuf uart_rv_pkt_buf;

TransceiveFlags transceive_flags = {0};

static FnState uart_read_t(const char* logName)
{
    vec_rm_all(&uart_rv_buf);
    uart_rv_buf.len = uart_read_bytes(STM32_UART, uart_rv_buf.data, uart_rv_buf.cap, pdMS_TO_TICKS(1));
    if (uart_rv_buf.len <= 0) return FNS_BUF_EMPTY;
    ESP_LOGI(logName, "Read %d bytes >>>", uart_rv_buf.len);
    ESP_LOG_BUFFER_HEXDUMP(logName, uart_rv_buf.data, uart_rv_buf.len, ESP_LOG_INFO);
    if (
        (uart_rv_buf.data[0] == UART_START_CODE)
        && (uart_rv_buf.data[uart_rv_buf.len-1] == UART_END_CODE)
    ) {
        FNS_ERROR_CHECK(connect_trcv_buf_push(&uart_rv_pkt_buf, &uart_rv_buf));
        return FNS_OK;
    }
    else
    {
        return FNS_NO_MATCH;
    }
}

static void uart_recv_task(void *arg) {
    static const char *TASK_TAG = "user_uart_RECV";
    esp_log_level_set(TASK_TAG, ESP_LOG_INFO);
    ESP_LOGI(TASK_TAG, "Uart RX task start");
    for(;;)
    {
        if (uart_read_t(TASK_TAG) != FNS_BUF_EMPTY) continue;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static FnState uart_write_t(const char* logName)
{
    vec_rm_all(&uart_tr_buf);
    FNS_ERROR_CHECK(vec_byte_push_byte(&uart_tr_buf, UART_START_CODE));
    FNS_ERROR_CHECK(connect_trcv_buf_pop(&uart_tr_pkt_buf, &uart_tr_buf));
    FNS_ERROR_CHECK(vec_byte_push_byte(&uart_tr_buf, UART_END_CODE));
    int len = uart_write_bytes(STM32_UART, uart_tr_buf.data, uart_tr_buf.len);
    if (len <= 0) return FNS_FAIL;
    ESP_LOGI(logName, "Wrote %d bytes", len);
    return FNS_OK;
}

static FnState uart_tr_pkt_proc(void)
{
    VecByte vec_u8;
    FNS_ERROR_CHECK(vec_byte_new(&vec_u8, UART_VEC_MAX));
    FNS_ERROR_CHECK_CLEAN(vec_byte_push_byte(&vec_u8, CMD_B0_DATA_START), vec_byte_free(&vec_u8));
    FNS_ERROR_CHECK_CLEAN(connect_trcv_buf_push(&uart_tr_pkt_buf, &vec_u8), vec_byte_free(&vec_u8));
    vec_byte_free(&vec_u8);
    return FNS_OK;
}

/**
 * @brief 從接收緩衝區反覆讀取封包並處理
 *        Pop packets from receive buffer and process them
 *
 * @param count 單次最大處理封包數量 (input maximum number of packets to process per time)
 * @return void
 */
static FnState uart_re_pkt_proc(size_t count)
{
    VecByte vec_u8;
    FNS_ERROR_CHECK(vec_byte_new(&vec_u8, UART_VEC_MAX));
    for (size_t i = 0; i < count; i++)
    {
        FNS_ERROR_CHECK_CLEAN(connect_trcv_buf_pop(&uart_rv_pkt_buf, &vec_u8), vec_byte_free(&vec_u8));
        uint8_t code = vec_u8.data[vec_u8.head];
        vec_rm_range(&vec_u8, 0, 1);
        switch (code)
        {
            case CMD_B0_DATA:
                break;
            default:
                break;
        }
    }
    vec_byte_free(&vec_u8);
    return FNS_OK;
}

static void uart_data_task(void *arg)
{
    static const char *TASK_TAG = "user_uart_DATA";
    esp_log_level_set(TASK_TAG, ESP_LOG_INFO);
    vTaskDelay(pdMS_TO_TICKS(1000));
    uart_tr_pkt_proc();
    size_t tick = 0;
    for(;;)
    {
        uart_write_t(TASK_TAG);
        uart_re_pkt_proc(5);
        if (tick % 500 == 0)
        {
            tick = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        tick++;
    }
}

void uart_setup(void) {
    ESP_LOGI(TAG, "uart_setup");
    uart_driver_install(STM32_UART, UART_VEC_MAX + 2, 0, 0, NULL, 0);
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(STM32_UART, &uart_config);
    // Tx:GPIO17 Rx:GPIO16
    uart_set_pin(STM32_UART, STM32_UART_TXD, STM32_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    FNS_ERROR_CHECK_VOID(vec_byte_new(&uart_tr_buf, UART_VEC_MAX + 2));
    FNS_ERROR_CHECK_VOID(vec_byte_new(&uart_rv_buf, UART_VEC_MAX + 2));
    FNS_ERROR_CHECK_VOID(connect_trcv_buf_setup(&uart_tr_pkt_buf, UART_TRCV_BUF_CAP, UART_VEC_MAX));
    FNS_ERROR_CHECK_VOID(connect_trcv_buf_setup(&uart_rv_pkt_buf, UART_TRCV_BUF_CAP, UART_VEC_MAX));

    xTaskCreate(uart_recv_task, "uart_recv_task", 1024, NULL, UART_READ_TASK_PRIO_SEQU, NULL);
    xTaskCreate(uart_data_task, "uart_data_task", 2048, NULL, UART_DATA_TASK_PRIO_SEQU, NULL);
}
