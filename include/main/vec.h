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

/**
 * @brief 從 Vec 中移除全部資料
 * 
 * @param self   指向 Vec 實例的指標
 */
void vec_rm_all(VecByte* self);
/**
 * @brief 從 Vec 中移除指定範圍的資料
 * 
 * @param self   指向 Vec 實例的指標
 * @param offset 要移除區段在目前資料（以 head 為起點）的起始位移
 * @param size   要移除的長度
 * 
 * @return FNS_OK 成功移除
 * @return FNS_FAIL offset 超過目前資料長度或 realign 失敗
 */
FnState vec_rm_range(VecByte* self, size_t offset, size_t size);
/**
 * @brief 初始化 VecByte，動態配置緩衝區
 * 
 * @param self 指向 VecByte 實例的指標
 * @param cap 要配置的容量（bytes），須大於 0
 * 
 * @return FNS_OK    成功
 * @return FNS_ERR_OOM 配置失敗（記憶體不足）
 * @return FNS_FAIL  輸入參數錯誤（如 cap==0）
 */
FnState vec_byte_new(VecByte* self, size_t cap);
/**
 * @brief 釋放 VecByte 佔用的資源，並重置欄位
 * 
 * @param self 指向 VecByte 實例的指標
 */
void vec_byte_free(VecByte* self);
/**
 * @brief 把 VecByte 裡的資料「搬到索引 0 開始」(head = 0)，並保留原本的儲存順序
 *
 * @param self 指向 VecByte 實例的指標
 */
void vec_byte_realign(VecByte* self);
/**
 * @brief 從 VecU8 中，讀取相對於 head 的第 id 個位元組
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 用來存放讀出位元組的位址參考
 * @param id 欲讀取的偏移量（相對 head 的索引，範圍須在 0 ~ len-1 之間）
 *
 * @return FNS_OK 已被填入對應值  
 * @return FNS_BUF_EMPTY 緩衝區為空
 * @return FNS_FAIL id 超出範圍
 */
FnState vec_byte_get_byte(const VecByte* self, size_t id, uint8_t *value);
/**
 * @brief 從 VecU8 中，彈出相對於 head 的第 id 個位元組
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 用來存放彈出位元組的位址參考
 * @param id 欲讀取的偏移量（相對 head 的索引，範圍須在 0 ~ len-1 之間）
 *
 * @return FNS_OK 已被填入對應值  
 * @return FNS_BUF_EMPTY 緩衝區為空
 * @return FNS_FAIL id 超出範圍
 */
FnState vec_byte_pop_byte(VecByte* self, size_t id, uint8_t* value);
/**
 * @brief 檢查 VecByte 起始位置是否以指定序列開頭
 * 
 * @param self 指向 VecByte 實例的指標
 * @param pre 指向要比對的序列
 * @param pre_len 序列長度
 * 
 * @return FNS_OK 開頭吻合
 * @return false 否則 (false otherwise)
 */
FnState vec_byte_starts_with(const VecByte* self, const uint8_t *pre, size_t pre_len);
/**
 * @brief 直接增加 VecByte 長度
 * 
 * @param self 指向 VecByte 實例的指標
 * @param len 增加的長度
 * 
 * @return FNS_OK 成功推入
 * @return FNS_OVERFLOW 推入失敗（超過容量）
 */
FnState vec_byte_add_len(VecByte* self, size_t len);
/**
 * @brief 將 src 指向的位元組組合並推入 VecByte 末端
 *
 * @param self 指向 VecByte 實例的指標
 * @param src 指向要推入的資料緩衝區
 * @param src_len 要推入的資料長度
 * 
 * @return FNS_OK 成功推入
 * @return FNS_OVERFLOW 推入失敗（超過容量）
 */
FnState vec_byte_push(VecByte* self, const void *src, size_t src_len);

FnState vec_byte_pop_can(VecByte* self, VecByte* container);
/**
 * @brief 將一 byte 推入 VecByte
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 要推入的原始值
 * 
 * @return FNS_OK 成功推入
 * @return FNS_OVERFLOW 推入失敗（超過容量）
 */
FnState vec_byte_push_byte(VecByte* self, uint8_t value);
/**
 * @brief 交換 16-bit 整數的大小端
 *
 * @param value 要交換大小端的 16-bit 值
 * 
 * @return uint16_t 交換後的 16-bit 值
 */
uint16_t swap_u16(const uint16_t value);
/**
 * @brief 將原始值轉換為 IEEE-754 大端序並推入 VecByte
 * 
 * @param self 指向 VecByte 實例的指標
 * @param value 要推入的原始值
 * 
 * @return FNS_OK 成功推入
 * @return FNS_OVERFLOW 推入失敗（超過容量）
 */
FnState vec_byte_push_u16(VecByte* self, uint16_t value);
/**
 * @brief 交換 32-bit 整數的大小端
 *
 * @param value 要交換大小端的 32-bit 值
 * 
 * @return uint32_t 交換後的 32-bit 值
 */
uint32_t swap_u32(uint32_t value);
/**
 * @brief 從 VecU8 中，讀取相對於 head 的第 id - id+4 個位元，以 be 組成 u32
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 用來存放讀取值的位址參考
 * @param id 欲讀取的偏移量（相對 head 的索引，範圍須在 0 ~ len-1 之間）
 *
 * @return FNS_OK 已被填入對應值  
 * @return FNS_BUF_EMPTY 緩衝區為空
 * @return FNS_FAIL id 超出範圍
 */
FnState vec_byte_get_u32(VecByte* self, size_t id, uint32_t* value);
/**
 * @brief 從 VecU8 中，彈出相對於 head 的第 id - id+4 個位元，以 be 組成 u32
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 用來存放讀出彈出值的位址參考
 * @param id 欲讀取的偏移量（相對 head 的索引，範圍須在 0 ~ len-1 之間）
 *
 * @return FNS_OK 已被填入對應值  
 * @return FNS_BUF_EMPTY 緩衝區為空
 * @return FNS_FAIL id 超出範圍
 */
FnState vec_byte_pop_u32(VecByte* self, size_t id, uint32_t* value);
/**
 * @brief 將原始值轉換為 IEEE-754 大端序並推入 VecByte
 * 
 * @param self 指向 VecByte 實例的指標
 * @param value 要推入的原始值
 * 
 * @return FNS_OK 成功推入
 * @return FNS_OVERFLOW 推入失敗（超過容量）
 */
FnState vec_byte_push_u32(VecByte* self, uint32_t value);
/**
 * @brief 將原始值轉換為 IEEE-754 大端序並推入 VecByte
 * 
 * @param self 指向 VecByte 實例的指標
 * @param value 要推入的原始值
 * 
 * @return FNS_OK 成功推入
 * @return FNS_OVERFLOW 推入失敗（超過容量）
 */
FnState vec_byte_push_f32(VecByte* self, float value);
