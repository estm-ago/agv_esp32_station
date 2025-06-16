#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "packet.h"

void uart_tr_pkt_proc(void);
void uart_re_pkt_proc(uint8_t count);

