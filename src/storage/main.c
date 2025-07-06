#include "storage/main.h"
#include "storage/sdcard/main.h"

static const char *TAG = "user_stg_main";

FileData stg_m_left_speed = {
    .path = MOUNT_POINT"/mls.hex",
    .type = sizeof(float),
};

FileData stg_m_left_duty = {
    .path = MOUNT_POINT"/mld.hex",
    .type = sizeof(uint8_t),
};

FileData stg_m_right_speed = {
    .path = MOUNT_POINT"/mrs.hex",
    .type = sizeof(float),
};

FileData stg_m_right_duty = {
    .path = MOUNT_POINT"/mrd.hex",
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
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_m_left_speed));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_m_left_duty));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_m_right_speed));
    ERROR_CHECK_FNS_RETURN(file_buf_init(&stg_m_right_duty));
    return FNS_OK;
}

static FnState store_data(FileData* file_data)
{
    if (file_data->buffer.len < STORAGE_BUF_TRI) return FNS_OK;
    ESP_LOGI(TAG, "buffer data >>>");
    ESP_LOG_BUFFER_HEXDUMP(TAG, file_data->buffer.data, file_data->buffer.len, ESP_LOG_INFO);
    FnState err = file_data_add(file_data->path, &file_data->buffer);
    vec_rm_all(&file_data->buffer);
    VecByte vec_byte;
    vec_byte_new(&vec_byte, file_data->type * 80);
    file_data_get(file_data->path, 80, &vec_byte);
    ESP_LOGI(TAG, "file data >>>");
    ESP_LOG_BUFFER_HEXDUMP(TAG, vec_byte.data, vec_byte.len, ESP_LOG_INFO);
    return err;
}

void storage_loop(void)
{
    store_data(&stg_m_left_speed);
    store_data(&stg_m_left_duty);
    store_data(&stg_m_right_speed);
    store_data(&stg_m_right_duty);
}
