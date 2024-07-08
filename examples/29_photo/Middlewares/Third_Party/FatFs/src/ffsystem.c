/**
 ****************************************************************************************************
 * @file        ffsystem.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-01-14
 * @brief       FATFS�ײ�(ffsystem) ��������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� M144Z-M4��Сϵͳ��STM32F407��
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20220114
 * ��һ�η���
 *
 ****************************************************************************************************
 */

#include "../../ATK_Middlewares/MALLOC/malloc.h"
#include "ff.h"


/**
 * @brief       ��̬�����ڴ�
 * @param       size : Ҫ������ڴ��С(�ֽ�)
 * @retval      ���䵽���ڴ��׵�ַ.
 */
void *ff_memalloc (UINT size)
{
    return (void*)mymalloc(SRAMIN,size);
}

/**
 * @brief       �ͷ��ڴ�
 * @param       mf  : �ڴ��׵�ַ
 * @retval      ��
 */
void ff_memfree (void* mf)
{
    myfree(SRAMIN,mf);
}

















