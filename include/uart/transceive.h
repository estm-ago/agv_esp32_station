#pragma once

#include <stdbool.h>

typedef struct {
    bool uart_transmit;
    bool uart_transmit_pkt_proc;
    bool uart_receive_pkt_proc;
    bool right_speed;
    bool right_adc;
} TransceiveFlags;
extern TransceiveFlags transceive_flags;

void uart_setup(void);

