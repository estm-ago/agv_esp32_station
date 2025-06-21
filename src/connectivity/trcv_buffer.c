#include "connectivity/trcv_buffer.h"

FnState uart_trcv_buf_setup(ByteTrcvBuf* self, size_t buf_size, size_t data_size)
{
    self->head = 0;
    self->len = 0;
    self->cap = buf_size;
    for (size_t i = 0; i < buf_size; i++)
    {
        FNS_ERROR_CHECK(vec_byte_new(&self->vecs[i], data_size));
    }
    return FNS_OK;
}

FnState uart_trcv_buf_push(ByteTrcvBuf* self, const VecByte* vec_u8)
{
    if (self->len >= self->cap) return FNS_BUF_OVERFLOW;
    size_t tail = (self->head + self->len) % self->cap;
    vec_rm_all(&self->vecs[tail]);
    FNS_ERROR_CHECK(vec_byte_push(&self->vecs[tail], vec_u8->data, vec_u8->len));
    self->len++;
    return FNS_OK;
}

FnState uart_trcv_buf_pop(ByteTrcvBuf* self, VecByte* vec_u8)
{
    if (self->len == 0) return FNS_BUF_EMPTY;
    FNS_ERROR_CHECK(vec_byte_push(vec_u8, self->vecs[self->head].data, self->vecs[self->head].len));
    if (--self->len == 0)
    {
        self->head = 0;
    }
    else
    {
        self->head = (self->head + 1) % self->cap;
    }
    return FNS_OK;
}
