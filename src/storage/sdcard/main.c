#include "storage/sdcard/main.h"
#include "storage/sdcard/file.h"

static const char *TAG = "user_sd_card";

static sdmmc_host_t sd_host = SDSPI_HOST_DEFAULT();
static sdspi_device_config_t sd_slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
static sdmmc_card_t *sd_card;
static const char mount_point[] = MOUNT_POINT;

// static esp_err_t s_example_write_file(const char *path, char *data)
// {
//     ESP_LOGI(TAG, "Opening file %s", path);
//     FILE *f = fopen(path, "w");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for writing");
//         return ESP_FAIL;
//     }
//     fprintf(f, data);
//     fclose(f);
//     ESP_LOGI(TAG, "File written");
//     return ESP_OK;
// }

// static esp_err_t s_example_read_file(const char *path)
// {
//     ESP_LOGI(TAG, "Reading file %s", path);
//     FILE *f = fopen(path, "r");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for reading");
//         return ESP_FAIL;
//     }
//     char line[EXAMPLE_MAX_CHAR_SIZE];
//     fgets(line, sizeof(line), f);
//     fclose(f);
//     // strip newline
//     char *pos = strchr(line, '\n');
//     if (pos) {
//         *pos = '\0';
//     }
//     ESP_LOGI(TAG, "Read from file: '%s'", line);
//     return ESP_OK;
// }

Result sd_mount(void)
{
    if (sd_card != NULL) return RESULT_ERROR(RES_ERR_INVALID);;
    sd_host.slot = SDSPI_DEFAULT_HOST;
    #ifndef SD_FIND_MAX_FREQ
    sd_host.max_freq_khz = 5000;
    #else
    sd_host.max_freq_khz = 100;
    #endif
    sd_slot_config.gpio_cs = SD_GPIO_CS;
    sd_slot_config.host_id = sd_host.slot;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SD_GPIO_MOSI,
        .miso_io_num = SD_GPIO_MISO,
        .sclk_io_num = SD_GPIO_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ESP_LOGI(TAG, "Initializing SD sd_card");
    esp_err_t ret = spi_bus_initialize(sd_host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return RESULT_ERROR(RES_ERR_FAIL);
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &sd_host, &sd_slot_config, &mount_config, &sd_card);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the sd_card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the sd_card (%s). "
                     "Make sure SD sd_card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return RESULT_ERROR(RES_ERR_FAIL);
    }
    ESP_LOGI(TAG, "Filesystem mounted");
    sdmmc_card_print_info(stdout, sd_card);
    return RESULT_OK(NULL);
}

Result sd_unmount(void)
{
    if (sd_card == NULL) return RESULT_ERROR(RES_ERR_INVALID);;
    esp_vfs_fat_sdcard_unmount(mount_point, sd_card);
    sd_card = NULL;
    ESP_LOGI(TAG, "Card unmounted");
    spi_bus_free(sd_host.slot);
    return RESULT_OK(NULL);
}

Result sd_format(void)
{
    if (sd_card == NULL) return RESULT_ERROR(RES_ERR_INVALID);;
    esp_err_t ret = esp_vfs_fat_sdcard_format(mount_point, sd_card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to format FATFS (%s)", esp_err_to_name(ret));
        return RESULT_ERROR(RES_ERR_FAIL);
    }

    // if (stat(file_foo, &st) == 0) {
    //     ESP_LOGI(TAG, "file still exists");
    //     return;
    // } else {
    //     ESP_LOGI(TAG, "file doesn't exist, formatting done");
    // }
    return RESULT_OK(NULL);
}

void sd_setup(void)
{
    sd_mount();

    // const char *file_test = MOUNT_POINT"/test";
    // FileHeader header = {
    //     .type = 4,
    //     .cap = 8,
    // };
    // file_new(file_test, &header);
    // VecByte vec_byte;
    // vec_byte_new(&vec_byte, 100);
    // vec_byte_push_u32(&vec_byte, 0);
    // vec_byte_push_u32(&vec_byte, 1);
    // vec_byte_push_u32(&vec_byte, 2);
    // vec_byte_push_u32(&vec_byte, 3);
    // vec_byte_push_u32(&vec_byte, 4);
    // vec_byte_push_u32(&vec_byte, 5);
    // vec_byte_push_u32(&vec_byte, 6);
    // vec_byte_push_u32(&vec_byte, 7);
    // vec_byte_push_u32(&vec_byte, 8);
    // vec_byte_push_u32(&vec_byte, 9);
    // file_data_add(file_test, &vec_byte);
    // vec_rm_all(&vec_byte);
    // vec_byte_push_u32(&vec_byte, 0+16);
    // vec_byte_push_u32(&vec_byte, 1+16);
    // vec_byte_push_u32(&vec_byte, 2+16);
    // vec_byte_push_u32(&vec_byte, 3+16);
    // vec_byte_push_u32(&vec_byte, 4+16);
    // vec_byte_push_u32(&vec_byte, 5+16);
    // vec_byte_push_u32(&vec_byte, 6+16);
    // vec_byte_push_u32(&vec_byte, 7+16);
    // vec_byte_push_u32(&vec_byte, 8+16);
    // vec_byte_push_u32(&vec_byte, 9+16);
    // file_data_add(file_test, &vec_byte);
    // vec_rm_all(&vec_byte);
    // file_data_get(file_test, 40, &vec_byte);
    // ESP_LOG_BUFFER_HEXDUMP(TAG, vec_byte.data, vec_byte.len, ESP_LOG_INFO);

    /*
    // Use POSIX and C standard library functions to work with files.

    // First create a file.
    const char *file_hello = MOUNT_POINT"/hello.txt";
    char data[EXAMPLE_MAX_CHAR_SIZE];
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Hello", sd_card->cid.name);

    esp_err_t ret = s_example_write_file(file_hello, data);
    if (ret != ESP_OK) {
        return;
    }

    const char *file_foo = MOUNT_POINT"/foo.txt";

    // Check if destination file exists before renaming
    struct stat st;
    if (stat(file_foo, &st) == 0) {
        // Delete it if it exists
        unlink(file_foo);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
    if (rename(file_hello, file_foo) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    ret = s_example_read_file(file_foo);
    if (ret != ESP_OK) {
        return;
    }

    const char *file_nihao = MOUNT_POINT"/nihao.txt";
    memset(data, 0, EXAMPLE_MAX_CHAR_SIZE);
    snprintf(data, EXAMPLE_MAX_CHAR_SIZE, "%s %s!\n", "Nihao", sd_card->cid.name);
    ret = s_example_write_file(file_nihao, data);
    if (ret != ESP_OK) {
        return;
    }

    //Open file for reading
    ret = s_example_read_file(file_nihao);
    if (ret != ESP_OK) {
        return;
    }
    */
}
