#include "storage/main.h"
#include "storage/sdcard/main.h"

static const char *TAG = "user_stg_main";

const char *stg_f_m_left_speed  = MOUNT_POINT"/mls.hex";
VecByte     stg_b_m_left_speed;
const char *stg_f_m_left_duty   = MOUNT_POINT"/mld.hex";
VecByte     stg_b_m_left_duty;
const char *stg_f_m_right_speed = MOUNT_POINT"/mrs.hex";
VecByte     stg_b_m_right_speed;
const char *stg_f_m_right_duty  = MOUNT_POINT"/mrd.hex";
VecByte     stg_b_m_right_duty;

FnState storage_setup(void)
{
    sd_setup();
    FileHeader file_header = {
        .cap = 60,
    };
    file_header.type = sizeof(float);
    ERROR_CHECK_FNS_RETURN(file_new(stg_f_m_left_speed, &file_header));
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&stg_b_m_left_speed, sizeof(float) * STORAGE_BUF_MAX));
    ERROR_CHECK_FNS_RETURN(file_new(stg_f_m_right_speed, &file_header));
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&stg_b_m_right_speed, sizeof(float) * STORAGE_BUF_MAX));
    file_header.type = sizeof(uint8_t);
    ERROR_CHECK_FNS_RETURN(file_new(stg_f_m_left_duty, &file_header));
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&stg_b_m_left_duty, sizeof(uint8_t) * STORAGE_BUF_MAX));
    ERROR_CHECK_FNS_RETURN(file_new(stg_f_m_right_duty, &file_header));
    ERROR_CHECK_FNS_RETURN(vec_byte_new(&stg_b_m_right_duty, sizeof(uint8_t) * STORAGE_BUF_MAX));
    return FNS_OK;
}

void storage_loop(void)
{
    if (stg_b_m_left_speed.len >= STORAGE_BUF_TRI)
    {
        file_data_add(stg_f_m_left_speed, &stg_b_m_left_speed);
        ESP_LOG_BUFFER_HEXDUMP(TAG, stg_b_m_left_speed.data, stg_b_m_left_speed.len, ESP_LOG_INFO);
        vec_rm_all(&stg_b_m_left_speed);
        VecByte vec_byte;
        vec_byte_new(&vec_byte, 240);
        file_data_get(stg_f_m_left_speed, vec_byte.cap / sizeof(uint32_t), &vec_byte);
        ESP_LOG_BUFFER_HEXDUMP(TAG, vec_byte.data, vec_byte.len, ESP_LOG_INFO);
    }
    if (stg_b_m_left_duty.len >= STORAGE_BUF_TRI)
    {
        file_data_add(stg_f_m_left_duty, &stg_b_m_left_duty);
    }
    if (stg_b_m_right_speed.len >= STORAGE_BUF_TRI)
    {
        file_data_add(stg_f_m_right_speed, &stg_b_m_right_speed);
    }
    if (stg_b_m_right_duty.len >= STORAGE_BUF_TRI)
    {
        file_data_add(stg_f_m_right_duty, &stg_b_m_right_duty);
    }
}
