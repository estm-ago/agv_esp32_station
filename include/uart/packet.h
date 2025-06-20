#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vec.h"

#define PACKET_START_CODE  ((uint8_t) '>')
#define PACKET_END_CODE    ((uint8_t) '\n')

#define PACKET_MAX_SIZE VECU8_MAX_CAPACITY
#define PACKET_DATA_MAX_SIZE (VECU8_MAX_CAPACITY - 2)

typedef struct UartPacket
{
    uint8_t     start;
    Vec_U8       vec;
    uint8_t     end;
} UartPacket;
#define UART_PKT_NEW()          \
{                               \
    .start = PACKET_START_CODE, \
    .end   = PACKET_END_CODE    \
}
FnState uart_pkt_add_data(UartPacket *self, Vec_U8 *vec_u8);
FnState uart_pkt_get_data(const UartPacket *self, Vec_U8 *vec_u8);
FnState uart_pkt_pack(UartPacket *self, Vec_U8 *vec_u8);
FnState uart_pkt_unpack(const UartPacket *self, Vec_U8 *vec_u8);
