#include "system_manager.h"
#include "dsp.h"
#include "imu_manager.h"
#include "system_config.h"
#include "bno08x_service.h"
#include "sys_logger.h"
#include "tx_api.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>



#define SYSTEM_MANAGER_STACK_SIZE 512
#define SYSTEM_MANAGER_QUEUE_SIZE ((sizeof(SystemConfig_t)*2)) //message queue size for changing configuration. keep a queue of 2 messages just for safety (should be enough one)

TX_QUEUE system_manager_queue;
TX_THREAD system_manager_thread;

uint8_t system_manager_queue_memory[SYSTEM_MANAGER_QUEUE_SIZE];
uint8_t system_manager_stack[SYSTEM_MANAGER_STACK_SIZE];




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
    memset(&system_config, 0, sizeof(SystemConfig_t));

    while(1)
    {
        SystemConfig_t old_config = system_config;
        tx_queue_receive(&system_manager_queue, &system_config, TX_WAIT_FOREVER);

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
    tx_queue_create(&system_manager_queue, "IMU_Queue", sizeof(IMU_config_t), system_manager_queue_memory, sizeof(system_manager_queue_memory));

    //create thread
    tx_thread_create(&system_manager_thread, "IMU_Thread", system_manager_thread_entry, 0, system_manager_stack, SYSTEM_MANAGER_STACK_SIZE, 25, 25, 1, TX_AUTO_START);

    return TX_SUCCESS;

}


void System_Send_Config(SystemConfig_t* config)
{
    tx_queue_send(&system_manager_queue, config, TX_NO_WAIT);

}

