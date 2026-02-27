#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include "tx_api.h"
#include "system_config.h"
#include <stdbool.h>
#include <stdint.h>


UINT IMUManager_Init(void);

/*
Sends a configuration in the queue
*/
void IMU_Send_Config(IMU_config_t* config);

#endif