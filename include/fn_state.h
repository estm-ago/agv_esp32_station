/*
#include "fn_state.h"
*/
#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef int FnState;

#define FNS_INVALID         -1
#define FNS_OK              0
#define FNS_FAIL            1
#define FNS_TIMEOUT         2
#define FNS_BUF_EMPTY       3
#define FNS_BUF_OVERFLOW    4
#define FNS_NO_MATCH        5
#define FNS_NOT_MOVE        6

#define FNS_ERROR_CHECK(expr)   \
    do {                        \
        FnState _err = (expr);  \
        if (_err != FNS_OK)     \
            return _err;        \
    } while (0)
