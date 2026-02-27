#include "imu_manager.h"
#include "sys_logger.h"
#include "bno08x_service.h"
#include "tx_api.h"
#include "imu.h"
#include "hw_timers.h"
#include <string.h>
#include <stdbool.h>

/*
This struct holds the state of the IMU
*/
typedef struct{
    bool is_on; // true if the imu is on
    ImuUARTProto protocol;
    uint32_t polling_time_ms; //polling interval in milliseconds. It is 0 if no polling is used (sensor is queried when used only)
} IMU_state_t;


#define IMU_MANAGER_STACK_SIZE 512
#define IMU_MANAGER_QUEUE_SIZE (sizeof(IMU_config_t)*2) //message queue size for changing configuration. keep a queue of 2 messages just for safety (should be enough one)

TX_QUEUE imu_manager_queue;
TX_THREAD imu_manager_thread;


uint8_t imu_queue_memory[IMU_MANAGER_QUEUE_SIZE];
uint8_t imu_manager_stack[IMU_MANAGER_STACK_SIZE];


void imu_init(IMU_state_t* state, IMU_config_t* config)
{
    if(!state->is_on) //if imu is off, turn it on, otherwise do nothing
        if(config->protocol == UART_RVC)
        {
            timer_imu_start(); //ensure IMU timer is on
            if(bno08x_RVC_init() == 0) //if returns 0, everything is fine
            {
                state->is_on = true;
                state->protocol = UART_RVC;
                state->polling_time_ms = 0; //no polling, we query the sensor when needed (UART RVC keep sending data every 10 ms, is useless to poll it anyway)
            }
        }
}           

void imu_close(IMU_state_t* state, IMU_config_t* config)
{
    if(state->is_on)
    {
        if(config->protocol == UART_RVC)
            bno08x_RVC_close();
        state->is_on = false;
        state->protocol = NO_PROTO;
        state->polling_time_ms = 0;
    }
}

VOID imu_manager_thread_entry(ULONG initial_input)
{
    TX_PARAMETER_NOT_USED(initial_input);
    IMU_config_t last_config;
    IMU_state_t state = {.is_on = false, .protocol = NO_PROTO, .polling_time_ms = 0};

    //clear config
    memset(&last_config, 0, sizeof(last_config));

    while(1)
    {
        tx_queue_receive(&imu_manager_queue, &last_config, TX_WAIT_FOREVER);

        System_Log(
                "[IMUManager] INFO: new IMU configuration received\r\n"
                "                   Current state -> is_on: %d, protocol: %d, polling_time_ms: %d\r\n"
                "                   New Config    -> imu_turn_on: %d, protocol: %d, polling_time_ms: %d\r\n",
                state.is_on,
                state.protocol,
                state.polling_time_ms,
                last_config.imu_turn_on,
                last_config.protocol,
                last_config.polling_time_ms
                );

        if(last_config.imu_turn_on)
            imu_init(&state, &last_config);
        else
            imu_close(&state, &last_config);

        /*
        For future expansion, manage also other configurations (protocol and polling time)
        */

        
    }
    
}

UINT IMUManager_Init()
{
    //create queue to send configurations
    tx_queue_create(&imu_manager_queue, "IMU_Queue", sizeof(IMU_config_t), imu_queue_memory, sizeof(imu_queue_memory));

    //create thread
    tx_thread_create(&imu_manager_thread, "IMU_Thread", imu_manager_thread_entry, 0, imu_manager_stack, IMU_MANAGER_STACK_SIZE, 25, 25, 1, TX_AUTO_START);

    return TX_SUCCESS;

}

void IMU_Send_Config(IMU_config_t* config)
{
    tx_queue_send(&imu_manager_queue, config, TX_NO_WAIT);
}