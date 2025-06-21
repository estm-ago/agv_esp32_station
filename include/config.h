/*
#include "config.h"
*/
#pragma once

#include <stdbool.h>
#include "freertos/FreeRTOSConfig.h"

#define BOARD_LED_TOGGLE HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5)

#define VECU8_MAX_CAPACITY 4096
#define WIFI_HTTPS_WS_TR_VEC_MAX 4096
#define WIFI_HTTPS_WS_RV_VEC_MAX 128
#define UART_VEC_MAX 128

#define TRCV_BUF_MAX_CAPACITY 10
#define WIFI_HTTPS_TRCV_BUF_CAP 10
#define UART_TRCV_BUF_CAP 10

#define UART_START_CODE  ((uint8_t) '>')
#define UART_END_CODE    ((uint8_t) '\n')

// #define DISABLE_FDCAN
// #define DISABLE_UART
// #define DISABLE_UART_TRSM
// #define DISABLE_UART_RECV

// configMAX_PRIORITIES = 25

#define HTTPS_TASK_PRIO_SEQU            19
#define HTTPS_DATA_TASK_PRIO_SEQU       14
#define UART_READ_TASK_PRIO_SEQU        20
#define UART_DATA_TASK_PRIO_SEQU        15
