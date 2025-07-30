#include "connectivity/wifi/https/trcv_buffer.h"

Result https_trcv_buf_setup(WSByteTrcvBuf* self, size_t buf_size, size_t data_size)
{
    if (buf_size > TRCV_BUF_MAX_CAP) return RESULT_ERROR(RES_ERR_FAIL);
    self->sockfds = malloc(buf_size * sizeof(uint8_t));
    if (self->sockfds == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);;
    return connect_trcv_buf_setup(&self->trcv_buf, buf_size, data_size);
}

Result https_trcv_buf_push(WSByteTrcvBuf* self, VecByte* vec_byte, int sockfd)
{
    size_t tail = (self->trcv_buf.head + self->trcv_buf.len) % self->trcv_buf.cap;
    self->sockfds[tail] = sockfd;
    return connect_trcv_buf_push(&self->trcv_buf, vec_byte);
}

Result https_trcv_buf_pop(WSByteTrcvBuf* self, VecByte* vec_byte, int* sockfd)
{
    if (self->trcv_buf.len == 0) return RESULT_ERROR(RES_ERR_EMPTY);;
    *sockfd = self->sockfds[self->trcv_buf.head];
    return connect_trcv_buf_pop(&self->trcv_buf, vec_byte);
}
