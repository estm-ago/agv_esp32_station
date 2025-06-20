#pragma once

#include <stdint.h>
#include "fn_state.h"
#include "uart/packet.h"

#define UART_TRCV_BUF_CAP 10
typedef struct UartTrcvBuf
{
    UartPacket  packets[UART_TRCV_BUF_CAP];
    uint8_t     head;
    uint8_t     len;
} UartTrcvBuf;
FnState uart_trcv_buf_push(UartTrcvBuf *self, const UartPacket *pkt);
FnState uart_trcv_buf_pop(UartTrcvBuf *self, UartPacket *pkt);
