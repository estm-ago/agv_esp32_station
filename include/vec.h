#pragma once
// ----------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "fn_state.h"
// ----------------------------------------------------------------------------------------------------

#define VECU8_MAX_CAPACITY  255

typedef struct VecU8 {
    uint8_t         data[VECU8_MAX_CAPACITY];
    uint16_t        head;
    uint16_t        len;
} VecU8;
#define VEC_U8_NEW() ((VecU8){0})
FnState vec_u8_realign(VecU8 *self);
FnState vec_u8_get_byte(const VecU8 *self, uint8_t *u8, uint16_t id);
FnState vec_u8_starts_with(const VecU8 *self, const uint8_t *pre, uint16_t pre_len);
FnState vec_u8_push(VecU8 *self, const void *src, uint16_t src_len);
FnState vec_u8_push_byte(VecU8 *self, uint8_t value);
FnState vec_u8_push_u16(VecU8 *self, uint16_t value);
FnState vec_u8_push_f32(VecU8 *self, float value);
FnState vec_u8_rm_all(VecU8 *self);
FnState vec_u8_rm_range(VecU8 *self, uint16_t offset, uint16_t size);

