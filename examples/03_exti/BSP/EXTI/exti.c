/**
 ****************************************************************************************************
 * @file        exti.c
 * @author      ALIENTEK
 * @brief       exti code
 * @license     Copyright (C) 2012-2024, ALIENTEK
 ****************************************************************************************************
 * @attention
 *
 * platform     : ALIENTEK M100-STM32F407 board
 * website      : www.alientek.com
 * forum        : www.openedv.com/forum.php
 *
 * change logs  ：
 * version      data         notes
 * V1.0         20240409     the first version
 *
 ****************************************************************************************************
 */

#include "exti.h"
#include "../KEY/key.h"
#include "../LED/led.h"


/**
  * @brief  EXTI line rising detection callback.
  * @param  GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    HAL_Delay(20);       	   /* delay 20ms to eliminate chattering */

    switch (GPIO_Pin)
    {
        case KEY0_Pin:
        	LED0_TOGGLE();     /* LED0 state is flipped */
        	break;
        case WK_UP_Pin:
            LED1_TOGGLE();     /* LED1 state is flipped */
        	break;
    }
}

