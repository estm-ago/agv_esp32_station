#pragma once

#include <esp_https_server.h>

typedef struct async_resp_arg {
    httpd_handle_t httpd_handle;
    int file_descriptor;
} async_resp_arg;

esp_err_t https_server_start(void);
esp_err_t https_server_stop(void);
void wifi_https_main(void);
