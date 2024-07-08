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
#include "led.h"
#include "../LCD/lcd.h"

/**
 * @brief    Initialize the PVD voltage monitor
 * @param    pls : PWR_PVD_detection_level
 * @retval   None
 */
void pwr_pvd_init(uint32_t pls)
{
    PWR_PVDTypeDef pwr_pvd = {0};

    pwr_pvd.PVDLevel = pls;                          /* Detecting voltage level */
    pwr_pvd.Mode = PWR_PVD_MODE_IT_RISING_FALLING;   /* A double edge trigger using the rising
                                                      * and falling edge of the interrupt line is used */
    HAL_PWR_ConfigPVD(&pwr_pvd);

    HAL_NVIC_SetPriority(PVD_IRQn, 3 ,3);
    HAL_NVIC_EnableIRQ(PVD_IRQn);
    HAL_PWR_EnablePVD();                             /* Enable PVD detection */
}

/**
 * @brief    PVD interrupt service function
 * @param    None
 * @retval   None
 */
void PVD_IRQHandler(void)
{
    HAL_PWR_PVD_IRQHandler();
}

/**
 * @brief    PVD interrupt service callback function
 * @param    None
 * @retval   None
 */
void HAL_PWR_PVDCallback(void)
{
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_PVDO) != RESET)
    {
        /* If the voltage is lower than the set voltage level, prompt accordingly */
        LED1(0);
        lcd_show_string(30, 130, 200, 16, 16, "PVD Low Voltage!", RED);
    }
    else
    {
        /* The voltage is not lower than the set voltage level, and the corresponding prompt is given  */
        LED1(1);
        lcd_show_string(30, 130, 200, 16, 16, "PVD Voltage OK! ", BLUE);
    }
}


