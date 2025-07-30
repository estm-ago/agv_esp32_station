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

extern ErrorType last_error;

typedef struct Result {
    bool is_ok;
    union {
        SuccessResult success;
        ErrorType error;
    } result;
} Result;

// RESULT_OK(NULL);
#define RESULT_OK(_obj_) ((Result){.is_ok = true, .result.success = {.obj = (_obj_)}})

#define RESULT_ERROR(_err_) ((Result){.is_ok = false, .result.error = (_err_)})

#define RESULT_BOOL(_cond_) ((_cond_) ? RESULT_OK(NULL) : RESULT_ERROR(RES_ERR_FAIL))

#define RESULT_CHECK_RAW(res) (!(res).is_ok)

#define RESULT_CHECK_SIMPLE(res)    \
    do {                            \
        if (RESULT_CHECK_RAW(res))  \
            return EXIT_FAILURE;    \
    } while (0)

    
#define RESULT_CHECK_HANDLE(expr)           \
    do {                                    \
        Result res = (expr);                \
        if (RESULT_CHECK_RAW(res))          \
        {                                   \
            last_error = res.result.error;  \
            Error_Handler();                \
        }                                   \
    } while (0)

#define UNWRAP_RESULT(res)          \
    ({                              \
        RESULT_CHECK_HANDLE(res);   \
        (res).result.success.obj;   \
    })

#define RESULT_CHECK_RET_VOID(expr)         \
    do {                                    \
        Result res = (expr);                \
        if (RESULT_CHECK_RAW(res))          \
        {                                   \
            last_error = res.result.error;  \
            return;                         \
        }                                   \
    } while (0)

#define RESULT_CHECK_RET_RES(expr)          \
    do {                                    \
        Result res = (expr);                \
        if (RESULT_CHECK_RAW(res))          \
        {                                   \
            last_error = res.result.error;  \
            return res;                     \
        }                                   \
    } while (0)

#define RESULT_CHECK_CLEANUP(expr)              \
    do {                                        \
        result = (expr);                        \
        if (RESULT_CHECK_RAW(result))          \
        {                                       \
            last_error = result.result.error;  \
            goto cleanup;                       \
        }                                       \
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

typedef struct Result_h
{
    Result vehicle_rotate_in_place;
    Result agv_forward_leave_strong_magnet;
} Result_h;
extern Result_h error_state;
void timeout_error(uint32_t start_time, Result *error_parameter);
#endif
