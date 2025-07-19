#include "main/vec.h"

void vec_rm_all(VecByte* self)
{
    if (self->data == NULL) return;
    self->head = 0;
    self->len  = 0;
    return;
}

FnState vec_rm_range(VecByte* self, size_t offset, size_t size)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    if (offset >= self->len) return FNS_FAIL;
    if (size == 0) return FNS_OK;
    if (size >= self->len)
    {
        vec_rm_all(self);
        return FNS_OK;
    }
    if (offset == 0)
    {
        self->head = (self->head + size) % self->cap;
        self->len -= size;
        return FNS_OK;
    }
    if (offset + size >= self->len)
    {
        self->len = offset;
        return FNS_OK;
    }
    vec_byte_realign(self);
    memmove(self->data + offset, self->data + (offset + size), self->len - (offset + size));
    self->len -= size;
    return FNS_OK;
}

FnState vec_byte_new(VecByte* self, size_t cap)
{
    if (cap == 0 || cap > VEC_BYTE_MAX_CAP) return FNS_FAIL;
    self->data = malloc(cap * sizeof(*self->data));
    if (!self->data) return FNS_ERR_OOM;
    self->cap = cap;
    self->head     = 0;
    self->len      = 0;
    return FNS_OK;
}

void vec_byte_free(VecByte* self)
{
    if (self->data == NULL) return;
    vec_rm_all(self);
    if (self->data) {
        free(self->data);
        self->data = NULL;
    }
    self->cap = 0;
    return;
}

void vec_byte_realign(VecByte* self)
{
    if (self->data == NULL) return;
    if (self->len == 0 || self->head == 0) return;
    size_t first_part = self->cap - self->head;
    if (first_part >= self->len)
    {
        memmove(self->data, self->data + self->head, self->len);
    }
    else
    {
        size_t second_part = self->len - first_part;
        uint8_t tmp[second_part];
        memcpy(tmp, self->data, second_part);
        memmove(self->data, self->data + self->head, first_part);
        memmove(self->data + first_part, tmp, second_part);
    }
    self->head = 0;
    return;
}

FnState vec_byte_starts_with(const VecByte* self, const uint8_t *pre, size_t pre_len)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    if (self->len < pre_len) return FNS_NOT_FOUND;
    if (
           (self->head + pre_len <= self->cap)
        && (memcmp(self->data + self->head, pre, pre_len) == 0)
    ) return FNS_OK;
    size_t first_part  = self->cap - self->head;
    size_t remaining = pre_len - first_part;
    if (memcmp(self->data + self->head, pre, first_part) != 0) return FNS_NOT_FOUND;
    if (memcmp(self->data, pre + first_part, remaining) != 0) return FNS_NOT_FOUND;
    return FNS_OK;
}

FnState vec_byte_add_len(VecByte* self, size_t len)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    if (len > self->cap) return FNS_OVERFLOW;
    self->len += len;
    return FNS_OK;
}

FnState vec_byte_push(VecByte* self, const void *src, size_t src_len)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    if (self->len + src_len > self->cap) return FNS_OVERFLOW;
    size_t tail = self->head + self->len;
    if (
           (tail >= self->cap)
        || (tail + src_len >= self->cap)
    )
    {
        vec_byte_realign(self);
        tail = self->len;
    }
    memcpy(self->data + tail, src, src_len);
    self->len += src_len;
    return FNS_OK;
}

FnState vec_byte_pop_can(VecByte* self, VecByte* container)
{
    size_t len = self->len;
    if (self->len > FDCAN_VEC_BYTE_CAP) len = FDCAN_VEC_BYTE_CAP;
    vec_byte_realign(self);
    FnState result;
    ERROR_CHECK_FNS_CLEANUP(vec_byte_push(container, self->data, len));
    ERROR_CHECK_FNS_CLEANUP(vec_rm_range(self, 0, FDCAN_VEC_BYTE_CAP));
    cleanup:
    return result;
}

FnState vec_byte_get_byte(const VecByte* self, size_t id, uint8_t *value)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    if (self->len < id + sizeof(*value)) return FNS_BUF_NOT_ENOU;
    if (id >= self->len) return FNS_FAIL;
    *value = self->data[(self->head + id) % self->cap];
    return FNS_OK;
}

FnState vec_byte_pop_byte(VecByte* self, size_t id, uint8_t* value)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    ERROR_CHECK_FNS_RETURN(vec_byte_get_byte(self, id, value));
    ERROR_CHECK_FNS_RETURN(vec_rm_range(self, id, sizeof(*value)));
    return FNS_OK;
}

FnState vec_byte_push_byte(VecByte* self, uint8_t value)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    return vec_byte_push(self, &value, 1);
}

uint16_t swap_u16(const uint16_t value)
{
    return    ((value & 0x00FFU) << 8)
            | ((value & 0xFF00U) >> 8);
}

FnState vec_byte_push_u16(VecByte* self, uint16_t value)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    uint16_t val = swap_u16(value);
    return vec_byte_push(self, &val, sizeof(value));
}

uint32_t swap_u32(uint32_t value)
{
    return    ((value & 0x000000FFU) << 24)
            | ((value & 0x0000FF00U) <<  8)
            | ((value & 0x00FF0000U) >>  8)
            | ((value & 0xFF000000U) >> 24);
}

FnState vec_byte_get_u32(VecByte* self, size_t id, uint32_t* value)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    if (self->len < id + sizeof(*value)) return FNS_BUF_NOT_ENOU;
    if (id >= self->len) return FNS_FAIL;
    size_t head = self->head + id;
    *value =  ((uint32_t)self->data[ head      % self->cap] << 24)
            | ((uint32_t)self->data[(head + 1) % self->cap] << 16)
            | ((uint32_t)self->data[(head + 2) % self->cap] <<  8)
            | ((uint32_t)self->data[(head + 3) % self->cap]      );
    return FNS_OK;
}

FnState vec_byte_pop_u32(VecByte* self, size_t id, uint32_t* value)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    ERROR_CHECK_FNS_RETURN(vec_byte_get_u32(self, id, value));
    ERROR_CHECK_FNS_RETURN(vec_rm_range(self, id, sizeof(*value)));
    return FNS_OK;
}

FnState vec_byte_push_u32(VecByte* self, uint32_t value)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    uint32_t val = swap_u32(value);
    return vec_byte_push(self, &val, sizeof(value));
}

FnState vec_byte_push_f32(VecByte* self, float value)
{
    if (self->data == NULL) return FNS_ERR_OOM;
    uint32_t u32;
    memcpy(&u32, &value, sizeof(u32));
    u32 = swap_u32(u32);
    return vec_byte_push(self, &u32, sizeof(u32));
}
