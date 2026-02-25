#ifndef DSP_THREAD_H
#define DSP_THREAD_H

#include <stdint.h>
#include "tx_api.h"

/*
This structure maintains the parameters of the DSP.
It is set up by the system manager thread and used by DSP thread.
*/
typedef struct {
    uint32_t bp_low_wc; //first bandpass low cutoff
    uint32_t bp_hig_wc; //first bandpass high cutoff
    uint32_t lp_wc; //envelope low pass cutoff
} DSP_config_t;

#endif