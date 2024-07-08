/**
 ****************************************************************************************************
 * @file        sccb.c
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

#include "sccb.h"
#include "delay.h"

/**
 * @brief   SCCB��ʱ����
 * @note    ���ڿ���SCCBͨ������
 * @param   ��
 * @retval  ��
 */
static void sccb_delay(void)
{
    delay_us(50);
}

/**
 * @brief   ��ʼ��SCCB
 * @param   ��
 * @retval  ��
 */
void sccb_init(void)
{
    sccb_stop();
}

/**
 * @brief   ����SCCB��ʼ�ź�
 * @param   ��
 * @retval  ��
 */
void sccb_start(void)
{
    SCCB_SDA(1);
    SCCB_SCL(1);
    sccb_delay();
    SCCB_SDA(0);
    sccb_delay();
    SCCB_SCL(0);
}

/**
 * @brief   ����SCCBֹͣ�ź�
 * @param   ��
 * @retval  ��
 */
void sccb_stop(void)
{
    SCCB_SDA(0);
    sccb_delay();
    SCCB_SCL(1);
    sccb_delay();
    SCCB_SDA(1);
    sccb_delay();
}

/**
 * @brief   ����SCCB NACK�ź�
 * @param   ��
 * @retval  ��
 */
void sccb_nack(void)
{
    sccb_delay();
    SCCB_SDA(1);
    SCCB_SCL(1);
    sccb_delay();
    SCCB_SCL(0);
    sccb_delay();
    SCCB_SDA(0);
    sccb_delay();
}

/**
 * @brief   SCCB����һ���ֽ�
 * @param   data: �����͵�һ�ֽ�����
 * @retval  ���ͽ��
 * @arg     0: ���ͳɹ�
 * @arg     1: ����ʧ��
 */
uint8_t sccb_send_byte(uint8_t data)
{
    uint8_t t;
    uint8_t res;
    
    for (t=0; t<8; t++)
    {
        SCCB_SDA((data & 0x80) >> 7);
        sccb_delay();
        SCCB_SCL(1);
        sccb_delay();
        SCCB_SCL(0);
        data <<= 1;
    }
    SCCB_SDA(1);
    sccb_delay();
    
    SCCB_SCL(1);
    sccb_delay();
    if (SCCB_SDA_READ)
    {
        res = 1;
    }
    else
    {
        res = 0;
    }
    SCCB_SCL(0);
    
    return res;
}

/**
 * @brief   SCCB��ȡһ���ֽ�
 * @param   ��
 * @retval  ��ȡ����һ�ֽ�����
 */
uint8_t sccb_read_byte(void)
{
    uint8_t i;
    uint8_t receive = 0;
    
    for (i=0; i<8; i++)
    {
        receive <<= 1;
        SCCB_SCL(1);
        sccb_delay();
        
        if (SCCB_SDA_READ)
        {
            receive++;
        }
        
        SCCB_SCL(0);
        sccb_delay();
    }
    
    return receive;
}
