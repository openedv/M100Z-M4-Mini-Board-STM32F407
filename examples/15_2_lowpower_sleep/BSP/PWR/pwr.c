/**
 ****************************************************************************************************
 * @file        pwr.c
 * @author      ALIENTEK
 * @brief       pwr code
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

#include "pwr.h"


/**
 * @brief   Enter CPU sleep mode
 * @param   None
 * @retval  None
 */
void pwr_enter_sleep(void)
{
	HAL_SuspendTick();  /* Pause the tick clock to prevent wakeup from being interrupted by the tick clock */
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); /* Execute the WFI command and enter the sleep mode */
}

/**
 * @brief    External interrupt callback function
 * @param    GPIO Pin : Interrupt pin
 * @note     This function is called by PWR WKUP INT IRQHandler()
 * @retval   None
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == WK_UP_Pin)
    {
        /* The HAL_GPIO_EXTI_IRQHandler() function has cleared the interrupt flag for us,
         *  so we can call the callback without doing anything */
    }
}
