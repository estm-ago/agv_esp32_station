#pragma once

#include "main/config.h"
#include "main/fn_state.h"
#include "connectivity/fdcan/trcv_buffer.h"

extern FdcanByteTrcvBuf fdcan_trsm_pkt_buf;
extern FdcanByteTrcvBuf fdcan_recv_pkt_buf;

extern FncState fdacn_data_trsm_ready;

void fdcan_setup(void);
