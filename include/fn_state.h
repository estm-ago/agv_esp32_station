/*
#include "main/fn_state.h"
*/
#pragma once

#include <stdint.h>
#include <stdbool.h>

#define error_timeout_time_limit 30 * 1000

typedef int FnState;
extern FnState last_error;

#define FNS_INVALID         -1
#define FNS_OK              0
#define FNS_FAIL            1
#define FNS_TIMEOUT         2
#define FNS_BUF_EMPTY       3
#define FNS_BUF_OVERFLOW    4
#define FNS_NO_MATCH        5
#define FNS_NOT_MOVE        6
#define FNS_ERR_OOM         7

#define FNS_ERROR_CHECK(expr)   \
    do {                        \
        FnState _err = (expr);  \
        if (_err != FNS_OK)     \
        {                       \
            last_error = _err;  \
            return _err;        \
        }                       \
    } while (0)

#define FNS_ERROR_CHECK_VOID(expr)  \
    do {                            \
        FnState _err = (expr);      \
        if (_err != FNS_OK)         \
        {                           \
            last_error = _err;      \
            return;                 \
        }                           \
    } while (0)

#define FNS_ERROR_CHECK_CLEAN(fncall, cleanup)  \
    do {                                        \
        FnState _err = (fncall);               \
        if (_err != FNS_OK)                    \
        {                                       \
            cleanup;                            \
            last_error = _err;                  \
            return _err;                       \
        }                                       \
    } while (0)

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

bool timeout_error(uint32_t error_start, FnState *error_parameter);
