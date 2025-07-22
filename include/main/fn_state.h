/*
#include "main/fn_state.h"
#include "fn_state.h"
*/
#pragma once

#include "main/config.h"

typedef struct SuccessResult {
    void* obj;
} SuccessResult;

typedef enum ErrorType {
    RES_ERR_INVALID = -1,
    RES_ERR_FAIL,
    RES_ERR_MEMORY_ERROR,
    RES_ERR_BUSY,
    RES_ERR_TIMEOUT,
    RES_ERR_EMPTY,
    RES_ERR_FULL,
    RES_ERR_OVERFLOW,
    RES_ERR_NOT_FOUND,
    RES_ERR_NOT_MOVE,
    RES_ERR_REMOVE_FAIL,
} ErrorType;

typedef struct Result {
    bool is_ok;
    union {
        SuccessResult success;
        ErrorType error;
    } result;
} Result;

#define RESULT_OK(_obj_) ((Result){.is_ok = true, .result.success = {.obj = (_obj_)}})

#define RESULT_ERROR(_err_) ((Result){.is_ok = false, .result.error = (_err_)})

#define RESULT_BOOL(_cond_) ((_cond_) ? RESULT_OK(NULL) : RESULT_ERROR(False))

#define CHECK_RESULT(res)           \
    do {                            \
        if (!(res).is_ok)           \
        {                           \
            return EXIT_FAILURE;    \
        }                           \
    } while (0)

#define CHECK_RES_CLEANUP(res)      \
    do {                            \
        ret = (res);                \
        if (!ret.is_ok)             \
            goto cleanup;           \
    } while (0)

#define UNWRAP_RESULT(res)          \
    ({                              \
        CHECK_RESULT(res);          \
        (res).result.success.obj;   \
    })

typedef uint8_t FnState;
extern FnState last_error;
#define FNS_INVALID         0xFF
#define FNS_OK              0
#define FNS_FAIL            1
#define FNS_TIMEOUT         2
#define FNS_BUF_EMPTY       3
#define FNS_BUF_NOT_ENOU    4
#define FNS_OVERFLOW        5
#define FNS_NOT_FOUND       6
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

#define ERROR_CHECK_FNS_CLEANUP(expr)   \
    do {                                \
        result = (expr);                \
        if (result != FNS_OK)           \
            goto cleanup;               \
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
            ESP_LOGE(TAG, "ERR_H: %d", _err);\
            Error_Handler();            \
        }                               \
    } while (0)

#ifdef AGV_STM32_DEVICE
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
#endif

#ifdef AGV_ESP32_DEVICE

void Error_Handler(void);

#endif

#ifdef PRINCIPAL_PROGRAM
#define ERROR_TIMEOUT_TIME_LIMIT (15*1000)

typedef struct FnState_h
{
    FnState vehicle_rotate_in_place;
    FnState agv_forward_leave_strong_magnet;
} FnState_h;
extern FnState_h error_state;
void timeout_error(uint32_t start_time, FnState *error_parameter);
#endif
