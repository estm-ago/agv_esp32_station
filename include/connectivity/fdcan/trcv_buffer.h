#pragma once

#include <stdint.h>
#include "connectivity/trcv_buffer.h"

typedef struct FdcanByteTrcvBuf
{
    ByteTrcvBuf     trcv_buf;
    uint32_t*       id;
} FdcanByteTrcvBuf;

FnState fdcan_trcv_buf_setup(FdcanByteTrcvBuf* self, size_t buf_size, size_t data_size);
FnState fdcan_trcv_buf_push(FdcanByteTrcvBuf* self, VecByte* vec_byte, uint32_t id);
FnState fdcan_trcv_buf_pop(FdcanByteTrcvBuf* self, VecByte* vec_byte, uint32_t* id);
