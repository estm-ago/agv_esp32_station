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

extern FileData stg_wheel_left_speed;
extern FileData stg_wheel_left_duty;
extern FileData stg_wheel_right_speed;
extern FileData stg_wheel_right_duty;
extern FileData stg_arm_bottom_angle;
extern FileData stg_arm_shoulder_angle;
extern FileData stg_arm_elbow_btm_angle;
extern FileData stg_arm_elbow_top_angle;
extern FileData stg_arm_wrist_angle;
extern FileData stg_arm_finger_angle;

Result storage_setup(void);
Result storage_store_data(FileData* file_data);
void storage_loop(void);
