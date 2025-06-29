#pragma once

#include <stdint.h>

#define CMD_BX_EMPTY            (uint8_t)0x00

//--------------------------------------------------
#define CMD_B0_DATA             (uint8_t)0x00   // 回傳資料
#define CMD_B0_DATA_STOP        (uint8_t)0x01   // 資料收集停止(can)
#define CMD_B0_DATA_START       (uint8_t)0x02   // 資料收集開始(can)

#define CMD_B1_LEFT_SPEED       (uint8_t)0x10   // 左馬達速度f32
#define CMD_B1_RIGHT_SPEED      (uint8_t)0x11   // 右馬達速度f32
#define CMD_B1_LEFT_DUTY        (uint8_t)0x20   // 左馬達功率u8
#define CMD_B1_RIGHT_DUTY       (uint8_t)0x21   // 右馬達功率u8

#define CMD_B2_TOTAL            (uint8_t)0x00   // 資料包總計(can)
#define CMD_B3_NUMBER           (uint8_t)0x00   // 資料包編號(can)

//--------------------------------------------------
#define CMD_B0_VECH_CONTROL     (uint8_t)0x10   // 車輛控制

#define CMD_B1_VEHICLE          (uint8_t)0x00   // 車輛
#define CMD_B1_LEFT_MOTOR       (uint8_t)0x40   // 左馬達
#define CMD_B1_RIGHT_MOTOR      (uint8_t)0x50   // 右馬達

#define CMD_B2_STOP             (uint8_t)0x00   // 停止
#define CMD_B2_FOWARD           (uint8_t)0x10   // 前進/正轉
#define CMD_B2_BACKWARD         (uint8_t)0x20   // 後退/反轉

#define CMD_B3_VALUE            (uint8_t)0x00   // 速度0-100

//--------------------------------------------------
#define CMD_B0_ARM_CONTROL      (uint8_t)0x20   // 手臂控制

#define CMD_B1_ARM              (uint8_t)0x00   // 手臂
#define CMD_B1_BOTTOM           (uint8_t)0x40   // 馬達
#define CMD_B1_SHOULDER         (uint8_t)0x50   // 馬達
#define CMD_B1_ELBOW_BTM        (uint8_t)0x60   // 馬達
#define CMD_B1_ELBOW_TOP        (uint8_t)0x70   // 馬達
#define CMD_B1_WRIST            (uint8_t)0x80   // 馬達
#define CMD_B1_FINGER           (uint8_t)0x90   // 馬達

#define CMD_B2_STOP             (uint8_t)0x00   // 停止
#define CMD_B2_SET              (uint8_t)0x10   // 方位設定

#define CMD_B3_VALUE            (uint8_t)0x00   // 方位值0-100
