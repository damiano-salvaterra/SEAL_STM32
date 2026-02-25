#include "hw_timers.h"
#include "stm32n6xx_hal_def.h"


bool timer_imu_start()
{
    HAL_StatusTypeDef ret = HAL_TIM_Base_Start(&htim2);

    if (ret != HAL_OK)
        return false;
    return true;
}