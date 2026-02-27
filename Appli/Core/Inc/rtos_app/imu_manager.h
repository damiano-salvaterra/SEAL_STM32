#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <stdbool.h>
#include <stdint.h>

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

UINT IMUManager_Init(void);

/*
Sends a configuration in the queue
*/
void IMU_Send_Config(IMU_config_t* config);

#endif