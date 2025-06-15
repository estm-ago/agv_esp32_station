#pragma once

#include "vec_mod.h"

typedef struct DataStorage {
    VecU8 motor_right_speed[4*100];
    VecU8 right_adc[2*100];
} DataStorage;
extern DataStorage data_storeage;

