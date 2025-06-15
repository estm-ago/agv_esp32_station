#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "vec_mod.h"

#define PACKET_START_CODE  ((uint8_t) '{')
#define PACKET_END_CODE    ((uint8_t) '}')

#define PACKET_MAX_SIZE VECU8_MAX_CAPACITY
#define PACKET_DATA_MAX_SIZE (VECU8_MAX_CAPACITY - 2)

typedef struct UartPacket UartPacket;
typedef struct UartPacket {
    uint8_t     start;
    VecU8       datas;
    uint8_t     end;
} UartPacket;
bool uart_pkt_add_data(UartPacket *self, VecU8 *vec_u8);
bool uart_pkt_get_data(const UartPacket *self, VecU8 *vec_u8);
bool uart_pkt_pack(UartPacket *self, VecU8 *vec_u8);
bool uart_pkt_unpack(const UartPacket *self, VecU8 *vec_u8);
UartPacket uart_packet_new(void);
