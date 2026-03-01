#ifndef SYS_MANAGER_H
#define SYS_MANAGER_H

#include "tx_api.h"
#include "system_config.h"

UINT SystemManager_Init(void);

void System_Send_Config(SystemConfig_t* config);

#endif