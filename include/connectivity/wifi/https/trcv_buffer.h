#pragma once

#include <esp_https_server.h>
#include "connectivity/trcv_buffer.h"

typedef struct WSByteTrcvBuf
{
    ByteTrcvBuf     trcv_buf;
    int*            sockfds;
} WSByteTrcvBuf;

Result https_trcv_buf_setup(WSByteTrcvBuf* self, size_t buf_size, size_t data_size);
Result https_trcv_buf_push(WSByteTrcvBuf* self, VecByte* vec_byte, int sockfd);
Result https_trcv_buf_pop(WSByteTrcvBuf* self, VecByte* vec_byte, int* sockfd);
