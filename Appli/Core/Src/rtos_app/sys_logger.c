#include "sys_logger.h"
#include "tx_api.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#define LOG_BLOCK_SIZE     128   // max 127 (+ null erminator) for each log
#define LOG_POOL_BLOCKS    20    //max log queue
#define LOGGER_STACK_SIZE  1024


/*
Declare RTOS constructs used by the thread.
These are private
*/
TX_BLOCK_POOL log_pool;
TX_QUEUE log_queue;
TX_THREAD logger_thread;

uint8_t log_pool_memory[LOG_POOL_BLOCKS*(LOG_BLOCK_SIZE + sizeof(VOID*))]; //allocate memory contiung the overhead of the linked list pointer
ULONG log_queue_memory[LOG_POOL_BLOCKS]; //queue only contain pointers to blocks
uint8_t logger_stack[LOGGER_STACK_SIZE];

VOID logger_thread_entry(ULONG initial_input)
{
    TX_PARAMETER_NOT_USED(initial_input);
    char* rx_msg;

    while(1)
    {
        tx_queue_receive(&log_queue, &rx_msg, TX_WAIT_FOREVER); //sleep forever till a message arrives in the queue
        
        printf("%s", rx_msg); //print message

        tx_block_release(rx_msg); //release memory block at pointer rx_msg

    }
}



UINT SysLogger_Init(void)
{
    //create block pool
    tx_block_pool_create(&log_pool, "Log_Pool", LOG_BLOCK_SIZE, log_pool_memory, sizeof(log_pool_memory));

    //create message queue
    tx_queue_create(&log_queue, "Log_Queue", TX_1_ULONG, log_queue_memory, sizeof(log_queue_memory));

    //create thread with very low priority
    tx_thread_create(&logger_thread, "Logger_Thread", logger_thread_entry, 0, logger_stack, LOGGER_STACK_SIZE, 25, 25, 1, TX_AUTO_START);

    return TX_SUCCESS;
}


void System_Log(const char* format, ...)
{
    char *allocated_block = NULL;

    //We need to pick an empty block and allocate it to write the message in 
    if(tx_block_allocate(&log_pool, (VOID**)&allocated_block, TX_NO_WAIT) == TX_SUCCESS)
    {
        //write formatted string into block (make use of the variadic function and API like in printf)
        va_list args;
        va_start(args, format);
        vsnprintf(allocated_block, LOG_BLOCK_SIZE, format, args);
        va_end(args);

        //now, send the pointer to the queue to tell the thread that there is something in the memory block
        if(tx_queue_send(&log_queue, &allocated_block, TX_NO_WAIT) != TX_SUCCESS)
            tx_block_release(allocated_block); // if the queue is full, release the block, otherwise it will never be read and cause memory leak
    }
    //if no allocation is possible, just drop the log
}