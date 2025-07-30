#pragma once

#include "main/config.h"
#include "main/fn_state.h"
#include "main/vec.h"

#define FILE_HEADER_LEN 14
typedef struct FileHeader
{
    /**
     * 資料型態 ex. 32/8=4
     */
    uint8_t type;
    /**
     * 最大資料個數
     */
    uint32_t cap;
    /**
     * 當前資料個數
     */
    uint32_t count;
    /**
     * 當前第一個資料
     */
    uint32_t head;
} FileHeader;

Result file_new(const char* path, const FileHeader* file_h);
Result file_data_get(const char* path, uint32_t count, VecByte* vec_byte);
Result file_data_add(const char* path, VecByte* vec_byte);
