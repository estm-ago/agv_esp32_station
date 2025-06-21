#pragma once

#include <esp_https_server.h>
#include "connectivity/trcv_buffer.h"

typedef struct WSByteTrcvBuf
{
    ByteTrcvBuf     trcv_buf;
    int*            sockfds;
} WSByteTrcvBuf;

FnState https_trcv_buf_setup(WSByteTrcvBuf* self, size_t buf_size, size_t data_size);
FnState https_trcv_buf_push(WSByteTrcvBuf* self, const VecByte* vec_u8, const int sockfd);
FnState https_trcv_buf_pop(WSByteTrcvBuf* self, VecByte* vec_u8, int* sockfd);
