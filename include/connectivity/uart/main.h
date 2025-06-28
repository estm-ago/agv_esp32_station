#pragma once

#include <stdbool.h>

#define STM32_UART      UART_NUM_2
#define STM32_UART_TXD  GPIO_NUM_17
#define STM32_UART_RXD  GPIO_NUM_16

typedef struct TransceiveFlags
{
    bool uart_transmit;
    bool uart_tr_pkt_proc;
    bool uart_re_pkt_proc;
    bool right_speed;
    bool right_adc;
} TransceiveFlags;

void uart_setup(void);
