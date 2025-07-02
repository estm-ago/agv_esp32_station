#pragma once

#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include "main/config.h"
#include "main/fn_state.h"

FnState sd_mount(void);
FnState sd_unmount(void);
FnState sd_format(void);

void sd_main(void);
