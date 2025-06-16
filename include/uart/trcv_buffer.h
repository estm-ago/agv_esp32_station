#pragma once

#include "uart/packet.h"

#define UART_TRCV_BUF_CAP 5
typedef struct UartTrcvBuf UartTrcvBuf;
typedef struct UartTrcvBuf {
    UartPacket  packets[UART_TRCV_BUF_CAP];
    uint8_t     head;
    uint8_t     len;
} UartTrcvBuf;
bool uart_trcv_buf_push(UartTrcvBuf *self, const UartPacket *pkt);
bool uart_trcv_buf_pop(UartTrcvBuf *self, UartPacket *pkt);
