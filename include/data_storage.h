#pragma once

#include "vec.h"

typedef struct DataStorage {
    Vec_U8 motor_right_speed[4*100];
    Vec_U8 right_adc[2*100];
} DataStorage;
extern DataStorage data_storeage;

