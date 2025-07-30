#pragma once

#include "main/config.h"
#include "main/fn_state.h"

typedef struct VecByte
{
    uint8_t*    data;
    size_t      cap;
    size_t      head;
    size_t      len;
} VecByte;
void vec_rm_all(VecByte* self);
Result vec_rm_range(VecByte* self, size_t offset, size_t size);
Result vec_byte_new(VecByte* self, size_t cap);
void vec_byte_free(VecByte* self);
void vec_byte_realign(VecByte* self);
Result vec_byte_get_byte(const VecByte* self, size_t id, uint8_t *value);
Result vec_byte_pop_byte(VecByte* self, size_t id, uint8_t* value);
Result vec_byte_starts_with(VecByte* self, const uint8_t *pre, size_t pre_len);
Result vec_byte_add_len(VecByte* self, size_t len);
Result vec_byte_push(VecByte* self, const void *src, size_t src_len);
Result vec_byte_push_byte(VecByte* self, uint8_t value);
uint16_t swap_u16(uint16_t value);
Result vec_byte_push_u16(VecByte* self, uint16_t value);
uint32_t swap_u32(uint32_t value);
Result vec_byte_get_u32(VecByte* self, size_t id, uint32_t* value);
Result vec_byte_pop_u32(VecByte* self, size_t id, uint32_t* value);
Result vec_byte_push_u32(VecByte* self, uint32_t value);
Result vec_byte_push_f32(VecByte* self, float value);
Result vec_byte_pop_can(VecByte* self, VecByte* container);
