#pragma once

#include "main/fn_state.h"
#include "connectivity/wifi/https/trcv_buffer.h"

extern httpd_handle_t https_server;

extern const httpd_uri_t ws;

extern WSByteTrcvBuf https_trsm_pkt_buf;
extern WSByteTrcvBuf https_recv_pkt_buf;

FnState https_server_setup(void);
