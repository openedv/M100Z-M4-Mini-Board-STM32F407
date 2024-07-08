/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */
/* Struct variable definition */
typedef struct
{
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    /* Calendar year, month, day and week */
    uint16_t year;
    uint8_t  month;
    uint8_t  date;
    uint8_t  week;
} _calendar_obj;
extern _calendar_obj g_calendar;
/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
void rtc_write_bkr(uint32_t bkrx, uint16_t data);
uint16_t rtc_read_bkr(uint32_t bkrx);
uint8_t rtc_set_time(uint8_t hour, uint8_t minute, uint8_t second, uint8_t ampm);
void rtc_set_alarm(uint8_t week, uint8_t hour, uint8_t minute, uint8_t second);
void rtc_get_time(uint8_t *hour, uint8_t *minute, uint8_t *second, uint8_t *ampm);
void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week);
uint8_t rtc_get_week(uint16_t year, uint8_t month, uint8_t date);
void rtc_set_wakeup(uint8_t clock, uint8_t count);
uint8_t rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

