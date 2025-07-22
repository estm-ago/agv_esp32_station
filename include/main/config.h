/*
#include "config.h"
*/
#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_event.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#define UNUSED_FNC __attribute__((unused))

// ! SYSTEM config, Change CAREFULLY --------------------

#define AGV_ESP32_DEVICE

#define VEC_BYTE_MAX_CAP    4096
#define TRCV_BUF_MAX_CAP    10

#define WIFI_STATIC     "10.27.246.20"
#define WIFI_NETMASK    "255.255.255.0"
#define WIFI_GW         "10.27.246.37"
#define WIFI_SSID       "HY-TPL-BF940"
#define WIFI_PSWD       "23603356"
// #define WIFI_SSID       "Vicky"
// #define WIFI_PSWD       "vicky23447"
#define CONNECT_MAXIMUM_RETRY 5

#define HTTPS_TRSM_VEC_MAX 2048
#define HTTPS_RECV_VEC_MAX 128 // Min 128
#define HTTPS_TRCV_BUF_MAX 5
#define HTTPS_RECV_BUF_MAX 20

// #define DISABLE_FDCAN
#define FDCAN_GPIO_TX       GPIO_NUM_25
#define FDCAN_GPIO_RX       GPIO_NUM_26
#define FDCAN_VEC_BYTE_CAP  8
#define FDCAN_TRCV_BUF_CAP  10

#define STM32_UART          UART_NUM_2
#define STM32_UART_TX       GPIO_NUM_17
#define STM32_UART_RX       GPIO_NUM_16
#define UART_BAUDRATE       115200
#define UART_VEC_BYTE_CAP   128
#define UART_TRCV_BUF_CAP   10
#define UART_START_CODE     ((uint8_t) '>')
#define UART_END_CODE       ((uint8_t) '\n')

#define STORAGE_BUF_MAX     30
#define STORAGE_BUF_TRI     20
#define STORAGE_GET_DATA    100
#define SD_GPIO_CS          GPIO_NUM_5
#define SD_GPIO_CLK         GPIO_NUM_18
#define SD_GPIO_MISO        GPIO_NUM_19
#define SD_GPIO_MOSI        GPIO_NUM_23
#define SD_FILE_HEAD_END    '\n'

// configMAX_PRIORITIES = 25

#define DISABLE_UART
// #define DISABLE_UART_TRSM
// #define DISABLE_UART_RECV
#define HTTPS_TASK_PRIO_SEQU            19
#define HTTPS_DATA_TASK_PRIO_SEQU       14
#define UART_READ_TASK_PRIO_SEQU        20
#define UART_DATA_TASK_PRIO_SEQU        15


// ! SYSTEM config END ------------------------------

typedef int8_t FncState;
#define FNC_CANCEL  -1
#define FNC_DISABLE 0
#define FNC_ENABLE  1

// #define ENABLE_CON_PKT_TEST
// #define DISABLE_FDCAN
#define DISABLE_UART
// #define DISABLE_UART_TRSM
// #define DISABLE_UART_RECV
