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
 * @brief   Enter CPU stop mode
 * @param   None
 * @retval  None
 */
void pwr_enter_stop(void)
{
	/* Turn off the SysTick interrupt */
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); /* Execute the WFI command and enter the stop mode */
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
    	/* Restart the SysTick interrupt */
        SysTick->CTRL  |= SysTick_CTRL_TICKINT_Msk;
    }
}
