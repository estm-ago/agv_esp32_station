#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include "vec_mod.h"

typedef struct DataStorage {
    VecU8 motor_right_speed[4*100];
    VecU8 right_adc[2*100];
} DataStorage;
extern DataStorage data_storeage;

#endif
