#include "imu.h"
#include "tim.h"
#include "bno08x_service.h"
#include <stdbool.h>

bool is_on = false;


void imu_start(){
    bool status = bno08x_init();

    if (status == 0)
        is_on = true;
    else
        is_on = false;
}

void imu_stop(){
    bno08x_close();

    is_on = false;
}

void imu_get_data(){
    bno08x_service();
}

bool imu_is_on(){
    return is_on;
}