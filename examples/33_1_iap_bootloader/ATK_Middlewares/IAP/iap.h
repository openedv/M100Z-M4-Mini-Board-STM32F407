/**
 ****************************************************************************************************
 * @file        iap.h
 * @author      ALIENTEK
 * @brief       IAP code
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

#ifndef __IAP_H
#define __IAP_H

#include "main.h"



typedef void (*iapfun)(void);                   /* Define an argument of type function */

#define FLASH_APP1_ADDR         0x08010000

void iap_load_app(uint32_t appxaddr);                                         /* Jump to APP program execution */
void iap_write_appbin(uint32_t appxaddr, uint8_t *appbuf, uint32_t applen);   /* At the specified address, write to bin */

#endif







































