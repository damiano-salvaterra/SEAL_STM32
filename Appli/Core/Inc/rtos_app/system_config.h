#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/*
Define the transport protocol running over the UART (for future use)
*/
typedef enum{
    NO_PROTO = 0,
    UART_RVC = 1,
    UART_SHTP = 2
} ImuUARTProto;
/*
This struct is used to Init the thread and to pass configurations to it
*/

typedef struct{
    bool imu_turn_on; // true if the IMU has to be turned on (depends on the mode of operation)
    ImuUARTProto protocol;
    uint32_t polling_time_ms; //polling interval in milliseconds. It is 0 if no polling is used (sensor is queried when used only)
} IMU_config_t;


typedef struct { //placeholder for debugging
    uint32_t foo;
    uint32_t bar;
} processing_config_t;


/*
System configuration struct
*/
typedef struct {
    IMU_config_t imu_config;
    processing_config_t dsp_config;
} SystemConfig_t;


#endif