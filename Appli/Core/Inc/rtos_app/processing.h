#ifndef PROCESSING_THREAD_H
#define PROCESSING_THREAD_H

#include "tx_api.h"
#include "system_config.h"
#include <stdint.h>

/*
This structure maintains the parameters of the DSP.
It is set up by the system manager thread and used by DSP thread.
*/
//typedef struct {
//    uint32_t bp_low_wc; //first bandpass low cutoff
//    uint32_t bp_hig_wc; //first bandpass high cutoff
//    uint32_t lp_wc; //envelope low pass cutoff
//} processing_config_t;


UINT Processing_Init(void);

void Processing_Send_Config(processing_config_t* config);

void Processing_Trigger_Run(void);

#endif