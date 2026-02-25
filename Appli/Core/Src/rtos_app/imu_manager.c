#include "imu_manager.h"

#define IMU_MANAGER_STACK_SIZE 512

uint8_t imu_manager_stack[IMU_MANAGER_STACK_SIZE];

VOID imu_manager_thread_entry(ULONG initial_input)
{
    TX_PARAMETER_NOT_USED(initial_input);

}