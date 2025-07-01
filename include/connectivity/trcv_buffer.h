#pragma once

#include "main/config.h"
#include "main/vec.h"

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
 * @param vec_byte 推入的封包
 * 
 * @return FNS_OK 推入成功
 * @return FNS_BUF_OVERFLOW 推入失敗（超過容量）
 */
FnState connect_trcv_buf_push(ByteTrcvBuf* self, VecByte* vec_byte);
/**
 * @brief 從環形緩衝區彈出一個封包
 *
 * @param self 指向 ByteTrcvBuf 的指標
 * @param vec_byte 接收彈出的封包
 * 
 * @return FNS_OK 彈出成功
 * @return FNS_BUF_EMPTY 彈出失敗（緩衝區為空）
 */
FnState connect_trcv_buf_pop(ByteTrcvBuf* self, VecByte* vec_byte);

#define ERROR_CHECK_FNS_WRI_PUSH(expr1, expr2, cleanup) \
    do {                                                \
        FnState _err = (expr1);                         \
        if (_err == FNS_OK)                             \
        {                                               \
            _err = (expr2);                             \
            if (_err != FNS_OK)                         \
            {                                           \
                cleanup;                                \
                return _err;                            \
            }                                           \
        }                                               \
    } while (0)
