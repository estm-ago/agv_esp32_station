#include "storage/sdcard/file.h"

static const char* TAG = "user_sd_file";

static Result file_open(const char* path, const char* type, FILE** file)
{
    *file = fopen(path, type);
    if (*file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open %s", path);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    return RESULT_OK(NULL);
}

static inline Result file_header_write_inner(VecByte* vec_byte, const FileHeader* file_h)
{
    RESULT_CHECK_RET_RES(vec_byte_new(vec_byte, FILE_HEADER_LEN));
    RESULT_CHECK_RET_RES(vec_byte_push_byte(vec_byte, file_h->type));
    RESULT_CHECK_RET_RES(vec_byte_push_u32(vec_byte, file_h->cap));
    RESULT_CHECK_RET_RES(vec_byte_push_u32(vec_byte, file_h->count));
    RESULT_CHECK_RET_RES(vec_byte_push_u32(vec_byte, file_h->head));
    RESULT_CHECK_RET_RES(vec_byte_push_byte(vec_byte, SD_FILE_HEAD_END));
    return RESULT_OK(NULL);
}

static Result file_header_write(FILE* file, const FileHeader* file_h)
{
    VecByte vec_byte;
    Result err = file_header_write_inner(&vec_byte, file_h);
    if (RESULT_CHECK_RAW(err))
    {
        ESP_LOGE(TAG, "Failed to make vec");
        vec_byte_free(&vec_byte);
        return err;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        ESP_LOGE(TAG, "fseek fail");
        vec_byte_free(&vec_byte);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    if (fwrite(vec_byte.data, 1, vec_byte.len, file) != vec_byte.len)
    {
        ESP_LOGE(TAG, "Failed to write header");
        vec_byte_free(&vec_byte);
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    vec_byte_free(&vec_byte);
    return RESULT_OK(NULL);
}

static inline Result file_header_read_inner(VecByte* vec_byte, FILE* file, FileHeader* file_h)
{
    RESULT_CHECK_RET_RES(vec_byte_new(vec_byte, FILE_HEADER_LEN));
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        ESP_LOGE(TAG, "fseek fail");
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    RESULT_CHECK_RET_RES(vec_byte_add_len(vec_byte, FILE_HEADER_LEN));
    if (fread(vec_byte->data, 1, FILE_HEADER_LEN, file) != FILE_HEADER_LEN)
    {
        ESP_LOGE(TAG, "fread fail");
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 0, &file_h->type));
    RESULT_CHECK_RET_RES(vec_byte_get_u32(vec_byte, 1, &file_h->cap));
    RESULT_CHECK_RET_RES(vec_byte_get_u32(vec_byte, 5, &file_h->count));
    RESULT_CHECK_RET_RES(vec_byte_get_u32(vec_byte, 9, &file_h->head));
    uint8_t end_check;
    RESULT_CHECK_RET_RES(vec_byte_get_byte(vec_byte, 13, &end_check));
    if (end_check != SD_FILE_HEAD_END)
    {
        ESP_LOGE(TAG, "header end no match");
        return RESULT_ERROR(RES_ERR_NOT_FOUND);
    }
    return RESULT_OK(NULL);
}

static Result file_header_read(FILE* file, FileHeader* file_h)
{
    VecByte vec_byte;
    Result err = file_header_read_inner(&vec_byte, file, file_h);
    vec_byte_free(&vec_byte);
    if (RESULT_CHECK_RAW(err))
    {
        ESP_LOGE(TAG, "Failed to read header");
        return err;
    }
    return RESULT_OK(NULL);
}

Result file_new(const char* path, const FileHeader* file_h)
{
    FILE* file;
    RESULT_CHECK_RET_RES(file_open(path, "wb+", &file));
    Result err = file_header_write(file, file_h);
    if (RESULT_CHECK_RAW(err))
    {
        fclose(file);
        return err;
    }
    fclose(file);
    return RESULT_OK(NULL);
}

static Result file_data_get_inner(FILE* file, FileHeader* file_h, uint32_t count, VecByte* vec_byte)
{
    uint32_t read_bytes = count * file_h->type;
    Result err = vec_byte_add_len(vec_byte, read_bytes);
    if (RESULT_CHECK_RAW(err))
    {
        ESP_LOGE(TAG, "vec_byte not enough cap");
        return err;
    }
    uint32_t head_byte = ((file_h->head + file_h->count - count) % file_h->cap) * file_h->type;
    uint32_t cap_bytes = file_h->cap * file_h->type;
    if (fseek(file, FILE_HEADER_LEN + head_byte, SEEK_SET) != 0)
    {
        ESP_LOGE(TAG, "fseek fail");
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    if (head_byte + read_bytes <= cap_bytes)
    {
        if (fread(vec_byte->data, 1, read_bytes, file) != read_bytes)
        {
            ESP_LOGE(TAG, "fread fail");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
    }
    else
    {
        uint32_t first_bytes = cap_bytes - head_byte;
        if (fread(vec_byte->data, 1, first_bytes, file) != first_bytes)
        {
            ESP_LOGE(TAG, "fread fail (part1)");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
        uint32_t second_bytes = read_bytes - first_bytes;
        if (fseek(file, FILE_HEADER_LEN, SEEK_SET) != 0)
        {
            ESP_LOGE(TAG, "fseek fail (part2)");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
        if (fread(vec_byte->data + first_bytes, 1, second_bytes, file) != second_bytes)
        {
            ESP_LOGE(TAG, "fread fail (part2)");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
    }
    return RESULT_OK(NULL);
}

Result file_data_get(const char* path, uint32_t count, VecByte* vec_byte)
{
    FILE* file;
    RESULT_CHECK_RET_RES(file_open(path, "rb", &file));
    FileHeader file_h;
    Result err = file_header_read(file, &file_h);
    if (RESULT_CHECK_RAW(err))
    {
        fclose(file);
        return err;
    }
    if (count > file_h.count) count = file_h.count;
    err = file_data_get_inner(file, &file_h, count, vec_byte);
    if (RESULT_CHECK_RAW(err))
    {
        ESP_LOGE(TAG, "Failed to get data");
        fclose(file);
        return err;
    }
    fclose(file);
    return RESULT_OK(NULL);
}

static Result file_data_add_inner(FILE* file, FileHeader* file_h, VecByte* vec_byte)
{
    vec_byte_realign(vec_byte);
    uint32_t tail_byte = ((file_h->head + file_h->count) % file_h->cap) * file_h->type;
    uint32_t cap_bytes = file_h->cap * file_h->type;
    if (tail_byte + vec_byte->len <= cap_bytes)
    {
        if (fseek(file, FILE_HEADER_LEN + tail_byte, SEEK_SET) != 0)
        {
            ESP_LOGE(TAG, "fseek fail");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
        if (fwrite(vec_byte->data, 1, vec_byte->len, file) != vec_byte->len)
        {
            ESP_LOGE(TAG, "fwrite fail");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
    }
    else
    {
        uint32_t first_bytes = cap_bytes - tail_byte;
        if (fseek(file, FILE_HEADER_LEN + tail_byte, SEEK_SET) != 0)
        {
            ESP_LOGE(TAG, "fseek fail (part1)");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
        if (fwrite(vec_byte->data, 1, first_bytes, file) != first_bytes)
        {
            ESP_LOGE(TAG, "fwrite fail (part1)");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
        uint32_t second_bytes = vec_byte->len - first_bytes;
        if (fseek(file, FILE_HEADER_LEN, SEEK_SET) != 0)
        {
            ESP_LOGE(TAG, "fseek fail (part2)");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
        if (fwrite(vec_byte->data + first_bytes, 1, second_bytes, file) != second_bytes)
        {
            ESP_LOGE(TAG, "fwrite fail (part2)");
            return RESULT_ERROR(RES_ERR_FAIL);
        }
    }
    file_h->count += vec_byte->len / file_h->type;
    if (file_h->count > file_h->cap)
    {
        file_h->head = (file_h->head + (file_h->count - file_h->cap)) % file_h->cap;
        file_h->count = file_h->cap;
    }
    Result err = file_header_write(file, file_h);
    if (RESULT_CHECK_RAW(err))
    {
        return err;
    }
    return RESULT_OK(NULL);
}

Result file_data_add(const char* path, VecByte* vec_byte)
{
    if (vec_byte->len == 0) return RESULT_ERROR(RES_ERR_EMPTY);;
    FILE* file;
    RESULT_CHECK_RET_RES(file_open(path, "rb+", &file));
    FileHeader file_h;
    Result result;
    RESULT_CHECK_CLEANUP(file_header_read(file, &file_h));
    if (vec_byte->len % file_h.type != 0)
    {
        ESP_LOGE(TAG, "Data no match");
        result = RESULT_ERROR(RES_ERR_FAIL);
        goto cleanup;
    }
    RESULT_CHECK_CLEANUP(file_data_add_inner(file, &file_h, vec_byte));
    cleanup:
    fclose(file);
    return result;
}
