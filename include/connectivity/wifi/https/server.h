#pragma once

#include <esp_https_server.h>
#include "config.h"
#include "fn_state.h"

typedef struct async_resp_arg {
    httpd_handle_t httpd_handle;
    int sockfd;
} async_resp_arg;

esp_err_t https_server_start(void);
esp_err_t https_server_stop(void);
