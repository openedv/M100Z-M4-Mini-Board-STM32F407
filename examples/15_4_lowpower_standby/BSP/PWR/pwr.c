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
 * change logs  :
 * version      data         notes
 * V1.0         20240409     the first version
 *
 ****************************************************************************************************
 */

#include "pwr.h"


/**
 * @brief   Enter CPU standby mode
 * @param   None
 * @retval  None
 */
void pwr_enter_standby(void)
{
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);  /* Enable the wake-up function of the WK_UP pin */
	/* Clear the standby mode flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	HAL_PWR_EnterSTANDBYMode();                /* Enter standby mode */
}
