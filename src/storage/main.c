#include "storage/main.h"

static const char *TAG = "user_stg_main";

FileData stg_wheel_left_speed = {
    .path = MOUNT_POINT"/whlls.hex",
    .type = sizeof(float),
};

FileData stg_wheel_left_duty = {
    .path = MOUNT_POINT"/whlld.hex",
    .type = sizeof(uint8_t),
};

FileData stg_wheel_right_speed = {
    .path = MOUNT_POINT"/whlrs.hex",
    .type = sizeof(float),
};

FileData stg_wheel_right_duty = {
    .path = MOUNT_POINT"/whlrd.hex",
    .type = sizeof(uint8_t),
};

FileData stg_arm_bottom_angle = {
    .path = MOUNT_POINT"/armbo.hex",
    .type = sizeof(uint8_t),
};

FileData stg_arm_shoulder_angle = {
    .path = MOUNT_POINT"/armsh.hex",
    .type = sizeof(uint8_t),
};

FileData stg_arm_elbow_btm_angle = {
    .path = MOUNT_POINT"/armeb.hex",
    .type = sizeof(uint8_t),
};

FileData stg_arm_elbow_top_angle = {
    .path = MOUNT_POINT"/armet.hex",
    .type = sizeof(uint8_t),
};

FileData stg_arm_wrist_angle = {
    .path = MOUNT_POINT"/armwr.hex",
    .type = sizeof(uint8_t),
};

FileData stg_arm_finger_angle = {
    .path = MOUNT_POINT"/armfi.hex",
    .type = sizeof(uint8_t),
};

static FnState file_buf_init(FileData* file_data)
{
    FileHeader file_header = {
        .cap = 300,
        .type = file_data->type,
    };
    ERROR_CHECK_FNS_RETURN(file_new(file_data->path, &file_header));
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&file_data->buffer, file_data->type * STORAGE_BUF_MAX));
    return FNS_OK;
}

FnState storage_setup(void)
{
    sd_setup();
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_wheel_left_speed));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_wheel_left_duty));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_wheel_right_speed));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_wheel_right_duty));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_arm_bottom_angle));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_arm_shoulder_angle));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_arm_elbow_btm_angle));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_arm_elbow_top_angle));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_arm_wrist_angle));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_arm_finger_angle));
    return FNS_OK;
}

FnState storage_store_data(FileData* file_data)
{
    if (file_data->buffer.len < STORAGE_BUF_TRI) return FNS_OK;
    // ESP_LOGI(TAG, "buffer data >>>");
    // ESP_LOG_BUFFER_HEXDUMP(TAG, file_data->buffer.data, file_data->buffer.len, ESP_LOG_INFO);
    FnState err = file_data_add(file_data->path, &file_data->buffer);
    vec_rm_all(&file_data->buffer);
    // VecByte vec_byte;
    // vec_byte_new(&vec_byte, file_data->type * 20);
    // file_data_get(file_data->path, 20, &vec_byte);
    // ESP_LOGI(TAG, "file data >>>");
    // ESP_LOG_BUFFER_HEXDUMP(TAG, vec_byte.data, vec_byte.len, ESP_LOG_INFO);
    return err;
}

void storage_loop(void)
{
    storage_store_data(&stg_wheel_left_speed);
    storage_store_data(&stg_wheel_left_duty);
    storage_store_data(&stg_wheel_right_speed);
    storage_store_data(&stg_wheel_right_duty);
    storage_store_data(&stg_arm_bottom_angle);
    storage_store_data(&stg_arm_shoulder_angle);
    storage_store_data(&stg_arm_elbow_btm_angle);
    storage_store_data(&stg_arm_elbow_top_angle);
    storage_store_data(&stg_arm_wrist_angle);
    storage_store_data(&stg_arm_finger_angle);
}
