/*
#include "main/fn_state.h"
#include "fn_state.h"
*/
#pragma once

#include "main/config.h"

typedef int FnState;
extern FnState last_error;
#define FNS_INVALID        -1
#define FNS_OK              0
#define FNS_FAIL            1
#define FNS_TIMEOUT         2
#define FNS_BUF_EMPTY       3
#define FNS_BUF_NOT_ENOU    4
#define FNS_BUF_OVERFLOW    5
#define FNS_NO_MATCH        6
#define FNS_NOT_MOVE        7
#define FNS_ERR_OOM         8
#define FNS_BUSY            9

#define ERROR_CHECK_FNS_RAW(expr) ((expr) != FNS_OK)

#define ERROR_CHECK_FNS_RETURN(expr)    \
    do {                                \
        FnState _err = (expr);          \
        if (_err != FNS_OK)             \
        {                               \
            last_error = _err;          \
            return _err;                \
        }                               \
    } while (0)

#define ERROR_CHECK_FNS_VOID(expr)  \
    do {                            \
        FnState _err = (expr);      \
        if (_err != FNS_OK)         \
        {                           \
            last_error = _err;      \
            return;                 \
        }                           \
    } while (0)

#define ERROR_CHECK_FNS_HANDLE(expr)    \
    do {                                \
        FnState _err = (expr);          \
        if (_err != FNS_OK)             \
        {                               \
            last_error = _err;          \
            Error_Handler();            \
        }                               \
    } while (0)

#define ERROR_CHECK_HAL_RETERN(expr)        \
    do {                                    \
        HAL_StatusTypeDef _err = (expr);    \
        if (_err != HAL_OK)                 \
        {                                   \
            return _err;                    \
        }                                   \
    } while (0)

#define ERROR_CHECK_HAL_HANDLE(expr)        \
    do {                                    \
        HAL_StatusTypeDef _err = (expr);    \
        if (_err != HAL_OK)                 \
        {                                   \
            Error_Handler();                \
        }                                   \
    } while (0)

#ifdef AGV_ESP32_DEVICE

void Error_Handler(void);

#endif

#ifdef PRINCIPAL_PROGRAM
#define ERROR_TIMEOUT_TIME_LIMIT (15*1000)

typedef struct FnState_h
{
    FnState vehicle_test_no_load_speed;
    FnState vehicle_over_hall_fall_back;
    FnState vehicle_rotate_in_place_hall;
    FnState vehicle_search_magnetic_path;
    FnState vehicle2_ensure_motor_stop;
    FnState vehicle2_renew_vehicle_rotation_status;
    FnState rotate_in_place__map_data_current_count;
    FnState breakdown_all_hall_lost__path_not_found;
} FnState_h;
extern FnState_h error_state;
void timeout_error(uint32_t start_time, FnState *error_parameter);
#endif
