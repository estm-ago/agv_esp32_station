#pragma once

#include <stdint.h>

#define CMD_BX_EMPTY        (uint8_t)0x00

//--------------------------------------------------
#define CMD_B0_DATA         (uint8_t)0x00   // 回傳資料
#define CMD_B0_DATA_STOP    (uint8_t)0x01   // 資料收集開始
#define CMD_B0_DATA_START   (uint8_t)0x02   // 資料收集停止

#define CMD_B1_LEFT_SPEED   (uint8_t)0x10   // 左馬達速度
#define CMD_B1_RIGHT_SPEED  (uint8_t)0x11   // 右馬達速度
#define CMD_B1_LEFT_DUTY    (uint8_t)0x20   // 左馬達功率
#define CMD_B1_RIGHT_DUTY   (uint8_t)0x21   // 右馬達功率

#define CMD_B2_TOTAL        (uint8_t)0x00   // 資料包總計(can)
#define CMD_B3_NUMBER       (uint8_t)0x00   // 資料包編號(can)

//--------------------------------------------------
#define CMD_B0_VECH_CONTROL (uint8_t)0x10   // 車輛控制

#define CMD_B1_VEHICLE_STOP (uint8_t)0x00   // 車輛停止
#define CMD_B1_VEHICLE_MOVE (uint8_t)0x01   // 車輛移動
#define CMD_B1_LEFT_STOP    (uint8_t)0x40   // 左馬達停止
#define CMD_B1_LEFT_SPIN    (uint8_t)0x41   // 左馬達旋轉
#define CMD_B1_RIGHT_STOP   (uint8_t)0x42   // 右馬達停止
#define CMD_B1_RIGHT_SPIN   (uint8_t)0x43   // 右馬達旋轉

#define CMD_B2_FOWARD       (uint8_t)0x00   // 前進/正轉
#define CMD_B2_BACKWARD     (uint8_t)0x00   // 後退/反轉

#define CMD_B2_VALUE        CMD_BX_EMPTY    // 速度
