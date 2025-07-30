#include "main/vec.h"

/**
 * @brief 從 Vec 中移除全部資料
 * 
 * @param self   指向 Vec 實例的指標
 */
void vec_rm_all(VecByte* self)
{
    if (self->data == NULL) return;
    self->head = 0;
    self->len  = 0;
    return;
}

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
Result vec_rm_range(VecByte* self, size_t offset, size_t size)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    if (offset >= self->len) return RESULT_ERROR(RES_ERR_FAIL);
    if (size == 0) return RESULT_OK(self);
    if (size >= self->len)
    {
        vec_rm_all(self);
        return RESULT_OK(self);
    }
    if (offset == 0)
    {
        self->head = (self->head + size) % self->cap;
        self->len -= size;
        return RESULT_OK(self);
    }
    if (offset + size >= self->len)
    {
        self->len = offset;
        return RESULT_OK(self);
    }
    vec_byte_realign(self);
    memmove(self->data + offset, self->data + (offset + size), self->len - (offset + size));
    self->len -= size;
    return RESULT_OK(self);
}

/**
 * @brief 初始化 VecByte，動態配置緩衝區
 * 
 * @param self 指向 VecByte 實例的指標
 * @param cap 要配置的容量（bytes），須大於 0
 * 
 * @return FNS_OK    成功
 * @return RESULT_ERROR(RES_ERR_MEMORY_ERROR); 配置失敗（記憶體不足）
 * @return FNS_FAIL  輸入參數錯誤（如 cap==0）
 */
Result vec_byte_new(VecByte* self, size_t cap)
{
    if (cap == 0 || cap > VEC_BYTE_MAX_CAP) return RESULT_ERROR(RES_ERR_FAIL);
    self->data = malloc(cap * sizeof(*self->data));
    if (!self->data) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    self->cap = cap;
    self->head     = 0;
    self->len      = 0;
    return RESULT_OK(self);
}

/**
 * @brief 釋放 VecByte 佔用的資源，並重置欄位
 * 
 * @param self 指向 VecByte 實例的指標
 */
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

/**
 * @brief 把 VecByte 裡的資料「搬到索引 0 開始」(head = 0)，並保留原本的儲存順序
 *
 * @param self 指向 VecByte 實例的指標
 */
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
Result vec_byte_starts_with(VecByte* self, const uint8_t *pre, size_t pre_len)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    if (self->len < pre_len) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    vec_byte_realign(self);
    if (memcmp(self->data, pre, pre_len) != 0) return RESULT_ERROR(RES_ERR_NOT_FOUND);
    return RESULT_OK(NULL);
}

/**
 * @brief 直接增加 VecByte 長度
 * 
 * @param self 指向 VecByte 實例的指標
 * @param len 增加的長度
 * 
 * @return FNS_OK 成功推入
 * @return RESULT_ERROR(RES_ERR_OVERFLOW) 推入失敗（超過容量）
 */
Result vec_byte_add_len(VecByte* self, size_t len)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    if (len > self->cap) return RESULT_ERROR(RES_ERR_OVERFLOW);
    self->len += len;
    return RESULT_OK(self);
}

/**
 * @brief 將 src 指向的位元組組合並推入 VecByte 末端
 *
 * @param self 指向 VecByte 實例的指標
 * @param src 指向要推入的資料緩衝區
 * @param src_len 要推入的資料長度
 * 
 * @return FNS_OK 成功推入
 * @return RESULT_ERROR(RES_ERR_OVERFLOW) 推入失敗（超過容量）
 */
Result vec_byte_push(VecByte* self, const void *src, size_t src_len)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    if (self->len + src_len > self->cap) return RESULT_ERROR(RES_ERR_OVERFLOW);
    size_t tail = self->head + self->len;
    if (
           (tail >= self->cap)
        || (tail + src_len >= self->cap)
    ) {
        vec_byte_realign(self);
        tail = self->len;
    }
    memcpy(self->data + tail, src, src_len);
    self->len += src_len;
    return RESULT_OK(self);
}

/**
 * @brief 從 VecU8 中，讀取相對於 head 的第 id 個位元組
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 用來存放讀出位元組的位址參考
 * @param id 欲讀取的偏移量（相對 head 的索引，範圍須在 0 ~ len-1 之間）
 *
 * @return FNS_OK 已被填入對應值  
 * @return RESULT_ERROR(RES_ERR_EMPTY); 緩衝區為空
 * @return FNS_FAIL id 超出範圍
 */
Result vec_byte_get_byte(const VecByte* self, size_t id, uint8_t *value)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    if (self->len < id + sizeof(*value)) return RESULT_ERROR(RES_ERR_OVERFLOW);
    if (id >= self->len) return RESULT_ERROR(RES_ERR_FAIL);
    *value = self->data[(self->head + id) % self->cap];
    return RESULT_OK(NULL);
}

/**
 * @brief 從 VecU8 中，彈出相對於 head 的第 id 個位元組
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 用來存放彈出位元組的位址參考
 * @param id 欲讀取的偏移量（相對 head 的索引，範圍須在 0 ~ len-1 之間）
 *
 * @return FNS_OK 已被填入對應值  
 * @return RESULT_ERROR(RES_ERR_EMPTY); 緩衝區為空
 * @return FNS_FAIL id 超出範圍
 */
Result vec_byte_pop_byte(VecByte* self, size_t id, uint8_t* value)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    RESULT_CHECK_RET_RES(vec_byte_get_byte(self, id, value));
    RESULT_CHECK_RET_RES(vec_rm_range(self, id, sizeof(*value)));
    return RESULT_OK(self);
}

/**
 * @brief 將一 byte 推入 VecByte
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 要推入的原始值
 * 
 * @return FNS_OK 成功推入
 * @return RESULT_ERROR(RES_ERR_OVERFLOW) 推入失敗（超過容量）
 */
Result vec_byte_push_byte(VecByte* self, uint8_t value)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    return vec_byte_push(self, &value, 1);
}

/**
 * @brief 交換 16-bit 整數的大小端
 *
 * @param value 要交換大小端的 16-bit 值
 * 
 * @return uint16_t 交換後的 16-bit 值
 */
uint16_t swap_u16(uint16_t value)
{
    return    ((value & 0x00FFU) << 8)
            | ((value & 0xFF00U) >> 8);
}

/**
 * @brief 將原始值轉換為 IEEE-754 大端序並推入 VecByte
 * 
 * @param self 指向 VecByte 實例的指標
 * @param value 要推入的原始值
 * 
 * @return FNS_OK 成功推入
 * @return RESULT_ERROR(RES_ERR_OVERFLOW) 推入失敗（超過容量）
 */
Result vec_byte_push_u16(VecByte* self, uint16_t value)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    uint16_t val = swap_u16(value);
    return vec_byte_push(self, &val, sizeof(value));
}

/**
 * @brief 交換 32-bit 整數的大小端
 *
 * @param value 要交換大小端的 32-bit 值
 * 
 * @return uint32_t 交換後的 32-bit 值
 */
uint32_t swap_u32(uint32_t value)
{
    return    ((value & 0x000000FFU) << 24)
            | ((value & 0x0000FF00U) <<  8)
            | ((value & 0x00FF0000U) >>  8)
            | ((value & 0xFF000000U) >> 24);
}

/**
 * @brief 從 VecU8 中，讀取相對於 head 的第 id - id+4 個位元，以 be 組成 u32
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 用來存放讀取值的位址參考
 * @param id 欲讀取的偏移量（相對 head 的索引，範圍須在 0 ~ len-1 之間）
 *
 * @return FNS_OK 已被填入對應值  
 * @return RESULT_ERROR(RES_ERR_EMPTY); 緩衝區為空
 * @return FNS_FAIL id 超出範圍
 */
Result vec_byte_get_u32(VecByte* self, size_t id, uint32_t* value)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    if (self->len < id + sizeof(*value)) return RESULT_ERROR(RES_ERR_OVERFLOW);
    if (id >= self->len) return RESULT_ERROR(RES_ERR_FAIL);
    size_t head = self->head + id;
    *value =  ((uint32_t)self->data[ head      % self->cap] << 24)
            | ((uint32_t)self->data[(head + 1) % self->cap] << 16)
            | ((uint32_t)self->data[(head + 2) % self->cap] <<  8)
            | ((uint32_t)self->data[(head + 3) % self->cap]      );
    return RESULT_OK(self);
}

/**
 * @brief 從 VecU8 中，彈出相對於 head 的第 id - id+4 個位元，以 be 組成 u32
 *
 * @param self 指向 VecByte 實例的指標
 * @param value 用來存放讀出彈出值的位址參考
 * @param id 欲讀取的偏移量（相對 head 的索引，範圍須在 0 ~ len-1 之間）
 *
 * @return FNS_OK 已被填入對應值  
 * @return RESULT_ERROR(RES_ERR_EMPTY); 緩衝區為空
 * @return FNS_FAIL id 超出範圍
 */
Result vec_byte_pop_u32(VecByte* self, size_t id, uint32_t* value)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    RESULT_CHECK_RET_RES(vec_byte_get_u32(self, id, value));
    RESULT_CHECK_RET_RES(vec_rm_range(self, id, sizeof(*value)));
    return RESULT_OK(self);
}

/**
 * @brief 將原始值轉換為 IEEE-754 大端序並推入 VecByte
 * 
 * @param self 指向 VecByte 實例的指標
 * @param value 要推入的原始值
 * 
 * @return FNS_OK 成功推入
 * @return RESULT_ERROR(RES_ERR_OVERFLOW) 推入失敗（超過容量）
 */
Result vec_byte_push_u32(VecByte* self, uint32_t value)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    uint32_t val = swap_u32(value);
    return vec_byte_push(self, &val, sizeof(value));
}

/**
 * @brief 將原始值轉換為 IEEE-754 大端序並推入 VecByte
 * 
 * @param self 指向 VecByte 實例的指標
 * @param value 要推入的原始值
 * 
 * @return FNS_OK 成功推入
 * @return RESULT_ERROR(RES_ERR_OVERFLOW) 推入失敗（超過容量）
 */
Result vec_byte_push_f32(VecByte* self, float value)
{
    if (self->data == NULL) return RESULT_ERROR(RES_ERR_MEMORY_ERROR);
    uint32_t u32;
    memcpy(&u32, &value, sizeof(u32));
    u32 = swap_u32(u32);
    return vec_byte_push(self, &u32, sizeof(u32));
}

Result vec_byte_pop_can(VecByte* self, VecByte* container)
{
    if (self->len < FDCAN_VEC_BYTE_CAP) return RESULT_ERROR(RES_ERR_EMPTY);
    if (self->head + self->len >= self->cap) vec_byte_realign(self);
    Result result = vec_byte_push(container, self->data + self->head, FDCAN_VEC_BYTE_CAP);
    if (!RESULT_CHECK_RAW(result)) self->len -= FDCAN_VEC_BYTE_CAP;
    return result;
}
