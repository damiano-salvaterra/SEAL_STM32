#include "sys_logger.h"
#include "tx_api.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#define MAX_LOG_ARGS 16

//typedef struct {
//    const char* format;
//    uint32_t args[MAX_LOG_ARGS];
//} deferred_log_msg_t;


#define LOG_BLOCK_SIZE     256 //TODO: check if this can be reduced

#define LOG_POOL_BLOCKS    20    //max log queue
#define LOGGER_STACK_SIZE  2048
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

        /*
        if(flags & LOGGER_EVENT_MESSAGE)
        {
            deferred_log_msg_t *msg;
            // Drain queue to handle burst of messages
            while(tx_queue_receive(&log_queue, &msg, TX_NO_WAIT) == TX_SUCCESS)
            {
                if(logging_enabled)
                    printf(msg->format, 
                           msg->args[0],  msg->args[1],  msg->args[2],  msg->args[3], 
                           msg->args[4],  msg->args[5],  msg->args[6],  msg->args[7], 
                           msg->args[8],  msg->args[9],  msg->args[10], msg->args[11], 
                           msg->args[12], msg->args[13], msg->args[14], msg->args[15]);
                tx_block_release(msg); //release memory block at pointer msg
            }
        }
            */
        if(flags & LOGGER_EVENT_MESSAGE)
        {
            char *msg; // receive pointer to string
            
            // Drain queue
            while(tx_queue_receive(&log_queue, &msg, TX_NO_WAIT) == TX_SUCCESS)
            {
                if(logging_enabled) {
                    // print formatted string
                    printf("%s", msg);
                }
                
                // free memory block
                tx_block_release(msg); 
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

    // Allochiamo un blocco di memoria dal pool
    if(tx_block_allocate(&log_pool, (VOID**)&allocated_block, TX_NO_WAIT) == TX_SUCCESS)
    {
        // Formattiamo la stringa IN MODO SICURO gestendo correttamente float e 64-bit!
        va_list args;
        va_start(args, format);
        vsnprintf(allocated_block, LOG_BLOCK_SIZE, format, args);
        va_end(args);

        // Invia il PUNTATORE alla stringa già formattata alla coda
        if(tx_queue_send(&log_queue, &allocated_block, TX_NO_WAIT) != TX_SUCCESS)
            tx_block_release(allocated_block); // Coda piena, droppa il log e libera memoria
        else
            tx_event_flags_set(&logger_events, LOGGER_EVENT_MESSAGE, TX_OR);
    }
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





/*
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
*/

/*
void System_Log(const char* format, ...) 
{
    deferred_log_msg_t *msg = NULL;

    // Allocate a block for the struct
    if(tx_block_allocate(&log_pool, (VOID**)&msg, TX_NO_WAIT) == TX_SUCCESS)
    {
        msg->format = format; //store format string
        
        // Count required arguments by scanning for '%' (they could be less than the array size)
        int arg_count = 0;
        for (const char* p = format; *p != '\0'; p++) {
            if (*p == '%') {
                p++;
                if (*p != '%' && *p != '\0') { // Ignore "%%" and edge cases
                    arg_count++;
                }
            }
        }
        if (arg_count > MAX_LOG_ARGS) arg_count = MAX_LOG_ARGS; //ignore oversized argument counts

        // extract raw 32-bit arguments
        va_list args;
        va_start(args, format);
        for(int i = 0; i < arg_count; i++) {
            msg->args[i] = va_arg(args, uint32_t);
        }
        va_end(args);

        // Fill any unused arguments with 0 
        for(int i = arg_count; i < MAX_LOG_ARGS; i++) {
            msg->args[i] = 0;
        }

        // Send the struct pointer to the logger thread
        if(tx_queue_send(&log_queue, &msg, TX_NO_WAIT) != TX_SUCCESS) {
            tx_block_release(msg); // Drop log if queue is full
        } else {
            tx_event_flags_set(&logger_events, LOGGER_EVENT_MESSAGE, TX_OR);
        }
    }
}
*/