/**
 ****************************************************************************************************
 * @file        norflash.c
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

#include "spi.h"
#include "usart.h"
#include "norflash.h"


static void norflash_wait_busy(void);
static void norflash_send_address(uint32_t address);
static void norflash_write_page(uint8_t *pbuf, uint32_t addr, uint16_t datalen);
static void norflash_write_nocheck(uint8_t *pbuf, uint32_t addr, uint16_t datalen);

uint16_t g_norflash_type;

/**
 * @brief       Initialize the SPI NOR FLASH
 * @param       None
 * @retval      None
 */
void norflash_init(void)
{
    uint8_t temp;

    NORFLASH_CS(1);                      /* Unselect a movie */

    MX_SPI2_Init();                   	 /* Initialize SPI2 */
    spi2_set_speed(SPI_BAUDRATEPRESCALER_4);/* SPI2 switches to a high-speed state of 18Mhz */

    g_norflash_type = norflash_read_id();/* Reading FLASH ID */
    printf("ID:%x\r\n",g_norflash_type);
    if (g_norflash_type == W25Q256)      /* The SPI FLASH is W25Q256 and must enable 4-byte address mode */
    {
        temp = norflash_read_sr(3);      /* Read status register 3 to determine the address pattern */

        if ((temp & 0X01) == 0)          /* If it is not 4-byte address mode, it enters 4-byte address mode */
        {
            norflash_write_enable();     /* Write enable */
            temp |= 1 << 1;              /* ADP=1, power on 4-bit address mode */
            norflash_write_sr(3, temp);  /* Write SR3 */

            NORFLASH_CS(0);
            spi2_read_write_byte(FLASH_Enable4ByteAddr);    /* Enable 4-byte address instructions */
            NORFLASH_CS(1);
        }
    }
}

/**
 * @brief       Wait to be idle
 * @param       None
 * @retval      None
 */
static void norflash_wait_busy(void)
{
    while ((norflash_read_sr(1) & 0x01) == 0x01);   /* Wait for the BUSY bit to empty */
}

/**
 * @brief       25QXX write enabled
 * @param       None
 * @retval      None
 */
void norflash_write_enable(void)
{
    NORFLASH_CS(0);
    spi2_read_write_byte(FLASH_WriteEnable);   /* Send write enable */
    NORFLASH_CS(1);
}

/**
 * @brief       25QXX send address
 * @param       address : The address to send to
 * @retval      None
 */
static void norflash_send_address(uint32_t address)
{
    if (g_norflash_type == W25Q256)                     /*  Only W25Q256 supports 4-byte address mode */
    {
        spi2_read_write_byte((uint8_t)((address)>>24)); /* Send bit31 to bit24 address */
    }
    spi2_read_write_byte((uint8_t)((address)>>16));     /* Send bit23 to bit16 address */
    spi2_read_write_byte((uint8_t)((address)>>8));      /* Send bit15 to bit8  address */
    spi2_read_write_byte((uint8_t)address);             /* Send bit7  to bit0  address */
}

/**
 * @brief       Read the status register of 25QXX. There are three status registers in 25QXX
 * @param       regno: Status register number, range :1 to 3
 * @retval      Status register value
 */
uint8_t norflash_read_sr(uint8_t regno)
{
    uint8_t byte = 0, command = 0;

    switch (regno)
    {
        case 1:
            command = FLASH_ReadStatusReg1;  /* Read status register 1 instruction */
            break;

        case 2:
            command = FLASH_ReadStatusReg2;  /* Read status register 2 instruction */
            break;

        case 3:
            command = FLASH_ReadStatusReg3;  /* Read status register 3 instruction */
            break;

        default:
            command = FLASH_ReadStatusReg1;
            break;
    }

    NORFLASH_CS(0);
    spi2_read_write_byte(command);      	/* Send the read register command */
    byte = spi2_read_write_byte(0Xff);  	/* Reading a byte */
    NORFLASH_CS(1);

    return byte;
}

/**
 * @brief   writes 25QXX status register
 * @param   regno : Status register number, range :1 to 3
 * @param   sr    : The value to write to the status register
 * @retval  None
 */
void norflash_write_sr(uint8_t regno, uint8_t sr)
{
    uint8_t command = 0;

    switch (regno)
    {
        case 1:
            command = FLASH_WriteStatusReg1;  /* Write status register 1 instruction */
            break;

        case 2:
            command = FLASH_WriteStatusReg2;  /* Write status register 2 instruction */
            break;

        case 3:
            command = FLASH_WriteStatusReg3;  /* Write status register 3 instruction */
            break;

        default:
            command = FLASH_WriteStatusReg1;
            break;
    }

    NORFLASH_CS(0);
    spi2_read_write_byte(command);  /* Send the read register command */
    spi2_read_write_byte(sr);       /* Writing a byte */
    NORFLASH_CS(1);
}

/**
 * @brief  reads the chip ID
 * @param  None
 * @retval FLASH chip ID
 */
uint16_t norflash_read_id(void)
{
    uint16_t deviceid;

    NORFLASH_CS(0);
    spi2_read_write_byte(FLASH_ManufactDeviceID);   /* Send read ID command */
    spi2_read_write_byte(0);    					/* Writing a byte */
    spi2_read_write_byte(0);
    spi2_read_write_byte(0);
    deviceid = spi2_read_write_byte(0xFF) << 8;     /* Read the high 8-bit bytes */
    deviceid |= spi2_read_write_byte(0xFF);         /* Read the low 8-bit bytes */
    NORFLASH_CS(1);

    return deviceid;
}

/**
 * @brief   reads SPI FLASH
 * @param   pbuf    : The data store
 * @param   addr    : The address to start reading (Max. 32 bits)
 * @param   datalen : Number of bytes to read (Max 65535)
 * @retval  None
 */
void norflash_read(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint16_t i;

    NORFLASH_CS(0);
    spi2_read_write_byte(FLASH_ReadData);       /* Send read command */
    norflash_send_address(addr);                /* send address */

    for(i=0;i<datalen;i++)
    {
        pbuf[i] = spi2_read_write_byte(0XFF);   /* Cyclic reading */
    }

    NORFLASH_CS(1);
}

/**
 * @brief    SPI writes less than 256 bytes in a page (0 to 65535)
 * @param    pbuf    : The data store
 * @param    addr    : The address to start writing (Max. 32 bits)
 * @param    datalen : The number of bytes to write (Max 256), it should not exceed the remaining bytes of the page!!
 * @retval   None
 */
static void norflash_write_page(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint16_t i;

    norflash_write_enable();    				/* Write enable */

    NORFLASH_CS(0);
    spi2_read_write_byte(FLASH_PageProgram);    /* Send the write page command */
    norflash_send_address(addr);                /* send address */

    for(i=0;i<datalen;i++)
    {
        spi2_read_write_byte(pbuf[i]);          /* Cyclic write */
    }

    NORFLASH_CS(1);
    norflash_wait_busy();       				/* Waiting for the write to finish */
}

/**
 * @brief   write SPI FLASH without checking
 * @param   pbuf    : The data store
 * @param   addr    : The address to start writing (Max. 32 bits)
 * @param   datalen : Number of bytes to write (Max 65535)
 * @retval  None
 */
static void norflash_write_nocheck(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint16_t pageremain;
    pageremain = 256 - addr % 256;  /* The number of bytes remaining in a page */

    if (datalen <= pageremain)      /* No more than 256 bytes */
    {
        pageremain = datalen;
    }

    while (1)
    {
        norflash_write_page(pbuf, addr, pageremain);

        if (datalen == pageremain)
        {
            break;
        }
        else     /* datalen > pageremain */
        {
            pbuf += pageremain;         /* pbuf pointer address offset; pageremain bytes have already been written */
            addr += pageremain;         /* Write address offset; pageremain bytes have already been written */
            datalen -= pageremain;      /* The total length of the write minus the number of bytes written */

            if (datalen > 256)          /* With more than one page of data remaining, you can write one page at a time */
            {
                pageremain = 256;       /* 256 bytes can be written at a time */
            }
            else                        /* The remaining data is less than a page and can be written all at once */
            {
                pageremain = datalen;   /* Not enough for 256 bytes */
            }
        }
    }
}

/**
 * @brief   write SPI FLASH
 * @param   pbuf    : The data store
 * @param   addr    : The address to start writing (Max. 32 bits)
 * @param   datalen : Number of bytes to write (Max 65535)
 * @retval  None
 */
uint8_t g_norflash_buf[4096];   /* Sector cache */

void norflash_write(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;
    uint8_t *norflash_buf;

    norflash_buf = g_norflash_buf;
    secpos = addr / 4096;       /* Sector address */
    secoff = addr % 4096;       /* An offset within a sector */
    secremain = 4096 - secoff;  /* The amount of free space in the sector */

    if (datalen <= secremain)
    {
        secremain = datalen;    /* No larger than 4,096 bytes */
    }

    while (1)
    {
        norflash_read(norflash_buf, secpos * 4096, 4096);   /* Read the contents of the entire sector */

        for (i = 0; i < secremain; i++)   /* check data */
        {
            if (norflash_buf[secoff + i] != 0XFF)
            {
                break;       /* Need to erase, just exit the for loop */
            }
        }

        if (i < secremain)   /* Need to erase */
        {
            norflash_erase_sector(secpos);  /* Erase this sector */

            for (i = 0; i < secremain; i++)   /* Copy */
            {
                norflash_buf[i + secoff] = pbuf[i];
            }

            norflash_write_nocheck(norflash_buf, secpos * 4096, 4096);  /* Write the entire sector */
        }
        else        /* Write erased, write directly to the sector remaining interval. */
        {
            norflash_write_nocheck(pbuf, addr, secremain);  /* Write sectors directly */
        }

        if (datalen == secremain)
        {
            break;
        }
        else
        {
            secpos++;               /* The sector address is incremented by 1 */
            secoff = 0;             /* The offset position is 0 */

            pbuf += secremain;      /* Pointer offset */
            addr += secremain;      /* Write address offset */
            datalen -= secremain;   /* Byte count decrement */

            if (datalen > 4096)
            {
                secremain = 4096;   /* I still can't finish writing the next sector */
            }
            else
            {
                secremain = datalen;/* finish writing the next sector */
            }
        }
    }
}

/**
 * @brief  erases the entire chip
 * @param  None
 * @retval None
 */
void norflash_erase_chip(void)
{
    norflash_write_enable();    /* Write enable */
    norflash_wait_busy();
    NORFLASH_CS(0);
    spi2_read_write_byte(FLASH_ChipErase);  /* Send the read register command */
    NORFLASH_CS(1);
    norflash_wait_busy();       /* Wait for chip erasure to finish */
}

/**
 * @brief       Erases a sector
 * @param       saddr : The sector address is set according to the actual capacity
 * @retval      None
 */
void norflash_erase_sector(uint32_t saddr)
{
    saddr *= 4096;
    norflash_write_enable();        /* Write enable */
    norflash_wait_busy();

    NORFLASH_CS(0);
    spi2_read_write_byte(FLASH_SectorErase);    /* Send the write page command */
    norflash_send_address(saddr);   /* send address */
    NORFLASH_CS(1);
    norflash_wait_busy();           /* Wait for the sector erasure to complete */
}
