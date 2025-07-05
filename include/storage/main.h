#pragma once

#include "main/config.h"
#include "main/fn_state.h"
#include "main/vec.h"

extern VecByte stg_b_m_left_speed;
extern VecByte stg_b_m_left_duty;
extern VecByte stg_b_m_right_speed;
extern VecByte stg_b_m_right_duty;

FnState storage_setup(void);
void storage_loop(void);
