#ifndef IMU_H
#define IMU_H
#include <stdint.h>
#include <stdbool.h>

void imu_start(void);

void imu_stop(void);

void imu_get_data(void);

bool imu_is_on(void);

 #endif