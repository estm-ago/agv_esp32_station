#include "vec.h"
#include <stdlib.h>
#include <string.h>
#include "config.h"

inline FnState vec_rm_all(VecByte *self)
{
    self->head = 0;
    self->len  = 0;
    return FNS_OK;
}

FnState vec_rm_range(VecByte *self, size_t offset, size_t size)
{
    if (offset >= self->len) return FNS_FAIL;
    if (size == 0) return FNS_OK;
    if (size >= self->len) return vec_rm_all(self);
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

FnState vec_byte_new(VecByte *self, size_t cap)
{
    if (cap == 0 || cap > VEC_BYTE_MAX_CAP) return FNS_FAIL;
    self->data = malloc(cap * sizeof(*self->data));
    if (!self->data) return FNS_ERR_OOM;
    self->cap = cap;
    self->head     = 0;
    self->len      = 0;
    return FNS_OK;
}

FnState vec_byte_free(VecByte *self)
{
    vec_rm_all(self);
    if (self->data) {
        free(self->data);
        self->data = NULL;
    }
    self->cap = 0;
    return FNS_OK;
}

FnState vec_byte_realign(VecByte *self)
{
    if (self->len == 0 || self->head == 0) return FNS_OK;
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
    return FNS_OK;
}

FnState vec_byte_get_byte(const VecByte *self, uint8_t *u8, size_t id)
{
    if (self->len == 0) return FNS_BUF_EMPTY;
    if (id >= self->len) return FNS_FAIL;
    size_t idx = (self->head + id) % self->cap;
    *u8 = self->data[idx];
    return FNS_OK;
}

FnState vec_byte_starts_with(const VecByte *self, const uint8_t *pre, size_t pre_len)
{
    if (self->len < pre_len) return FNS_NO_MATCH;
    if (
        (self->head + pre_len <= self->cap) &&
        (memcmp(self->data + self->head, pre, pre_len) == 0)
    ) return FNS_OK;
    size_t first_part  = self->cap - self->head;
    size_t remaining = pre_len - first_part;
    if (memcmp(self->data + self->head, pre, first_part) != 0) return FNS_NO_MATCH;
    if (memcmp(self->data, pre + first_part, remaining) != 0) return FNS_NO_MATCH;
    return FNS_OK;
}

FnState vec_byte_push(VecByte *self, const void *src, size_t src_len)
{
    if (self->len + src_len > self->cap) return FNS_BUF_OVERFLOW;
    size_t tail = self->head + self->len;
    if (
        (tail >= self->cap) ||
        (tail + src_len >= self->cap)
    )
    {
        vec_byte_realign(self);
        tail = self->len;
    }
    memcpy(self->data + tail, src, src_len);
    self->len += src_len;
    return FNS_OK;
}

inline FnState vec_byte_push_byte(VecByte *self, uint8_t value)
{
    return vec_byte_push(self, &value, 1);
}

/**
 * @brief 交換 16-bit 整數的大小端
 *
 * @param value 要交換大小端的 16-bit 值
 * 
 * @return uint16_t 交換後的 16-bit 值
 */
static inline uint16_t swap16(const uint16_t value)
{
    return  ((value & 0x00FFU) << 8) |
            ((value & 0xFF00U) >> 8);
}

FnState vec_byte_push_u16(VecByte *self, uint16_t value)
{
    uint16_t u16 = swap16(value);
    return vec_byte_push(self, &u16, sizeof(u16));
}

/**
 * @brief 交換 32-bit 整數的大小端
 *
 * @param value 要交換大小端的 32-bit 值
 * 
 * @return uint32_t 交換後的 32-bit 值
 */
static inline uint32_t swap32(uint32_t value)
{
    return  ((value & 0x000000FFU) << 24) |
            ((value & 0x0000FF00U) <<  8) | 
            ((value & 0x00FF0000U) >>  8) | 
            ((value & 0xFF000000U) >> 24);
}

FnState vec_byte_push_f32(VecByte *self, float value)
{
    uint32_t u32;
    uint8_t u32_len = sizeof(u32);
    memcpy(&u32, &value, u32_len);
    u32 = swap32(u32);
    return vec_byte_push(self, &u32, u32_len);
}
