#include "connectivity/trcv_buffer.h"
#include <stdlib.h>

Result connect_trcv_buf_setup(ByteTrcvBuf* self, size_t buf_size, size_t data_size)
{
    if (buf_size > TRCV_BUF_MAX_CAP) return RESULT_ERROR(RES_ERR_FAIL);
    self->head = 0;
    self->len = 0;
    self->cap = buf_size;
    self->vecs = malloc(buf_size * sizeof(VecByte));
    if (self->vecs == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);;
    for (size_t i = 0; i < buf_size; i++)
    {
        RESULT_CHECK_RET_RES(vec_byte_new(&self->vecs[i], data_size));
    }
    return RESULT_OK(NULL);
}

Result connect_trcv_buf_push(ByteTrcvBuf* self, VecByte* vec_byte)
{
    if (self->len >= self->cap) return RESULT_ERROR(RES_ERR_OVERFLOW);
    size_t tail = (self->head + self->len) % self->cap;
    vec_rm_all(&self->vecs[tail]);
    vec_byte_realign(vec_byte);
    RESULT_CHECK_RET_RES(vec_byte_push(&self->vecs[tail], vec_byte->data + vec_byte->head, vec_byte->len));
    self->len++;
    return RESULT_OK(NULL);
}

Result connect_trcv_buf_pop(ByteTrcvBuf* self, VecByte* vec_byte)
{
    if (self->len == 0) return RESULT_ERROR(RES_ERR_EMPTY);;
    vec_rm_all(vec_byte);
    RESULT_CHECK_RET_RES(vec_byte_push(vec_byte, self->vecs[self->head].data, self->vecs[self->head].len));
    if (--self->len == 0)
    {
        self->head = 0;
    }
    else
    {
        self->head = (self->head + 1) % self->cap;
    }
    return RESULT_OK(NULL);
}
