#include "main.h"
#include "stm32n6xx_hal_gpio.h"
#include "stm32n657xx.h"
#include "stdio.h"
#include "tx_api.h"

extern TX_SEMAPHORE bno_service_semaphore;

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
 
}


void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
    if(GPIO_Pin == USR_BTN_Pin)
    {
        //tx_semaphore_put(&bno_service_semaphore);
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    // Ignore
}

