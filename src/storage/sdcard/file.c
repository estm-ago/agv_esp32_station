#include "storage/sdcard/file.h"

static const char* TAG = "user_sd_file";

static FnState file_open(const char* path, const char* type, FILE** file)
{
    *file = fopen(path, type);
    if (*file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open %s", path);
        return FNS_FAIL;
    }
    return FNS_OK;
}

static inline FnState file_header_write_inner(VecByte* vec_byte, const FileHeader* file_h)
{
    ERROR_CHECK_FNS_RETURN(vec_byte_new(vec_byte, FILE_HEADER_LEN));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_byte(vec_byte, file_h->type));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_u32(vec_byte, file_h->cap));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_u32(vec_byte, file_h->count));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_u32(vec_byte, file_h->head));
    ERROR_CHECK_FNS_RETURN(vec_byte_push_byte(vec_byte, SD_FILE_HEAD_END));
    return FNS_OK;
}

static FnState file_header_write(FILE* file, const FileHeader* file_h)
{
    VecByte vec_byte;
    FnState err = file_header_write_inner(&vec_byte, file_h);
    if (ERROR_CHECK_FNS_RAW(err))
    {
        ESP_LOGE(TAG, "Failed to make vec");
        vec_byte_free(&vec_byte);
        return err;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        ESP_LOGE(TAG, "fseek fail");
        vec_byte_free(&vec_byte);
        return FNS_FAIL;
    }
    if (fwrite(vec_byte.data, 1, vec_byte.len, file) != vec_byte.len)
    {
        ESP_LOGE(TAG, "Failed to write header");
        vec_byte_free(&vec_byte);
        return FNS_FAIL;
    }
    vec_byte_free(&vec_byte);
    return FNS_OK;
}

static inline FnState file_header_read_inner(VecByte* vec_byte, FILE* file, FileHeader* file_h)
{
    ERROR_CHECK_FNS_RETURN(vec_byte_new(vec_byte, FILE_HEADER_LEN));
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        ESP_LOGE(TAG, "fseek fail");
        return FNS_FAIL;
    }
    ERROR_CHECK_FNS_RETURN(vec_byte_add_len(vec_byte, FILE_HEADER_LEN));
    if (fread(vec_byte->data, 1, FILE_HEADER_LEN, file) != FILE_HEADER_LEN)
    {
        ESP_LOGE(TAG, "fread fail");
        return FNS_FAIL;
    }
    ERROR_CHECK_FNS_RETURN(vec_byte_get_byte(vec_byte, 0, &file_h->type));
    ERROR_CHECK_FNS_RETURN(vec_byte_get_u32(vec_byte, 1, &file_h->cap));
    ERROR_CHECK_FNS_RETURN(vec_byte_get_u32(vec_byte, 5, &file_h->count));
    ERROR_CHECK_FNS_RETURN(vec_byte_get_u32(vec_byte, 9, &file_h->head));
    uint8_t end_check;
    ERROR_CHECK_FNS_RETURN(vec_byte_get_byte(vec_byte, 13, &end_check));
    if (end_check != SD_FILE_HEAD_END)
    {
        ESP_LOGE(TAG, "header end no match");
        return FNS_NOT_FOUND;
    }
    return FNS_OK;
}

static FnState file_header_read(FILE* file, FileHeader* file_h)
{
    VecByte vec_byte;
    FnState err = file_header_read_inner(&vec_byte, file, file_h);
    vec_byte_free(&vec_byte);
    if (ERROR_CHECK_FNS_RAW(err))
    {
        ESP_LOGE(TAG, "Failed to read header");
        return err;
    }
    return FNS_OK;
}

FnState file_new(const char* path, const FileHeader* file_h)
{
    FILE* file;
    ERROR_CHECK_FNS_RETURN(file_open(path, "wb+", &file));
    FnState err = file_header_write(file, file_h);
    if (ERROR_CHECK_FNS_RAW(err))
    {
        fclose(file);
        return err;
    }
    fclose(file);
    return FNS_OK;
}

static FnState file_data_get_inner(FILE* file, FileHeader* file_h, uint32_t count, VecByte* vec_byte)
{
    uint32_t read_bytes = count * file_h->type;
    FnState err = vec_byte_add_len(vec_byte, read_bytes);
    if (ERROR_CHECK_FNS_RAW(err))
    {
        ESP_LOGE(TAG, "vec_byte not enough cap");
        return err;
    }
    uint32_t head_byte = ((file_h->head + file_h->count - count) % file_h->cap) * file_h->type;
    uint32_t cap_bytes = file_h->cap * file_h->type;
    if (fseek(file, FILE_HEADER_LEN + head_byte, SEEK_SET) != 0)
    {
        ESP_LOGE(TAG, "fseek fail");
        return FNS_FAIL;
    }
    if (head_byte + read_bytes <= cap_bytes)
    {
        if (fread(vec_byte->data, 1, read_bytes, file) != read_bytes)
        {
            ESP_LOGE(TAG, "fread fail");
            return FNS_FAIL;
        }
    }
    else
    {
        uint32_t first_bytes = cap_bytes - head_byte;
        if (fread(vec_byte->data, 1, first_bytes, file) != first_bytes)
        {
            ESP_LOGE(TAG, "fread fail (part1)");
            return FNS_FAIL;
        }
        uint32_t second_bytes = read_bytes - first_bytes;
        if (fseek(file, FILE_HEADER_LEN, SEEK_SET) != 0)
        {
            ESP_LOGE(TAG, "fseek fail (part2)");
            return FNS_FAIL;
        }
        if (fread(vec_byte->data + first_bytes, 1, second_bytes, file) != second_bytes)
        {
            ESP_LOGE(TAG, "fread fail (part2)");
            return FNS_FAIL;
        }
    }
    return FNS_OK;
}

FnState file_data_get(const char* path, uint32_t count, VecByte* vec_byte)
{
    FILE* file;
    ERROR_CHECK_FNS_RETURN(file_open(path, "rb", &file));
    FileHeader file_h;
    FnState err = file_header_read(file, &file_h);
    if (ERROR_CHECK_FNS_RAW(err))
    {
        fclose(file);
        return err;
    }
    if (count > file_h.count) count = file_h.count;
    err = file_data_get_inner(file, &file_h, count, vec_byte);
    if (ERROR_CHECK_FNS_RAW(err))
    {
        ESP_LOGE(TAG, "Failed to get data");
        fclose(file);
        return err;
    }
    fclose(file);
    return FNS_OK;
}

static FnState file_data_add_inner(FILE* file, FileHeader* file_h, VecByte* vec_byte)
{
    vec_byte_realign(vec_byte);
    uint32_t tail_byte = ((file_h->head + file_h->count) % file_h->cap) * file_h->type;
    uint32_t cap_bytes = file_h->cap * file_h->type;
    if (tail_byte + vec_byte->len <= cap_bytes)
    {
        if (fseek(file, FILE_HEADER_LEN + tail_byte, SEEK_SET) != 0)
        {
            ESP_LOGE(TAG, "fseek fail");
            return FNS_FAIL;
        }
        if (fwrite(vec_byte->data, 1, vec_byte->len, file) != vec_byte->len)
        {
            ESP_LOGE(TAG, "fwrite fail");
            return FNS_FAIL;
        }
    }
    else
    {
        uint32_t first_bytes = cap_bytes - tail_byte;
        if (fseek(file, FILE_HEADER_LEN + tail_byte, SEEK_SET) != 0)
        {
            ESP_LOGE(TAG, "fseek fail (part1)");
            return FNS_FAIL;
        }
        if (fwrite(vec_byte->data, 1, first_bytes, file) != first_bytes)
        {
            ESP_LOGE(TAG, "fwrite fail (part1)");
            return FNS_FAIL;
        }
        uint32_t second_bytes = vec_byte->len - first_bytes;
        if (fseek(file, FILE_HEADER_LEN, SEEK_SET) != 0)
        {
            ESP_LOGE(TAG, "fseek fail (part2)");
            return FNS_FAIL;
        }
        if (fwrite(vec_byte->data + first_bytes, 1, second_bytes, file) != second_bytes)
        {
            ESP_LOGE(TAG, "fwrite fail (part2)");
            return FNS_FAIL;
        }
    }
    file_h->count += vec_byte->len / file_h->type;
    if (file_h->count > file_h->cap)
    {
        file_h->head = (file_h->head + (file_h->count - file_h->cap)) % file_h->cap;
        file_h->count = file_h->cap;
    }
    FnState err = file_header_write(file, file_h);
    if (ERROR_CHECK_FNS_RAW(err))
    {
        return err;
    }
    return FNS_OK;
}

FnState file_data_add(const char* path, VecByte* vec_byte)
{
    if (vec_byte->len == 0) return FNS_BUF_EMPTY;
    FILE* file;
    ERROR_CHECK_FNS_RETURN(file_open(path, "rb+", &file));
    FileHeader file_h;
    FnState result;
    ERROR_CHECK_FNS_CLEANUP(file_header_read(file, &file_h));
    if (vec_byte->len % file_h.type != 0)
    {
        ESP_LOGE(TAG, "Data no match");
        result = FNS_FAIL;
        goto cleanup;
    }
    ERROR_CHECK_FNS_CLEANUP(file_data_add_inner(file, &file_h, vec_byte));
    cleanup:
    fclose(file);
    return result;
}
