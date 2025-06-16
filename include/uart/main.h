#pragma once

#include <stdbool.h>

typedef struct TransceiveFlags {
    bool uart_transmit;
    bool uart_tr_pkt_proc;
    bool uart_re_pkt_proc;
    bool right_speed;
    bool right_adc;
} TransceiveFlags;

void uart_setup(void);
