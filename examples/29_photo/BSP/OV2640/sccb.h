/**
 ****************************************************************************************************
 * @file        sccb.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2023-04-23
 * @brief       SCCB��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 * 
 * ʵ��ƽ̨:����ԭ�� M100Z-M4��Сϵͳ��STM32F407��
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 * 
 ****************************************************************************************************
 */

#ifndef __SCCB_H
#define __SCCB_H

#include "main.h"


#define SCCB_SCL(x)                 do { (x) ?                                                                      \
                                        HAL_GPIO_WritePin(SCCB_SCL_GPIO_Port, SCCB_SCL_Pin, GPIO_PIN_SET):     \
                                        HAL_GPIO_WritePin(SCCB_SCL_GPIO_Port, SCCB_SCL_Pin, GPIO_PIN_RESET);   \
                                    } while (0)
#define SCCB_SDA(x)                 do { (x) ?                                                                      \
                                        HAL_GPIO_WritePin(SCCB_SDA_GPIO_Port, SCCB_SDA_Pin, GPIO_PIN_SET):     \
                                        HAL_GPIO_WritePin(SCCB_SDA_GPIO_Port, SCCB_SDA_Pin, GPIO_PIN_RESET);   \
                                    } while (0)
#define SCCB_SDA_READ               ((HAL_GPIO_ReadPin(SCCB_SDA_GPIO_Port, SCCB_SDA_Pin) == GPIO_PIN_RESET) ? 0 : 1)


void sccb_init(void);
void sccb_start(void);
void sccb_stop(void);
void sccb_nack(void);
uint8_t sccb_send_byte(uint8_t data);
uint8_t sccb_read_byte(void);

#endif
