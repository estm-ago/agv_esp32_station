#pragma once

#include <stdbool.h>

typedef struct TransceiveFlags {
    bool uart_transmit;
    bool uart_transmit_pkt_proc;
    bool uart_receive_pkt_proc;
    bool right_speed;
    bool right_adc;
} TransceiveFlags;

void uart_setup(void);
