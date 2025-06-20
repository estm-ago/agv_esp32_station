#pragma once

#include <esp_https_server.h>

typedef struct async_resp_arg {
    httpd_handle_t httpd_handle;
    int file_descriptor;
} async_resp_arg;

void wifi_https_main(void);
