#include "system_manager.h"
#include "bno08x_service.h"
#include "sys_logger.h"
#include"imu_manager.h"
#include "tx_api.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>



#define SYSTEM_MANAGER_STACK_SIZE 512
#define SYSTEM_MANAGER_QUEUE_CAPACITY 2
#define SYSTEM_MANAGER_MESSAGE_SIZE   TX_8_ULONG
#define SYSTEM_MANAGER_QUEUE_MEM_SIZE (32 * SYSTEM_MANAGER_QUEUE_CAPACITY)

TX_QUEUE system_manager_queue;
TX_THREAD system_manager_thread;

ULONG system_manager_queue_memory[SYSTEM_MANAGER_QUEUE_MEM_SIZE / sizeof(ULONG)];
ULONG system_manager_stack[SYSTEM_MANAGER_STACK_SIZE / sizeof(ULONG)];




bool IMU_config_equal(IMU_config_t* a, IMU_config_t* b)
{
    return a->imu_turn_on == b->imu_turn_on &&
            a->protocol == b->protocol &&
            a->polling_time_ms == b->polling_time_ms;
}

VOID system_manager_thread_entry(ULONG initial_input)
{
    TX_PARAMETER_NOT_USED(initial_input);

    SystemConfig_t system_config;
    ULONG rx_buffer[8]; // Exact size of TX_8_ULONG (32 bytes). THe message received are 16 bytes, if we cast them directly in the struct (12 bytes, we smash other variables in the stack)
    memset(&system_config, 0, sizeof(SystemConfig_t));

    while(1)
    {
        SystemConfig_t old_config = system_config;

        //receive the message and copy it into a struct, copying only the meaningful bytes
        tx_queue_receive(&system_manager_queue, rx_buffer, TX_WAIT_FOREVER);
        memcpy(&system_config, rx_buffer, sizeof(SystemConfig_t));

        System_Log("[SystemManager] INFO: New configuration received.");

        //TODO: Implement logic to change configurations


        //send new configuration only if is different than the old one
        if(!IMU_config_equal(&system_config.imu_config, &old_config.imu_config))
        {
            IMU_Send_Config(&system_config.imu_config);
            System_Log("[SystemManager] INFO: Sending new configuration to IMU.");
        }

        //TODO:do the same with the DSP config
        
    }
}



UINT SystemManager_Init()
{
    //create queue to send configurations
    tx_queue_create(&system_manager_queue, "SystemManager_Queue", SYSTEM_MANAGER_MESSAGE_SIZE, system_manager_queue_memory, SYSTEM_MANAGER_QUEUE_MEM_SIZE);

    //create thread
    tx_thread_create(&system_manager_thread, "SystemManager_Thread", system_manager_thread_entry, 0, system_manager_stack, SYSTEM_MANAGER_STACK_SIZE, 10, 10, 1, TX_AUTO_START);

    return TX_SUCCESS;

}


void System_Send_Config(SystemConfig_t* config)
{
    ULONG tx_buffer[8] = {0}; // Initialize all 32 bytes to zero
    memcpy(tx_buffer, config, sizeof(SystemConfig_t)); // Copy the valid 20 bytes
    tx_queue_send(&system_manager_queue, tx_buffer, TX_NO_WAIT);

}

