#pragma once

#include <stdint.h>

#define CMD_BX_EMPTY                (uint8_t)0x00

//----------------------------------------------------------------------------------------------------
#define CMD_DATA_B0_CONTROL         (uint8_t)0x10   // 回傳資料
#define CMD_DATA_B0_STOP            (uint8_t)0x11   // 資料收集停止(can)
#define CMD_DATA_B0_START           (uint8_t)0x12   // 資料收集開始(can)

#define CMD_DATA_B1_VEHI_POSI       (uint8_t)0x00   // 車輛位置
#define CMD_DATA_B1_LEFT_SPEED      (uint8_t)0x10   // 左馬達速度f32
#define CMD_DATA_B1_RIGHT_SPEED     (uint8_t)0x11   // 右馬達速度f32
#define CMD_DATA_B1_LEFT_DUTY       (uint8_t)0x20   // 左馬達功率u8
#define CMD_DATA_B1_RIGHT_DUTY      (uint8_t)0x21   // 右馬達功率u8
#define CMD_DATA_B1_ARM_BOTTOM      (uint8_t)0x40   // 手臂馬達 基座左右
#define CMD_DATA_B1_ARM_SHOULDER    (uint8_t)0x41   // 手臂馬達 基座上下
#define CMD_DATA_B1_ARM_ELBOW_BTM   (uint8_t)0x42   // 手臂馬達
#define CMD_DATA_B1_ARM_ELBOW_TOP   (uint8_t)0x43   // 手臂馬達
#define CMD_DATA_B1_ARM_WRIST       (uint8_t)0x44   // 手臂馬達
#define CMD_DATA_B1_ARM_FINGER      (uint8_t)0x45   // 手臂馬達

//      CMD_DATA_B2_VALUE                           // 資料總計         // 位置ID(車輛位置)

//      CMD_DATA_B3_VALUE                           // 資料編號(can)    // 目標ID(車輛位置)

//----------------------------------------------------------------------------------------------------
#define CMD_WHEEL_B0_CONTROL        (uint8_t)0x20   // 馬達控制

#define CMD_WHEEL_B1_LEFT           (uint8_t)0x01   // 左
#define CMD_WHEEL_B1_RIGHT          (uint8_t)0x02   // 右

#define CMD_WHEEL_B2_MODE           (uint8_t)0x00   // 模式
#define CMD_WHEEL_B2_MOTION         (uint8_t)0x01   // 方向
#define CMD_WHEEL_B2_SPEED          (uint8_t)0x02   // 速度

#define CMD_WHEEL_B3_CONTROL        (uint8_t)0x00   // rps控制模式  /停止
#define CMD_WHEEL_B3_FREE           (uint8_t)0x01   // duty自由模式 /正轉
#define CMD_WHEEL_B3_SLOW           (uint8_t)0x02   // 減速模式     /反轉
//      CMD_WHEEL_B3_VALUE                          // 速度0-100
// #define CMD_WHEEL_B3_LOCK           (uint8_t)0x03   // 鎖定

//----------------------------------------------------------------------------------------------------
#define CMD_VEHI_B0_CONTROL         (uint8_t)0x21   // 車輛控制

#define CMD_VEHI_B1_MODE            (uint8_t)0x00   // 模式
#define CMD_VEHI_B1_MOTION          (uint8_t)0x01   // 方向
#define CMD_VEHI_B1_SPEED           (uint8_t)0x02   // 速度

#define CMD_VEHI_B2_FREE            (uint8_t)0x00   // 自由模式/車輛停止
#define CMD_VEHI_B2_END             (uint8_t)0x01   // 停止模式/車輛前進
#define CMD_VEHI_B2_F_TRACK         (uint8_t)0x02   // 離點模式/車輛後退
#define CMD_VEHI_B2_TRACK           (uint8_t)0x03   // 循跡模式/車輛左旋
#define CMD_VEHI_B2_SEARCH          (uint8_t)0x04   // 尋找模式/車輛右旋
#define CMD_VEHI_B2_ROTATE          (uint8_t)0x05   // 旋轉模式
//      CMD_VEHI_B2_VALUE                           // 速度0-100

//      CMD_VEHI_B3_VALUE                           // 車輛旋轉模式轉幾個磁條數

//----------------------------------------------------------------------------------------------------
#define CMD_MAP_B0_CONTROL          (uint8_t)0x30   // 地圖控制

//----------------------------------------------------------------------------------------------------
#define CMD_ARM_B0_CONTROL          (uint8_t)0x31   // 手臂控制

#define CMD_ARM_B1_BOTTOM           (uint8_t)0x00   // 馬達
#define CMD_ARM_B1_SHOULDER         (uint8_t)0x01   // 馬達
#define CMD_ARM_B1_ELBOW_BTM        (uint8_t)0x02   // 馬達
#define CMD_ARM_B1_ELBOW_TOP        (uint8_t)0x03   // 馬達
#define CMD_ARM_B1_WRIST            (uint8_t)0x04   // 馬達
#define CMD_ARM_B1_FINGER           (uint8_t)0x05   // 馬達
#define CMD_ARM_B1_ARM              (uint8_t)0x10   // 手臂

#define CMD_ARM_B2_STOP             (uint8_t)0x00   // 停止
#define CMD_ARM_B2_SET              (uint8_t)0x01   // 方位設定
#define CMD_ARM_B2_IDLE             (uint8_t)0x00   // 待機位置

//      CMD_ARM_B3_VALUE                            // 方位值0-100

//----------------------------------------------------------------------------------------------------
#define CMD_RFID_B0_CONTROL         (uint8_t)0x32   // RFID控制

#define CMD_RFID_B1_SELECT          (uint8_t)0x00
#define CMD_RFID_B1_INP_DATA        (uint8_t)0x01

//      CMD_RFID_B2_SECTOR                          // 扇區選擇 0-15
//      CMD_RFID_B2_NUMBER                          // DATA號碼 0-3

//      CMD_RFID_B3_BLOCK                           // 區塊選擇 0-2
//      CMD_RFID_(B3-B6)_DATA                       // 4 byte DATA

//      CMD_RFID_B4_CONFIRM                         // 直接送出 0 或 1
#define CMD_RFID_B4_ONLY_SET        (uint8_t)0x00
#define CMD_RFID_B4_WRITE           (uint8_t)0x01

//----------------------------------------------------------------------------------------------------
#define CMD_B0_TEST                 (uint8_t)0xF0

#define CMD_B1_TEST0                (uint8_t)0x00
