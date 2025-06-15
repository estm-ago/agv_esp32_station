#pragma once

#include "freertos/FreeRTOSConfig.h"

#define UART_READ_TASK_PRIO_SEQU            configMAX_PRIORITIES-1
#define WIFI_UDP_READ_TASK_PRIO_SEQU        configMAX_PRIORITIES-2
#define WIFI_TCP_READ_TASK_PRIO_SEQU        configMAX_PRIORITIES-3
#define UART_WRITE_TASK_PRIO_SEQU           configMAX_PRIORITIES-4
#define WIFI_UDP_WRITE_TASK_PRIO_SEQU       configMAX_PRIORITIES-5
#define WIFI_TCP_WRITE_TASK_PRIO_SEQU       configMAX_PRIORITIES-6

