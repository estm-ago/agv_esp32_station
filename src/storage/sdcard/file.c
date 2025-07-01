#include "storage/sdcard/file.h"

static const char *TAG = "SD_IO";

static inline FnState new_init(VecByte* vec_byte)
{
    ERROR_CHECK_FNS_RETURN(vec_byte_new(vec_byte, 9));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_u32(vec_byte, 0));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_u32(vec_byte, 0));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_byte(vec_byte, '\n'));
    return FNS_OK;
}

FnState file_new(const char *path)
{
    FILE *file = fopen(path, "w");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open %s", path);
        return FNS_FAIL;
    }
    VecByte vec_byte;
    FnState err = new_init(&vec_byte);
    if (err != FNS_OK)
    {
        ESP_LOGE(TAG, "Failed to create vec");
        fclose(file);
        return err;
    }
    if (fwrite(vec_byte.data, 1, vec_byte.len, file) != vec_byte.len)
    {
        ESP_LOGE(TAG, "Failed to write vec in %s", path);
        fclose(file);
        return FNS_FAIL;
    }
    vec_byte_free(&vec_byte);
    fclose(file);
    return FNS_OK;
}
