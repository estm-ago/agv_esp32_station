#pragma once

#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include "main/config.h"
#include "main/fn_state.h"
#include "storage/sdcard/file.h"

#define EXAMPLE_MAX_CHAR_SIZE    64
#define MOUNT_POINT "/sdcard"

Result sd_mount(void);
Result sd_unmount(void);
Result sd_format(void);

void sd_setup(void);
