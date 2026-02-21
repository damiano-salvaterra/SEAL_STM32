/*
 * Copyright 2020-21 CEVA, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License and 
 * any applicable agreements you may have with CEVA, Inc.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "main.h"

#include "rvc_hal.h"

#include <stdbool.h>
#include <string.h>
#include "rvc.h"

#include "stm32n6xx_hal.h"

#include "usart.h"

// -------------------------------------------------------------------------
// Constants

#define RVC_BPS (115200)         // 115200 bps for RVC mode

// Keep reset asserted this long.
// (Some targets have a long RC decay on reset.)
#define RESET_DELAY_US (10000)

// ------------------------------------------------------------------------
// Private data

static bool isOpen = false;

// True between asserting reset and seeing first data from sensor hub
static volatile bool inReset;

// Timer handle
extern TIM_HandleTypeDef htim2;

// USART1 handle
extern UART_HandleTypeDef huart2;

// DMA stream for USART1 Rx.
extern DMA_HandleTypeDef handle_GPDMA1_Channel0;

// receive support
#define DMA_SIZE (256)  // Must be a power of 2
static uint8_t rxBuffer[DMA_SIZE]; // receives UART data via DMA (must be a power of 2)
static uint32_t rxIndex = 0;               // next index to read

// RVC frame
#define RVC_HEADER    ( 0)
#define RVC_INDEX     ( 2)
#define RVC_YAW_LSB   ( 3)
#define RVC_YAW_MSB   ( 4)
#define RVC_PITCH_LSB ( 5)
#define RVC_PITCH_MSB ( 6)
#define RVC_ROLL_LSB  ( 7)
#define RVC_ROLL_MSB  ( 8)
#define RVC_ACC_X_LSB ( 9)
#define RVC_ACC_X_MSB (10)
#define RVC_ACC_Y_LSB (11)
#define RVC_ACC_Y_MSB (12)
#define RVC_ACC_Z_LSB (13)
#define RVC_ACC_Z_MSB (14)
#define RVC_MI        (15)
#define RVC_MR        (16)
#define RVC_RESERVED  (17)
#define RVC_CSUM      (18)

#define RVC_FRAME_LEN (19)
static uint8_t rxFrame[RVC_FRAME_LEN];  // receives frame from sensor hub
static uint32_t rxFrameLen = 0;         // length of frame so far
static bool rxFrameReady = false;       // set true when a full, valid frame received.


// ------------------------------------------------------------------------
// Private methods

static void disableInts(void)
{
    HAL_NVIC_DisableIRQ(GPDMA1_Channel0_IRQn);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
}

static void enableInts(void)
{
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);
}


static void rstn(bool state)
{
    HAL_GPIO_WritePin(RSTN_GPIO_Port, RSTN_Pin, 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void ps0_waken(bool state)
{
    HAL_GPIO_WritePin(PS0_WAKEN_GPIO_Port, PS0_WAKEN_Pin, 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void ps1(bool state)
{
    HAL_GPIO_WritePin(PS1_GPIO_Port, PS1_Pin, 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void bootn(bool state)
{
    HAL_GPIO_WritePin(BOOTN_GPIO_Port, BOOTN_Pin, 
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint32_t timeNowUs(void)
{
    return __HAL_TIM_GET_COUNTER(&htim2);
}

static uint64_t timeNowUs_64(void)
{
    static uint32_t last = 0;
    static uint32_t rollovers = 0;
    
    uint32_t now = timeNowUs();
    if (now < last) {
        rollovers++;
    }
    
    last = now;

    return ((uint64_t)rollovers << 32) | now;
}

// delay for t microseconds
static void delay_us(uint32_t t)
{
    uint32_t start, now;

    now = timeNowUs();
    start = now;
    while ((now - start) < t) {
        now = timeNowUs();
    }
}

static bool checksumOk()
{
    uint8_t check = 0;

    for (int n = 2; n < RVC_CSUM; n++) {
        check += rxFrame[n];
    }

    return (check == rxFrame[RVC_CSUM]);
}

// Process one character of an RVC frame
void rx(uint8_t c)
{
    if (rxFrameLen == RVC_FRAME_LEN) {
        // rxFrame has filled but we still don't have a valid frame.
        // (a checksum error?  Or bad frame sync?)  Anyways, shift data
        // into the rxFrame buffer until we have a good, whole frame
        for (int n = 0; n < RVC_FRAME_LEN-1; n++) {
            rxFrame[n] = rxFrame[n+1];
        }
        rxFrame[RVC_FRAME_LEN-1] = c;
    }
    else {
        // Just stuff the latest char into the buffer at the end
        rxFrame[rxFrameLen++] = c;
    }

    // If rx buffer is full, see if we have a valid frame
    if ((rxFrameLen == RVC_FRAME_LEN) &&
        (rxFrame[0] == 0xAA) &&
        (rxFrame[1] == 0xAA) &&
        checksumOk()) {
        // It's a good frame
        rxFrameReady = true;
    }
}


// -----------------------------------------------------------------------
// Public methods

int rvc_hal_open(void)
{
    if (isOpen) {
        // It's an error to open an already-open RVC device
        return RVC_ERR;
    }
    
    // disable interrupts
    disableInts();


    
    // reset part
    rstn(false);
    inReset = true;
    
    // delay to ensure reset hold time
    delay_us(RESET_DELAY_US);
    
    // re-enable interrupts
    enableInts();
    
    // start data flowing through UART
    for (int n = 0; n < sizeof(rxBuffer); n++) {
        rxBuffer[n] = 0xAA;
    }
    HAL_UART_Receive_DMA(&huart2, rxBuffer, sizeof(rxBuffer));
    
    // set PS0, PS1, BOOTN to boot into RVC mode
    // To boot in RVC mode, must have PS1=0, PS0=1.
    // PS1 is set via jumper.
    // PS0 will be 0 if PS0 jumper is 0 OR (PS1 jumper is 1 AND PS0_WAKEN sig is 0)
    // So we set PS0_WAKEN signal to 0 just in case PS1 jumper is in 1 position.
    ps0_waken(true);   
    ps1(false);
    bootn(true);

    // deassert reset
    rstn(true);
    inReset = false;
    
    isOpen = true;

    return RVC_OK;
}


void rvc_hal_close(void)
{
    // disable interrupts
    disableInts();
    
    // hold device in reset
    rstn(false);
    inReset = true;
    
    // disable USART, DMA
    __HAL_UART_DISABLE(&huart2);
    __HAL_DMA_DISABLE(&handle_GPDMA1_Channel0);

    // disable timer
    __HAL_TIM_DISABLE(&htim2);
    
    isOpen = false;
}

int rvc_hal_read(rvc_SensorEvent_t *pEvent)
{
    int retval = 0;

    // DMA has received characters up to this point in rxBuffer
    //uint32_t stopPoint = sizeof(rxBuffer)-__HAL_DMA_GET_COUNTER(&handle_GPDMA1_Channel0);
    uint32_t stopPoint = (sizeof(rxBuffer) - __HAL_DMA_GET_COUNTER(&handle_GPDMA1_Channel0)) & (sizeof(rxBuffer)-1);
    // Process data until we've seen everything or we have a fully formed
    // RVC frame in rxFrame.
    while ((rxIndex != stopPoint) && !rxFrameReady) {
        // process one input character
        rx(rxBuffer[rxIndex]);
        rxIndex = (rxIndex + 1) & (sizeof(rxBuffer)-1);
    }

    if (rxFrameReady) {
        // copy data into pBuffer
        pEvent->timestamp_uS = timeNowUs_64();
        pEvent->index = rxFrame[RVC_INDEX];
        pEvent->yaw =   (int16_t)((rxFrame[RVC_YAW_MSB] << 8) + rxFrame[RVC_YAW_LSB]);
        pEvent->pitch = (int16_t)((rxFrame[RVC_PITCH_MSB] << 8) + rxFrame[RVC_PITCH_LSB]);
        pEvent->roll =  (int16_t)((rxFrame[RVC_ROLL_MSB] << 8) + rxFrame[RVC_ROLL_LSB]);
        pEvent->acc_x = (int16_t)((rxFrame[RVC_ACC_X_MSB] << 8) + rxFrame[RVC_ACC_X_LSB]);
        pEvent->acc_y = (int16_t)((rxFrame[RVC_ACC_Y_MSB] << 8) + rxFrame[RVC_ACC_Y_LSB]);
        pEvent->acc_z = (int16_t)((rxFrame[RVC_ACC_Z_MSB] << 8) + rxFrame[RVC_ACC_Z_LSB]);
        pEvent->mi =    (uint8_t)(rxFrame[RVC_MI]);
        pEvent->mr =    (uint8_t)(rxFrame[RVC_MI]);

        retval = 1;

        // reset the rx frame
        rxFrameReady = false;
        rxFrameLen = 0;
    }
    
    return retval;
}

