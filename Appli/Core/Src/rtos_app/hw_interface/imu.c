#include "imu.h"
#include "tim.h"
#include "bno08x_service.h"
#include <stdbool.h>


void imu_get_data(){
    bno08x_service();
}
