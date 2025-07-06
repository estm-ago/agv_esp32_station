#pragma once

#include "main/config.h"
#include "main/fn_state.h"
#include "main/vec.h"
#include "storage/sdcard/main.h"

typedef struct FileData {
    const char* path;
    uint8_t type;
    VecByte buffer;
} FileData;

extern FileData stg_m_left_speed;
extern FileData stg_m_left_duty;
extern FileData stg_m_right_speed;
extern FileData stg_m_right_duty;

FnState storage_setup(void);
FnState store_data(FileData* file_data);
void storage_loop(void);
