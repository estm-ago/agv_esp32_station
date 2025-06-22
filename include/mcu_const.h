#pragma once

#include <stdint.h>

#define CMD_BX_EMPTY        (uint8_t)0x00

//--------------------------------------------------
#define CMD_B0_DATA         (uint8_t)0x00
#define CMD_B0_DATA_STOP    (uint8_t)0x01
#define CMD_B0_DATA_START   (uint8_t)0x02

#define CMD_B1_LEFT_SPEED   (uint8_t)0x10
#define CMD_B1_RIGHT_SPEED  (uint8_t)0x11
#define CMD_B1_LEFT_DUTY    (uint8_t)0x20
#define CMD_B1_RIGHT_DUTY   (uint8_t)0x21

#define CMD_B2_TOTAL        (uint8_t)0x00
#define CMD_B3_NUMBER       (uint8_t)0x00

//--------------------------------------------------
#define CMD_B0_VECH_CONTROL (uint8_t)0x10

#define CMD_B1_LEFT_STOP    (uint8_t)0x00
#define CMD_B1_LEFT_SPIN    (uint8_t)0x01
#define CMD_B1_RIGHT_STOP   (uint8_t)0x10
#define CMD_B1_RIGHT_SPIN   (uint8_t)0x11
#define CMD_B1_VEHICLE_STOP (uint8_t)0x80
#define CMD_B1_VEHICLE_MOVE (uint8_t)0x81

#define CMD_B2_VALUE        (uint8_t)0x00

#define CMD_B3_NONE         CMD_BX_EMPTY
