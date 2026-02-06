/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined ( __ICCARM__ )
#  define CMSE_NS_CALL  __cmse_nonsecure_call
#  define CMSE_NS_ENTRY __cmse_nonsecure_entry
#else
#  define CMSE_NS_CALL  __attribute((cmse_nonsecure_call))
#  define CMSE_NS_ENTRY __attribute((cmse_nonsecure_entry))
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32n6xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* Function pointer declaration in non-secure*/
#if defined ( __ICCARM__ )
typedef void (CMSE_NS_CALL *funcptr)(void);
#else
typedef void CMSE_NS_CALL (*funcptr)(void);
#endif

/* typedef for non-secure callback functions */
typedef funcptr funcptr_NS;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void button_handler(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BOOTN_Pin GPIO_PIN_4
#define BOOTN_GPIO_Port GPIOC
#define INTN_Pin GPIO_PIN_5
#define INTN_GPIO_Port GPIOC
#define INTN_EXTI_IRQn EXTI5_IRQn
#define PS0_WAKEN_Pin GPIO_PIN_0
#define PS0_WAKEN_GPIO_Port GPIOC
#define USART_CONSOLE_TX_Pin GPIO_PIN_5
#define USART_CONSOLE_TX_GPIO_Port GPIOE
#define PS1_Pin GPIO_PIN_2
#define PS1_GPIO_Port GPIOC
#define CLKSEL0_Pin GPIO_PIN_4
#define CLKSEL0_GPIO_Port GPIOE
#define USART_CONSOLE_RX_Pin GPIO_PIN_6
#define USART_CONSOLE_RX_GPIO_Port GPIOE
#define USART_BNO_TX_Pin GPIO_PIN_5
#define USART_BNO_TX_GPIO_Port GPIOD
#define RSTN_Pin GPIO_PIN_3
#define RSTN_GPIO_Port GPIOC
#define USR_BTN_Pin GPIO_PIN_13
#define USR_BTN_GPIO_Port GPIOC
#define USR_BTN_EXTI_IRQn EXTI13_IRQn
#define USART_BNO_RX_Pin GPIO_PIN_6
#define USART_BNO_RX_GPIO_Port GPIOF
#define GREEN_LED_Pin GPIO_PIN_0
#define GREEN_LED_GPIO_Port GPIOG
#define RED_LED_Pin GPIO_PIN_10
#define RED_LED_GPIO_Port GPIOG
#define BLUE_LED_Pin GPIO_PIN_8
#define BLUE_LED_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
