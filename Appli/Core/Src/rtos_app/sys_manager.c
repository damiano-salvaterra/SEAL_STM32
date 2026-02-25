#include "sys_manager.h"
#include "system_config.h"
#include "bno08x_service.h"
#include "sys_logger.h"

VOID sys_manager_thread_entry(ULONG initial_input)
{
    SystemConfig_t *config = (SystemConfig_t*)initial_input;

    if(config == NULL)
    {
        System_Log("[SysManager] ERROR: Null config passed.\n\r");
    }

}