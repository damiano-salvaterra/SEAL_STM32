#ifndef IMU_H
#define IMU_H
#include <stdint.h>
#include <stdbool.h>


typedef struct{
    uint64_t timestamp_uS;
    uint8_t index;
    float yaw_deg;
    float pitch_deg;
    float roll_deg;
    float acc_x_g;
    float acc_y_g;
    float acc_z_g;
} IMUValue_t;

/*
Returns the buffer filled up with data (IMUValue_t struct).
num_data is the actual number of data element in the array
req_frames is the number of dataframes to request
(It is possible that the number of data requested is not available so less data are returned)*/
void imu_get_data(uint8_t req_frames, IMUValue_t* buff_ptr, uint8_t* num_frames);

 #endif