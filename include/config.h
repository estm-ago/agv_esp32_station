/*
#include "config.h"
*/
#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <esp_log.h>
#include <esp_event.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"

#define UNUSED_FNC __attribute__((unused))

// ! SYSTEM config, Change CAREFULLY --------------------

#define VEC_BYTE_MAX_CAP 4096
#define TRCV_BUF_MAX_CAP 10

#define WIFI_HTTPS_WS_TR_VEC_MAX 4096
#define WIFI_HTTPS_WS_RV_VEC_MAX 128
#define WIFI_HTTPS_TRCV_BUF_CAP 10

// #define DISABLE_FDCAN
#define FDCAN_VEC_BYTE_CAP  8
#define FDCAN_TRCV_BUF_CAP  10

#define UART_BAUDRATE       115200
#define UART_VEC_BYTE_CAP   128
#define UART_TRCV_BUF_CAP   10
#define UART_START_CODE     ((uint8_t) '>')
#define UART_END_CODE       ((uint8_t) '\n')

// configMAX_PRIORITIES = 25

#define DISABLE_UART
// #define DISABLE_UART_TRSM
// #define DISABLE_UART_RECV
#define HTTPS_TASK_PRIO_SEQU            19
#define HTTPS_DATA_TASK_PRIO_SEQU       14
#define UART_READ_TASK_PRIO_SEQU        20
#define UART_DATA_TASK_PRIO_SEQU        15


// ! SYSTEM config END ------------------------------
