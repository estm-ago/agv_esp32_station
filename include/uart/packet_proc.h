#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "packet.h"

void uart_transmit_pkt_proc(void);
void uart_receive_pkt_proc(uint8_t count);

