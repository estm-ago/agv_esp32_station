#include "uart/main.h"
#include "prioritites_sequ.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_log.h>
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "global_variable.h"
#include "uart/packet.h"
#include "uart/trcv_buffer.h"
#include "mcu_const.h"

static const char *TAG = "user_uart_main";

static const int RX_BUF_SIZE = VECU8_MAX_CAPACITY;

#define STM32_UART UART_NUM_2
#define STM32_UART_TXD GPIO_NUM_17
#define STM32_UART_RXD GPIO_NUM_16

#define UART_READ_TIMEOUT_MS 10

UartTrcvBuf uart_tr_pkt_buf = {0};
UartTrcvBuf uart_rv_pkt_buf = {0};

float f32_test = 1;
uint16_t u16_test = 1;

/**
 * @brief 組合並傳輸封包至傳輸緩衝區
 *        Assemble and transmit packet into transfer buffer
 *
 * @note 根據 transceive_flags 決定回應內容
 *
 * @return void
 */
static FnState uart_tr_pkt_proc(void)
{
    f32_test++;
    Vec_U8 vec_u8 = VEC_U8_NEW();
    FNS_ERROR_CHECK(vec_u8_push_byte(&vec_u8, CMD_CODE_DATA_TRRE));
    FNS_ERROR_CHECK(vec_u8_push(&vec_u8, CMD_RIGHT_SPEED_STORE, sizeof(CMD_RIGHT_SPEED_STORE)));
    // FNS_ERROR_CHECK(vec_u8_push_f32(&vec_u8, motor_right.speed_present));
    FNS_ERROR_CHECK(vec_u8_push_f32(&vec_u8, f32_test));
    UartPacket packet = UART_PKT_NEW();
    FNS_ERROR_CHECK(uart_pkt_add_data(&packet, &vec_u8));
    FNS_ERROR_CHECK(uart_trcv_buf_push(&uart_tr_pkt_buf, &packet));
    return FNS_OK;
}

/**
 * @brief 處理接收命令並存儲/回應資料
 *        Process received commands and store or respond data
 *
 * @param vec_u8 指向去除命令碼後的資料向量 (input vector without command code)
 * @return void
 */
static FnState uart_re_pkt_proc_data_store(Vec_U8 *vec_u8)
{
    bool data_proc_flag = true;
    while (data_proc_flag)
    {
        data_proc_flag = false;
        if (vec_u8_starts_with(vec_u8, CMD_RIGHT_SPEED_STOP, sizeof(CMD_RIGHT_SPEED_STOP)) == FNS_OK)
        {
            FNS_ERROR_CHECK(vec_u8_rm_range(vec_u8, 0, sizeof(CMD_RIGHT_SPEED_STOP)));
            data_proc_flag = true;
        }
        else if (vec_u8_starts_with(vec_u8, CMD_RIGHT_SPEED_ONCE, sizeof(CMD_RIGHT_SPEED_ONCE)) == FNS_OK)
        {
            FNS_ERROR_CHECK(vec_u8_rm_range(vec_u8, 0, sizeof(CMD_RIGHT_SPEED_ONCE)));
            data_proc_flag = true;
        }
        else if (vec_u8_starts_with(vec_u8, CMD_RIGHT_SPEED_START, sizeof(CMD_RIGHT_SPEED_START)) == FNS_OK)
        {
            FNS_ERROR_CHECK(vec_u8_rm_range(vec_u8, 0, sizeof(CMD_RIGHT_SPEED_START)));
            data_proc_flag = true;
        }
        else if (vec_u8_starts_with(vec_u8, CMD_RIGHT_ADC_STOP, sizeof(CMD_RIGHT_ADC_STOP)) == FNS_OK)
        {
            FNS_ERROR_CHECK(vec_u8_rm_range(vec_u8, 0, sizeof(CMD_RIGHT_ADC_STOP)));
            data_proc_flag = true;
        }
        else if (vec_u8_starts_with(vec_u8, CMD_RIGHT_ADC_ONCE, sizeof(CMD_RIGHT_ADC_ONCE)) == FNS_OK)
        {
            FNS_ERROR_CHECK(vec_u8_rm_range(vec_u8, 0, sizeof(CMD_RIGHT_ADC_ONCE)));
            data_proc_flag = true;
        }
        else if (vec_u8_starts_with(vec_u8, CMD_RIGHT_ADC_START, sizeof(CMD_RIGHT_ADC_START)) == FNS_OK)
        {
            FNS_ERROR_CHECK(vec_u8_rm_range(vec_u8, 0, sizeof(CMD_RIGHT_ADC_START)));
            data_proc_flag = true;
        }
    }
    return FNS_OK;
}

/**
 * @brief 從接收緩衝區反覆讀取封包並處理
 *        Pop packets from receive buffer and process them
 *
 * @param count 單次最大處理封包數量 (input maximum number of packets to process per time)
 * @return void
 */
static FnState uart_re_pkt_proc()
{
    uint8_t i;
    for (i = 0; i < 5; i++)
    {
        UartPacket packet = UART_PKT_NEW();
        FNS_ERROR_CHECK(uart_trcv_buf_pop(&uart_rv_pkt_buf, &packet));
        Vec_U8 vec_u8 = VEC_U8_NEW();
        FNS_ERROR_CHECK(uart_pkt_get_data(&packet, &vec_u8));
        uint8_t code = vec_u8.data[vec_u8.head];
        FNS_ERROR_CHECK(vec_u8_rm_range(&vec_u8, 0, 1));
        switch (code)
        {
            case CMD_CODE_DATA_TRRE:
                FNS_ERROR_CHECK(uart_re_pkt_proc_data_store(&vec_u8));
                break;
            default:
                break;
        }
    }
    return FNS_OK;
}

static FnState uart_write_t(const char* logName, UartPacket *packet)
{
    Vec_U8 vec_u8 = VEC_U8_NEW();
    uart_pkt_unpack(packet, &vec_u8);
    int len = uart_write_bytes(STM32_UART, vec_u8.data, vec_u8.len);
    if (len <= 0) return FNS_FAIL;
    ESP_LOGI(logName, "Wrote %d bytes", len);
    return FNS_OK;
}

static FnState uart_read_t(const char* logName, UartPacket *packet)
{
    Vec_U8 vec_u8 = VEC_U8_NEW();
    vec_u8.len = uart_read_bytes(STM32_UART, vec_u8.data, VECU8_MAX_CAPACITY, pdMS_TO_TICKS(UART_READ_TIMEOUT_MS));
    if (vec_u8.len <= 0) return 0;
    ESP_LOGI(logName, "Read %d bytes >>>", vec_u8.len);
    ESP_LOG_BUFFER_HEXDUMP(logName, vec_u8.data, vec_u8.len, ESP_LOG_INFO);
    UartPacket new = UART_PKT_NEW();
    FNS_ERROR_CHECK(uart_pkt_pack(&new, &vec_u8));
    // ESP_LOGI(logName, "Pack %d bytes", vec_u8.len);
    *packet = new;
    return FNS_OK;
}

static void uart_write_task(void *arg) {
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);

    while (1)
    {
        UartPacket packet = UART_PKT_NEW();
        if (uart_trcv_buf_pop(&global_state.uart_trsm_pkt_buf, &packet))
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        if (uart_write_t(TX_TASK_TAG, &packet))
        {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
    }
    
    vTaskDelete(NULL);
}

static void uart_read_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    ESP_LOGI(RX_TASK_TAG, "Uart read task start");
    while (1)
    {
        UartPacket packet = UART_PKT_NEW();
        if (uart_read_t(RX_TASK_TAG, &packet)) {
            continue;
        }
        uart_trcv_buf_push(&global_state.uart_recv_pkt_buf, &packet);
        // ESP_LOGI(RX_TASK_TAG, "Buf len: %d", global_state.uart_recv_pkt_buf.len);
    }
    vTaskDelete(NULL);
}

void uart_setup(void) {
    ESP_LOGI(TAG, "uart_setup");
    uart_driver_install(STM32_UART, RX_BUF_SIZE, 0, 0, NULL, 0);
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(STM32_UART, &uart_config);
    uart_set_pin(STM32_UART, STM32_UART_TXD, STM32_UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void uart_start(void) {
    xTaskCreate(uart_write_task, "uart_tx_task", 3072, NULL, UART_WRITE_TASK_PRIO_SEQU, NULL);
    xTaskCreate(uart_read_task, "uart_rx_task", 3072, NULL, UART_READ_TASK_PRIO_SEQU, NULL);
}
