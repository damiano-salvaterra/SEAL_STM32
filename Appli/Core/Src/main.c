/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "gpdma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ADC_DMA_BUF_SIZE  4096
#define SAMPLE_WINDOW     10
/* USER CODE END PM */
extern DMA_HandleTypeDef handle_GPDMA1_Channel1 ;

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint32_t adc_dma_buffer[ADC_DMA_BUF_SIZE];
volatile uint16_t adc1_extracted[SAMPLE_WINDOW];
volatile uint16_t adc2_extracted[SAMPLE_WINDOW];
volatile uint8_t process_data_flag = 0; // 0 = Nothing, 1 = Data ready (Half), 2 = Data ready (Full)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemIsolation_Config(void);
/* USER CODE BEGIN PFP */
#if defined(__ICCARM__)
__ATTRIBUTES size_t __write(int, const unsigned char *, size_t);
#endif /* __ICCARM__ */

#if defined(__ICCARM__)
/* New definition from EWARM V9, compatible with EWARM8 */
int iar_fputc(int ch);
#define PUTCHAR_PROTOTYPE int iar_fputc(int ch)
#elif defined ( __CC_ARM ) || defined(__ARMCC_VERSION)
/* ARM Compiler 5/6*/
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#elif defined(__GNUC__)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#endif /* __ICCARM__ */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_TIM3_Init();
  SystemIsolation_Config();
  /* USER CODE BEGIN 2 */
  __enable_irq();

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADCEx_Calibration_Start(&hadc2,  ADC_SINGLE_ENDED);

  HAL_ADC_Start(&hadc2); //start slave

  if (HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc_dma_buffer, ADC_DMA_BUF_SIZE) != HAL_OK)
  {
      Error_Handler();
  }

  HAL_TIM_Base_Start(&htim3);

  printf("Application booted and initialized, entering Application main loop...\n\r");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    //HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
    //printf("Looping in application main Loop.\n\r");
	  //HAL_Delay(500);

    if (process_data_flag != 0)
      {
          uint8_t current_event = process_data_flag;
          process_data_flag = 0;

          if (current_event == 1) printf("--- HALF BUFFER (Last 10) ---\r\n");
          else                    printf("--- FULL BUFFER (Last 10) ---\r\n");

          printf("IDX | ADC1 (V) | ADC2 (V)\r\n");
          
          for (int i = 0; i < SAMPLE_WINDOW; i++)
          {
              //(0-1023 -> 0-1.8V). CHECK THIS, it should be 12 bits for each ADC
              float v1 = (adc1_extracted[i] * 1.8f) / 1023.0f;
              float v2 = (adc2_extracted[i] * 1.8f) / 1023.0f;

              printf("#%02d |  %.2f V  |  %.2f V\r\n", i, v1, v2);
          }
          printf("\r\n");
      }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_TIM;
  PeriphClkInitStruct.TIMPresSelection = RCC_TIMPRES_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RIF Initialization Function
  * @param None
  * @retval None
  */
  static void SystemIsolation_Config(void)
{

/* USER CODE BEGIN RIF_Init 0 */

/* USER CODE END RIF_Init 0 */

  /* set all required IPs as secure privileged */
  __HAL_RCC_RIFSC_CLK_ENABLE();

  /*RISUP configuration*/
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_ADC12 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_NPRIV);

  /* RIF-Aware IPs Config */

  /* set up GPDMA configuration */
  /* set GPDMA1 channel 1 used by ADC1 */
  if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel1,DMA_CHANNEL_SEC|DMA_CHANNEL_PRIV|DMA_CHANNEL_SRC_SEC|DMA_CHANNEL_DEST_SEC)!= HAL_OK )
  {
    Error_Handler();
  }

  /* set up GPIO configuration */
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);

/* USER CODE BEGIN RIF_Init 1 */

/* USER CODE END RIF_Init 1 */
/* USER CODE BEGIN RIF_Init 2 */

/* USER CODE END RIF_Init 2 */

}

/* USER CODE BEGIN 4 */
  /**
    * @brief  Retargets the C library printf function to the USART.
    * @param  None

    * @retval None
    */
  PUTCHAR_PROTOTYPE
  {
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART1 and Loop until the end of transmission */
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);

    return ch;
  }

  #if defined(__ICCARM__)
  size_t __write(int file, unsigned char const *ptr, size_t len)
  {
    size_t idx;
    unsigned char const *pdata = ptr;

    for (idx = 0; idx < len; idx++)
    {
      iar_fputc((int)*pdata);
      pdata++;
    }
    return len;
  }
  #endif /* __ICCARM__ */






/**
 * Internal helper function to unpack data.
 * start_idx: starting index to read from the DMA buffer.
 */
void Extract_ADC_Data(uint32_t start_idx)
{
    for (int i = 0; i < SAMPLE_WINDOW; i++)
    {
        uint32_t raw_index = start_idx + i;
        uint32_t raw_data = adc_dma_buffer[raw_index];

        // EXTRACTION (Bitmasking for 10-bit packed mode)
        // ADC1 (Master) -> Bits 0-9
        // ADC2 (Slave)  -> Bits 16-25 (usually shifted by 16)
        adc1_extracted[i] = (uint16_t)(raw_data & 0x03FF);
        adc2_extracted[i] = (uint16_t)((raw_data >> 16) & 0x03FF);
    }
}

// Called when the buffer is half full (samples 0 -> 2047 are ready)
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    // Execute only for ADC1 (the DMA master)
    if (hadc->Instance == ADC1)
    {
        // We want the last 10 samples of the FIRST half.
        // End of first half = index 2048.
        // Start = 2048 - 10 = 2038.
        Extract_ADC_Data((ADC_DMA_BUF_SIZE / 2) - SAMPLE_WINDOW);
        process_data_flag = 1; // Signal to main: "Half buffer data ready"
    }
}

// Called when the buffer is completely full (samples 2048 -> 4095 are ready)
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        // We want the last 10 samples of the SECOND half (end of buffer).
        // End of buffer = index 4096.
        // Start = 4096 - 10 = 4086.
        Extract_ADC_Data(ADC_DMA_BUF_SIZE - SAMPLE_WINDOW);
        process_data_flag = 2; // Signal to main: "End of buffer data ready"
    }
}









  
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    printf("APPLICATION ERROR\n\r");
    HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
