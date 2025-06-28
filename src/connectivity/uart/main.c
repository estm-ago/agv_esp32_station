#include "connectivity/uart/main.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include "string.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "global_variable.h"
#include "config.h"
#include "connectivity/trcv_buffer.h"
#include "connectivity/cmds.h"

static const char *TAG = "user_uart_main";

bool uart_enable = false;

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
        ERROR_CHECK_FNS_RETURN(connect_trcv_buf_push(&uart_rv_pkt_buf, &uart_rv_buf));
        return FNS_OK;
    }
    else
    {
        return FNS_NO_MATCH;
    }
}

static UNUSED_FNC void uart_recv_task(void *arg) {
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
    if (uart_enable != true) return FNS_INVALID;
    vec_rm_all(&uart_tr_buf);
    ERROR_CHECK_FNS_RETURN(vec_byte_push_byte(&uart_tr_buf, UART_START_CODE));
    ERROR_CHECK_FNS_RETURN(connect_trcv_buf_pop(&uart_tr_pkt_buf, &uart_tr_buf));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_byte(&uart_tr_buf, UART_END_CODE));
    int len = uart_write_bytes(STM32_UART, uart_tr_buf.data, uart_tr_buf.len);
    if (len <= 0) return FNS_FAIL;
    ESP_LOGI(logName, "Wrote %d bytes", len);
    return FNS_OK;
}

static FnState uart_tr_pkt_proc(void)
{
    VecByte vec_byte;
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&vec_byte, UART_VEC_BYTE_CAP));
    ERROR_CHECK_FNS_CLEAN(vec_byte_push_byte(&vec_byte, CMD_B0_DATA_START), vec_byte_free(&vec_byte));
    ERROR_CHECK_FNS_CLEAN(connect_trcv_buf_push(&uart_tr_pkt_buf, &vec_byte), vec_byte_free(&vec_byte));
    vec_byte_free(&vec_byte);
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
    VecByte vec_byte;
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&vec_byte, UART_VEC_BYTE_CAP));
    for (size_t i = 0; i < count; i++)
    {
        ERROR_CHECK_FNS_CLEAN(connect_trcv_buf_pop(&uart_rv_pkt_buf, &vec_byte), vec_byte_free(&vec_byte));
        uint8_t code = vec_byte.data[vec_byte.head];
        vec_rm_range(&vec_byte, 0, 1);
        switch (code)
        {
            case CMD_B0_DATA:
                break;
            default:
                break;
        }
    }
    vec_byte_free(&vec_byte);
    return FNS_OK;
}

static UNUSED_FNC void uart_data_task(void *arg)
{
    static const char *TASK_TAG = "user_uart_DATA";
    esp_log_level_set(TASK_TAG, ESP_LOG_INFO);
    vTaskDelay(pdMS_TO_TICKS(1000));
    uart_tr_pkt_proc();
    size_t tick = 0;
    for(;;)
    {
#ifndef DISABLE_UART_TRSM
        uart_write_t(TASK_TAG);
#endif
        uart_re_pkt_proc(5);
        if (tick % 500 == 0)
        {
            tick = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
        tick++;
    }
}

void uart_setup(void)
{
#ifndef DISABLE_UART
    ESP_LOGI(TAG, "uart_setup");
    uart_driver_install(STM32_UART, UART_VEC_BYTE_CAP + 2, 0, 0, NULL, 0);
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
    FNS_ERROR_CHECK_VOID(vec_byte_new(&uart_tr_buf, UART_VEC_BYTE_CAP + 2));
    FNS_ERROR_CHECK_VOID(vec_byte_new(&uart_rv_buf, UART_VEC_BYTE_CAP + 2));
    FNS_ERROR_CHECK_VOID(connect_trcv_buf_setup(&uart_tr_pkt_buf, UART_TRCV_BUF_CAP, UART_VEC_BYTE_CAP));
    FNS_ERROR_CHECK_VOID(connect_trcv_buf_setup(&uart_rv_pkt_buf, UART_TRCV_BUF_CAP, UART_VEC_BYTE_CAP));

    xTaskCreate(uart_recv_task, "uart_recv_task", 1024, NULL, UART_READ_TASK_PRIO_SEQU, NULL);
    xTaskCreate(uart_data_task, "uart_data_task", 2048, NULL, UART_DATA_TASK_PRIO_SEQU, NULL);
    uart_enable = true;
#endif
}
