/*
#include "fn_state.h"
*/
#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum FnState
{
    FNS_INVALID = -1,
    FNS_OK      = 0,
    FNS_ERROR   = 1,
    FNS_TIMEOUT,
    FNS_BUF_EMPTY,
    FNS_BUF_OVERFLOW,
    FNS_NO_MATCH,

    FNS_NOT_MOVE,
} FnState;
#define FNS_ERROR_CHECK(expr)   \
    do {                        \
        FnState _err = (expr);  \
        if (_err != FNS_OK)     \
            return _err;        \
    } while (0)
