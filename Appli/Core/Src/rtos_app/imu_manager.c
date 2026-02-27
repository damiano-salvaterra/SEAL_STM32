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
#define IMU_QUEUE_CAPACITY      2
#define IMU_MESSAGE_SIZE        TX_4_ULONG
#define IMU_QUEUE_MEM_SIZE      (16 * IMU_QUEUE_CAPACITY)


TX_QUEUE imu_manager_queue;
TX_THREAD imu_manager_thread;


ULONG imu_queue_memory[IMU_QUEUE_MEM_SIZE / sizeof(ULONG)];
ULONG imu_manager_stack[IMU_MANAGER_STACK_SIZE / sizeof(ULONG)];


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
    IMU_config_t current_config;
    ULONG rx_buffer[4]; // Exact size of TX_4_ULONG (16 bytes)
    IMU_state_t state = {.is_on = false, .protocol = NO_PROTO, .polling_time_ms = 0};

    //clear config
    memset(&current_config, 0, sizeof(current_config));

    while(1)
    {
        tx_queue_receive(&imu_manager_queue, rx_buffer, TX_WAIT_FOREVER);
        memcpy(&current_config, rx_buffer, sizeof(IMU_config_t));

        System_Log(
                "[IMUManager] INFO: new IMU configuration received\r\n"
                "                   Current state -> is_on: %d, protocol: %d, polling_time_ms: %d\r\n"
                "                   New Config    -> imu_turn_on: %d, protocol: %d, polling_time_ms: %d\r\n",
                state.is_on,
                state.protocol,
                state.polling_time_ms,
                current_config.imu_turn_on,
                current_config.protocol,
                current_config.polling_time_ms
                );

        if(current_config.imu_turn_on)
            imu_init(&state, &current_config);
        else
            imu_close(&state, &current_config);

        /*
        For future expansion, manage also other configurations (protocol and polling time)
        */

        
    }
    
}

UINT IMUManager_Init()
{
    //create queue to send configurations
    tx_queue_create(&imu_manager_queue, "IMU_Queue", IMU_MESSAGE_SIZE, imu_queue_memory, IMU_QUEUE_MEM_SIZE);
    //create thread
    tx_thread_create(&imu_manager_thread, "IMU_Thread", imu_manager_thread_entry, 0, imu_manager_stack, IMU_MANAGER_STACK_SIZE, 8, 8, 1, TX_AUTO_START);

    return TX_SUCCESS;

}

void IMU_Send_Config(IMU_config_t* config)
{
    ULONG tx_buffer[4] = {0}; // Initialize all 16 bytes to zero
    memcpy(tx_buffer, config, sizeof(IMU_config_t)); // Copy the valid 12 bytes
    tx_queue_send(&imu_manager_queue, tx_buffer, TX_NO_WAIT);
}