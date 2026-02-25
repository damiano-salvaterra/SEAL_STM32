#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <stdbool.h>


typedef struct{
    bool imu_in_use; // true if the IMU is supposed to be used in the current mode of operation
    bool is_on; // true if the imu is on
    bool timer_is_on; //true if the timer used by the imu is on
} IMU_config_t;


UINT IMUManager_Init(void);


#endif