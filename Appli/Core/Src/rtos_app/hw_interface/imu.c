#include "imu.h"
#include "tim.h"
#include "bno08x_service.h"
#include <stdbool.h>


void imu_get_data(uint8_t req_frames, IMUValue_t* buff_ptr, uint8_t* num_frames){
    //this function should fill up the IMUValue_t buffer with values returned from bno08x_service
    if (buff_ptr == NULL || num_frames == NULL || req_frames == 0) {
        if (num_frames != NULL) *num_frames = 0;
        return;
    }
    *num_frames = bno08x_get_imu_data(req_frames, buff_ptr);
}
