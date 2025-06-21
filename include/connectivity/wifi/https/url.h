#pragma once

#include <esp_https_server.h>
#include "connectivity/wifi/https/trcv_buffer.h"

extern WSByteTrcvBuf https_rv_pkt_buf;

FnState https_ws_setup(void);
extern const httpd_uri_t ws;
