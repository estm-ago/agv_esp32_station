#pragma once

#include <stddef.h>
#include <stdint.h>
#include "fn_state.h"
#include "config.h"
#include "vec.h"

typedef struct ByteTrcvBuf
{
    VecByte*    vecs;
    size_t      cap;
    size_t      head;
    size_t      len;
} ByteTrcvBuf;

FnState connect_trcv_buf_setup(ByteTrcvBuf* self, size_t buf_size, size_t data_size);
/**
 * @brief 將封包推入環形緩衝區
 * 
 * @param self 指向 ByteTrcvBuf 的指標
 * @param vec_u8 推入的封包
 * 
 * @return FNS_OK 推入成功
 * @return FNS_BUF_OVERFLOW 推入失敗（超過容量）
 */
FnState connect_trcv_buf_push(ByteTrcvBuf* self, VecByte* vec_u8);
/**
 * @brief 從環形緩衝區彈出一個封包
 *
 * @param self 指向 ByteTrcvBuf 的指標
 * @param vec_u8 接收彈出的封包
 * 
 * @return FNS_OK 彈出成功
 * @return FNS_BUF_EMPTY 彈出失敗（緩衝區為空）
 */
FnState connect_trcv_buf_pop(ByteTrcvBuf* self, VecByte* vec_u8);
