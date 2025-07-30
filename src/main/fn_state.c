#include "main/fn_state.h"

ErrorType last_error;

#ifdef PRINCIPAL_PROGRAM

Result_h error_state = {
    .vehicle_test_no_load_speed = FNS_INVALID,
    .vehicle_over_hall_fall_back = FNS_INVALID,
    .vehicle_rotate_in_place_hall = FNS_INVALID,
    .vehicle_search_magnetic_path = FNS_INVALID,
    .vehicle2_ensure_motor_stop = FNS_INVALID,
    .vehicle2_renew_vehicle_rotation_status = FNS_INVALID,
    .rotate_in_place__map_data_current_count = FNS_INVALID,
    .breakdown_all_hall_lost__path_not_found = FNS_INVALID,
};

void timeout_error(uint32_t start_time, Result *error_parameter) {
    if (!sys_run_switch.enable_timeout_error) return;

    if (HAL_GetTick() - start_time > ERROR_TIMEOUT_TIME_LIMIT) {
        *error_parameter = FNS_TIMEOUT;
        while(true);
    }
}

#endif

#ifdef AGV_ESP32_DEVICE

void Error_Handler(void)
{
    while (1)
    {
    }
}

#endif

