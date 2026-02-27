#include "sys_logger.h"
#include "tx_api.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#define LOG_BLOCK_SIZE     128   // max 127 (+ null erminator) for each log
#define LOG_POOL_BLOCKS    20    //max log queue
#define LOGGER_STACK_SIZE  1024
#define LOG_POOL_MEM_BYTES (LOG_POOL_BLOCKS * (LOG_BLOCK_SIZE + sizeof(VOID*)))

#define LOGGER_EVENT_MESSAGE   (1UL << 0)
#define LOGGER_EVENT_ENABLE    (1UL << 1)
#define LOGGER_EVENT_DISABLE   (1UL << 2)


/*
Declare RTOS constructs used by the thread.
These are private
*/
TX_BLOCK_POOL log_pool;
TX_QUEUE log_queue;
TX_EVENT_FLAGS_GROUP logger_events;
TX_THREAD logger_thread;

ULONG log_pool_memory[LOG_POOL_MEM_BYTES / sizeof(ULONG)]; //allocate memory contiung the overhead of the linked list pointer
ULONG log_queue_memory[LOG_POOL_BLOCKS]; //queue only contain pointers to blocks
ULONG logger_stack[LOGGER_STACK_SIZE / sizeof(ULONG)];

VOID logger_thread_entry(ULONG initial_input)
{
    TX_PARAMETER_NOT_USED(initial_input);
    char* rx_msg;
    ULONG flags;
    UINT logging_enabled = 1; // start with logging disabled

    while(1)
    {
        // Wait for message or enable/disable event
        tx_event_flags_get(&logger_events,
                           LOGGER_EVENT_MESSAGE |
                           LOGGER_EVENT_ENABLE  |
                           LOGGER_EVENT_DISABLE,
                           TX_OR_CLEAR,
                           &flags,
                           TX_WAIT_FOREVER);

        if(flags & LOGGER_EVENT_ENABLE)
            logging_enabled = 1;

        if(flags & LOGGER_EVENT_DISABLE)
            logging_enabled = 0;

        if(flags & LOGGER_EVENT_MESSAGE)
        {
            // Drain queue to handle burst of messages
            while(tx_queue_receive(&log_queue, &rx_msg, TX_NO_WAIT) == TX_SUCCESS)
            {
                if(logging_enabled)
                    printf("%s", rx_msg); //print message

                tx_block_release(rx_msg); //release memory block at pointer rx_msg
            }
        }
    }
}



UINT SysLogger_Init(void)
{
    //create block pool
    tx_block_pool_create(&log_pool, "Log_Pool", LOG_BLOCK_SIZE, log_pool_memory, sizeof(log_pool_memory));

    //create message queue
    tx_queue_create(&log_queue, "Log_Queue", TX_1_ULONG, log_queue_memory, sizeof(log_queue_memory));
    
    // Create event flags group
    tx_event_flags_create(&logger_events, "Logger_Events");

    //create thread with very low priority
    tx_thread_create(&logger_thread, "Logger_Thread", logger_thread_entry, 0, logger_stack, LOGGER_STACK_SIZE, 12, 12, 1, TX_AUTO_START);

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
        else
            // Wake up the logger thread to process the queue
            tx_event_flags_set(&logger_events, LOGGER_EVENT_MESSAGE, TX_OR);
    }
    //if no allocation is possible, just drop the log
}

/*
Public API to enable logging
*/
void SysLogger_Enable(void)
{
    tx_event_flags_set(&logger_events, LOGGER_EVENT_ENABLE, TX_OR);
}


/*
Public API to disable logging
*/
void SysLogger_Disable(void)
{
    tx_event_flags_set(&logger_events, LOGGER_EVENT_DISABLE, TX_OR);
}