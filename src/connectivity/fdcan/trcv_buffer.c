#include "connectivity/fdcan/trcv_buffer.h"
#include <stdlib.h>

Result fdcan_trcv_buf_setup(FdcanByteTrcvBuf* self, size_t buf_size, size_t data_size)
{
    if (buf_size > TRCV_BUF_MAX_CAP) return RESULT_ERROR(RES_ERR_FAIL);
    self->id = malloc(buf_size * sizeof(uint32_t));
    if (self->id == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);;
    return connect_trcv_buf_setup(&self->trcv_buf, buf_size, data_size);
}

Result fdcan_trcv_buf_push(FdcanByteTrcvBuf* self, VecByte* vec_byte, uint32_t id)
{
    size_t tail = (self->trcv_buf.head + self->trcv_buf.len) % self->trcv_buf.cap;
    self->id[tail] = id;
    return connect_trcv_buf_push(&self->trcv_buf, vec_byte);
}

Result fdcan_trcv_buf_pop(FdcanByteTrcvBuf* self, VecByte* vec_byte, uint32_t* id)
{
    if (self->trcv_buf.len == 0) return RESULT_ERROR(RES_ERR_EMPTY);;
    *id = self->id[self->trcv_buf.head];
    return connect_trcv_buf_pop(&self->trcv_buf, vec_byte);
}
