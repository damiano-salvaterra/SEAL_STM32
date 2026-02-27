#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H
#include "dsp.h"
#include "imu_manager.h"

/*
System configuration struct
*/
typedef struct {
    IMU_config_t imu_config;
    DSP_config_t dsp_config;
} SystemConfig_t;

#endif