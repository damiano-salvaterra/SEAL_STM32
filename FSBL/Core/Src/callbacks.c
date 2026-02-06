#include "main.h"
#include "stm32n6xx_hal_gpio.h"
#include "uart_hal.h"
#include "stm32n657xx.h"
#include "stdio.h"

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) { 
        notify_uart_error();
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) { // O USART1
        // Gestione specifica per il modo DFU
        notify_tx_cplt();
    }
}


void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == INTN_Pin)
    {
        notify_bno_interrupt_falling();
    }
    //else if(GPIO_Pin == USR_BTN_Pin)
    //{
    //    button_handler();
    //}
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
    if(GPIO_Pin == USR_BTN_Pin)
    {
        button_handler();
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    // Ignore
}
