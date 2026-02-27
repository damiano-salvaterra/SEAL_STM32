#include "processing.h"
#include "sys_logger.h"
#include "tx_api.h"
#include "imu.h"


#define PROCESSING_STACK_SIZE 4096

#define PROCESSING_QUEUE_CAPACITY 2
#define PROCESSING_MESSAGE_SIZE   TX_2_ULONG //TODO: to be modified when actually implemented
#define PROCESSING_QUEUE_MEM_SIZE (8 * PROCESSING_QUEUE_CAPACITY)

#define PROCESSING_EVENT_CONFIG   (1UL << 0)
#define PROCESSING_EVENT_RUN      (1UL << 1)

TX_EVENT_FLAGS_GROUP processing_events;
TX_QUEUE processing_thread_queue;
TX_THREAD processing_thread;

ULONG processing_thread_queue_memory[PROCESSING_QUEUE_MEM_SIZE / sizeof(ULONG)];
ULONG processing_thread_stack[PROCESSING_STACK_SIZE / sizeof(ULONG)];



VOID processing_thread_entry(ULONG initial_input)
{
    TX_PARAMETER_NOT_USED(initial_input);

    processing_config_t current_config;
    ULONG flags = 0;

    //clear config
    memset(&current_config, 0, sizeof(current_config));

    while(1)
    {
        tx_event_flags_get(&processing_events, PROCESSING_EVENT_CONFIG | PROCESSING_EVENT_RUN,
                            TX_OR_CLEAR, &flags, TX_WAIT_FOREVER);

        if(flags & PROCESSING_EVENT_CONFIG)
        {
            //drain the queue totally
            while(tx_queue_receive(&processing_thread_queue, &current_config, TX_NO_WAIT)==TX_SUCCESS)
            {
                System_Log(
                "[ProcessingThread] INFO: new DSP configuration received\r\n"
                "                   New Config    -> foo: %d, bar: %d\r\n",
                current_config.foo,
                current_config.bar
                );
            }
        }

        if (flags & PROCESSING_EVENT_RUN)
        {
            System_Log("[ProcessingThread] INFO: processing trigger received.\n\r");
            imu_get_data();
        }
                

    }

}



UINT Processing_Init(void)
{
    //create event flags to separate configurations from processing triggers
    tx_event_flags_create(&processing_events, "ProcessingEvents");

    //create queue to send configurations
    tx_queue_create(&processing_thread_queue, "Processing_Queue",  PROCESSING_MESSAGE_SIZE, processing_thread_queue_memory, PROCESSING_QUEUE_MEM_SIZE);
    //create thread
    tx_thread_create(&processing_thread, "Processing_Thread", processing_thread_entry, 0, processing_thread_stack, PROCESSING_STACK_SIZE, 6, 6, 1, TX_AUTO_START);

    return TX_SUCCESS;

}



void Processing_Send_Config(processing_config_t* config)
{
    tx_queue_send(&processing_thread_queue, config, TX_NO_WAIT);

    //this thread waits on the event flags
    tx_event_flags_set(&processing_events, PROCESSING_EVENT_CONFIG, TX_OR);
}

void Processing_Trigger_Run(void){
    tx_event_flags_set(&processing_events, PROCESSING_EVENT_RUN, TX_OR);

}
