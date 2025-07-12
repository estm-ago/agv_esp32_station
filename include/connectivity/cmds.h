#pragma once

#include <stdint.h>

#define CMD_BX_EMPTY                (uint8_t)0x00

//----------------------------------------------------------------------------------------------------
#define CMD_DATA_B0                 (uint8_t)0x00   // 回傳資料
#define CMD_DATA_B0_STOP            (uint8_t)0x01   // 資料收集停止(can)
#define CMD_DATA_B0_START           (uint8_t)0x02   // 資料收集開始(can)

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

//      CMD_DATA_B2_TOTAL                           // 資料總計
//      CMD_DATA_B2_VEHI_POS                        // 位置ID(車輛位置)

//      CMD_DATA_B3_VEHI_NUMBER                     // 資料編號(can)
//      CMD_DATA_B3_VEHI_FACING                     // 目標ID(車輛位置)

//----------------------------------------------------------------------------------------------------
#define CMD_VEHI_B0_CONTROL         (uint8_t)0x10   // 車輛控制

#define CMD_VEHI_B1_VEHICLE         (uint8_t)0x00   // 車輛
#define CMD_VEHI_B1_LEFT_MOTOR      (uint8_t)0x40   // 左馬達
#define CMD_VEHI_B1_RIGHT_MOTOR     (uint8_t)0x50   // 右馬達

#define CMD_VEHI_B2_MODE            (uint8_t)0x00   // 模式
#define CMD_VEHI_B2_DIRECT          (uint8_t)0x10   // 方向
#define CMD_VEHI_B2_SPEED           (uint8_t)0x20   // 速度

//      CMD_VEHI_B3_VALUE                           // 速度0-100
#define CMD_VEHI_B3_STOP            (uint8_t)0x00   // 車輛自由模式/停止    // 馬達rps控制模式 /停止
#define CMD_VEHI_B3_FOWARD          (uint8_t)0x01   // 車輛循跡模式/前進    // 馬達duty自由模式/正轉
#define CMD_VEHI_B3_BACKWARD        (uint8_t)0x02   // 車輛尋找模式/後退    // 馬達減速模式    /反轉
#define CMD_VEHI_B3_C_CLOCK         (uint8_t)0x03   // 車輛原地左旋
#define CMD_VEHI_B3_CLOCK           (uint8_t)0x04   // 車輛原地右旋

//----------------------------------------------------------------------------------------------------
#define CMD_ARM_B0_CONTROL          (uint8_t)0x20   // 手臂控制

#define CMD_ARM_B1_ARM              (uint8_t)0x00   // 手臂
#define CMD_ARM_B1_BOTTOM           (uint8_t)0x40   // 馬達
#define CMD_ARM_B1_SHOULDER         (uint8_t)0x50   // 馬達
#define CMD_ARM_B1_ELBOW_BTM        (uint8_t)0x60   // 馬達
#define CMD_ARM_B1_ELBOW_TOP        (uint8_t)0x70   // 馬達
#define CMD_ARM_B1_WRIST            (uint8_t)0x80   // 馬達
#define CMD_ARM_B1_FINGER           (uint8_t)0x90   // 馬達

#define CMD_ARM_B2_STOP             (uint8_t)0x00   // 停止
#define CMD_ARM_B2_SET              (uint8_t)0x10   // 方位設定

//      CMD_ARM_B3_VALUE                            // 方位值0-100

//----------------------------------------------------------------------------------------------------
#define CMD_RFID_B0_CONTROL         (uint8_t)0x21   // RFID控制

#define CMD_RFID_B1_SELECT          (uint8_t)0x00
#define CMD_RFID_B1_INP_DATA        (uint8_t)0x10

//      CMD_RFID_B2_SECTOR                          // 扇區選擇 0-15
//      CMD_RFID_B2_NUMBER                          // DATA號碼 0-3

//      CMD_RFID_B3_BLOCK                           // 區塊選擇 0-2
//      CMD_RFID_(B3-B6)_DATA                       // 4 byte DATA

//      CMD_RFID_B4_CONFIRM                         // 直接送出 0 或 1
#define CMD_RFID_B4_ONLY_SET        (uint8_t)0x00
#define CMD_RFID_B4_WRITE           (uint8_t)0x01

//----------------------------------------------------------------------------------------------------
#define CMD_B0_TEST                 (uint8_t)0xFF

#define CMD_B1_TEST0                (uint8_t)0x00
