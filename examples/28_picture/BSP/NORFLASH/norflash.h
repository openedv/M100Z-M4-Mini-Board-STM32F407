/**
 ****************************************************************************************************
 * @file        norflash.h
 * @author      ALIENTEK
 * @brief       norflash code
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

#ifndef NORFLASH_NORFLASH_H_
#define NORFLASH_NORFLASH_H_

/* NORFLASH slice selection signal */
#define NORFLASH_CS(x)      do{ x ? \
                                  HAL_GPIO_WritePin(NORFLASH_CS_GPIO_Port, NORFLASH_CS_Pin, GPIO_PIN_SET) : \
                                  HAL_GPIO_WritePin(NORFLASH_CS_GPIO_Port, NORFLASH_CS_Pin, GPIO_PIN_RESET); \
                            }while(0)

/* FLASH Chip List */
#define W25Q80      0XEF13
#define W25Q16      0XEF14
#define W25Q32      0XEF15
#define W25Q64      0XEF16
#define W25Q128     0XEF17
#define W25Q256     0XEF18
#define BY25Q64     0X6816
#define BY25Q128    0X6817
#define NM25Q64     0X5216
#define NM25Q128    0X5217

extern uint16_t norflash_TYPE;      /* Define the FLASH chip model */

/* instruction list */
#define FLASH_WriteEnable           0x06
#define FLASH_WriteDisable          0x04
#define FLASH_ReadStatusReg1        0x05
#define FLASH_ReadStatusReg2        0x35
#define FLASH_ReadStatusReg3        0x15
#define FLASH_WriteStatusReg1       0x01
#define FLASH_WriteStatusReg2       0x31
#define FLASH_WriteStatusReg3       0x11
#define FLASH_ReadData              0x03
#define FLASH_FastReadData          0x0B
#define FLASH_FastReadDual          0x3B
#define FLASH_FastReadQuad          0xEB
#define FLASH_PageProgram           0x02
#define FLASH_PageProgramQuad       0x32
#define FLASH_BlockErase            0xD8
#define FLASH_SectorErase           0x20
#define FLASH_ChipErase             0xC7
#define FLASH_PowerDown             0xB9
#define FLASH_ReleasePowerDown      0xAB
#define FLASH_DeviceID              0xAB
#define FLASH_ManufactDeviceID      0x90
#define FLASH_JedecDeviceID         0x9F
#define FLASH_Enable4ByteAddr       0xB7
#define FLASH_Exit4ByteAddr         0xE9
#define FLASH_SetReadParam          0xC0
#define FLASH_EnterQPIMode          0x38
#define FLASH_ExitQPIMode           0xFF


void norflash_init(void);
uint16_t norflash_read_id(void);
void norflash_write_enable(void);
uint8_t norflash_read_sr(uint8_t regno);
void norflash_write_sr(uint8_t regno,uint8_t sr);

void norflash_erase_chip(void);
void norflash_erase_sector(uint32_t saddr);
void norflash_read(uint8_t *pbuf, uint32_t addr, uint16_t datalen);
void norflash_write(uint8_t *pbuf, uint32_t addr, uint16_t datalen);

#endif /* NORFLASH_NORFLASH_H_ */
