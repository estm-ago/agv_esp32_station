#pragma once

#include "connectivity/uart/main.h"

typedef struct GlobalState {
    // /**
    //  * @brief 傳輸/接收操作旗標
    //  *        Transmit/receive operation flags
    //  *
    //  * @details 控制資料處理流程 (Control data processing flow)
    //  */
    // TransceiveFlags transceive_flags;
    // /**
    //  * @brief 全域傳輸緩衝區
    //  *        Global transmit ring buffer
    //  */
    // UartTrcvBuf uart_trsm_pkt_buf;
    // /**
    //  * @brief 全域接收緩衝區
    //  *        Global receive ring buffer
    //  */
    // UartTrcvBuf uart_recv_pkt_buf;
} GlobalState;
extern GlobalState global_state;
