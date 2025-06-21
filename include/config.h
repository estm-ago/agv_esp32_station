/*
#include "config.h"
*/
#pragma once

#include <stdbool.h>

#define BOARD_LED_TOGGLE HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5)

#define VECU8_MAX_CAPACITY 256

#define WIFI_HTTPS_WS_VEC_MAX 256
#define UART_VEC_MAX 128
#define UART_TRCV_BUF_CAP 10
#define UART_START_CODE  ((uint8_t) '>')
#define UART_END_CODE    ((uint8_t) '\n')

// #define DISABLE_FDCAN
// #define DISABLE_UART
// #define DISABLE_UART_TRSM
// #define DISABLE_UART_RECV

typedef struct{
    bool enable_PI;
    bool enable_adc;
    bool enable_search_magnetic_path;
    bool enable_timeout_error;

    bool enable_debug_breakdown_all_hall_lost;
    bool enable_debug_test_no_load_speed;
} SYSTEM_RUNTIME_SWITCH;
