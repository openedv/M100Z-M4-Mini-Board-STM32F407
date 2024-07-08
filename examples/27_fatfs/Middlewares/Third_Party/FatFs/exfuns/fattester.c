/**
 ****************************************************************************************************
 * @file        fattester.c
 * @author      ALIENTEK
 * @brief       fattester code
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

#include "string.h"
#include "malloc.h"
#include "usart.h"
#include "exfuns.h"
#include "fattester.h"
#include "sdio.h"

/* If filesystem testing is supported, the following code is enabled */
#if USE_FATTESTER == 1

_m_fattester fattester;


/**
 * @brief   Initialize filesystem test (request memory)
 * @param   None
 * @retval  result: 0, success; 1. fail;
 */
uint8_t mf_init(void)
{
    fattester.file = (FIL *)mymalloc(0, sizeof(FIL));      /* Request memory for file */
    fattester.fatbuf = (uint8_t *)mymalloc(0, 512);        /* Request memory for fattester.fatbuf */

    if (fattester.file && fattester.fatbuf)
    {
        return 0;   /* application approved */
    }
    else
    {
        mf_free();  /* free the memory */
        return 1;   /* fail to apply */
    }
}

/**
 * @brief   frees the memory requested by the filesystem test
 * @param   None
 * @retval  None
 */
void mf_free(void)
{
    myfree(0, fattester.file);
    myfree(0, fattester.fatbuf);
}

/**
 * @brief    registers the workspace for the disk
 * @param    path : Disk path such as "0:", "1:"
 * @param    mt   : 0, do not register immediately (register later); 1. Register now
 * @retval   result (see FATFS, FRESULT definition)
 */
uint8_t mf_mount(uint8_t *path, uint8_t mt)
{
    return f_mount(fs[1], (const TCHAR *)path, mt);
}

/**
 * @brief   Opens the file
 * @param   path : Path + filename
 * @param   mode : The mode is turned on
 * @retval  result (see FATFS, FRESULT definition)
 */
uint8_t mf_open(uint8_t *path, uint8_t mode)
{
    uint8_t res;
    res = f_open(fattester.file, (const TCHAR *)path, mode);    /* Open file */
    return res;
}

/**
 * @brief  Closes the file
 * @param  None
 * @retval result (see FATFS, FRESULT definition)
 */
uint8_t mf_close(void)
{
    f_close(fattester.file);
    return 0;
}

/**
 * @brief  reads out the data
 * @param  len : The length to read out
 * @retval result (see FATFS, FRESULT definition)
 */
uint8_t mf_read(uint16_t len)
{
    uint16_t i, t;
    uint8_t res = 0;
    uint16_t tlen = 0;
    uint32_t br = 0;
    printf("\r\nRead fattester.file data is:\r\n");

    for (i = 0; i < len / 512; i++)
    {
        res = f_read(fattester.file, fattester.fatbuf, 512, &br);

        if (res)
        {
            printf("Read Error:%d\r\n", res);
            break;
        }
        else
        {
            tlen += br;

            for (t = 0; t < br; t++)printf("%c", fattester.fatbuf[t]);
        }
    }

    if (len % 512)
    {
        res = f_read(fattester.file, fattester.fatbuf, len % 512, &br);

        if (res)    /* There was an error reading the data */
        {
            printf("\r\nRead Error:%d\r\n", res);
        }
        else
        {
            tlen += br;

            for (t = 0; t < br; t++)printf("%c", fattester.fatbuf[t]);
        }
    }

    if (tlen)printf("\r\nReaded data len:%d\r\n", tlen);    /* The size of the data read */

    printf("Read data over\r\n");
    return res;
}

/**
 * @brief    writes the data
 * @param    pdata : The data cache
 * @param    len   : Write length
 * @retval   result (see FATFS, FRESULT definition)
 */
uint8_t mf_write(uint8_t *pdata, uint16_t len)
{
    uint8_t res;
    uint32_t bw = 0;

    printf("\r\nBegin Write fattester.file...\r\n");
    printf("Write data len:%d\r\n", len);
    res = f_write(fattester.file, pdata, len, &bw);

    if (res)
    {
        printf("Write Error:%d\r\n", res);
    }
    else
    {
        printf("Writed data len:%d\r\n", bw);
    }

    printf("Write data over.\r\n");
    return res;
}

/**
 * @brief   Opens the directory
 * @param   path : The path
 * @retval  result (see FATFS, FRESULT definition)
 */
uint8_t mf_opendir(uint8_t *path)
{
    return f_opendir(&fattester.dir, (const TCHAR *)path);
}

/**
 * @brief  Closes the directory
 * @param  None
 * @retval result (see FATFS, FRESULT definition)
 */
uint8_t mf_closedir(void)
{
    return f_closedir(&fattester.dir);
}

/**
 * @brief  reads folders
 * @param  None
 * @retval result (see FATFS, FRESULT definition)
 */
uint8_t mf_readdir(void)
{
    uint8_t res;
    res = f_readdir(&fattester.dir, &fattester.fileinfo);   /* Read information about a file */

    if (res != FR_OK)return res;    /* something went wrong */

    printf("\r\n fattester.dir info:\r\n");

    printf("fattester.dir.id:%d\r\n", fattester.dir.obj.id);

    printf("\r\n");
    printf("fattester.file Name is:%s\r\n", fattester.fileinfo.fname);
    printf("fattester.file Size is:%lu\r\n", fattester.fileinfo.fsize);
    printf("fattester.file data is:%d\r\n", fattester.fileinfo.fdate);
    printf("fattester.file time is:%d\r\n", fattester.fileinfo.ftime);
    printf("fattester.file Attr is:%d\r\n", fattester.fileinfo.fattrib);
    printf("\r\n");
    return 0;
}

/**
 * @brief  Iterates over the file
 * @param  path : The path
 * @retval result (see FATFS, FRESULT definition)
 */
uint8_t mf_scan_files(uint8_t *path)
{
    FRESULT res;
    res = f_opendir(&fattester.dir, (const TCHAR *)path); /* Opening a directory */

    if (res == FR_OK)
    {
        printf("\r\n");

        while (1)
        {
            res = f_readdir(&fattester.dir, &fattester.fileinfo);   /* Read a file in a directory */

            if (res != FR_OK || fattester.fileinfo.fname[0] == 0)
            {
                break;  /* Error/at the end, exit */
            }
            
            // if (fattester.fileinfo.fname[0] == '.') continue;    /* Ignore the parent directory */
            printf("%s/", path);    /* Print path */
            printf("%s\r\n", fattester.fileinfo.fname); /* Print the filename */
        }
    }

    return res;
}

/**
 * @brief   shows the remaining capacity
 * @param   path : path (disk character)
 * @retval  Remaining capacity (bytes)
 */
uint32_t mf_showfree(uint8_t *path)
{
    FATFS *fs1;
    uint8_t res;
    uint32_t fre_clust = 0, fre_sect = 0, tot_sect = 0;
    /* Get the disk information and the number of free clusters */
    res = f_getfree((const TCHAR *)path, (DWORD *)&fre_clust, &fs1);

    if (res == 0)
    {
        tot_sect = (fs1->n_fatent - 2) * fs1->csize;/* The total number of sectors is obtained */
        fre_sect = fre_clust * fs1->csize;          /* The number of free sectors is obtained */
#if _MAX_SS!=512
        tot_sect *= fs1->ssize / 512;
        fre_sect *= fs1->ssize / 512;
#endif

        if (tot_sect < 20480)   /* The total capacity is less than 10M */
        {
            /* Print free space in unit of KB (assuming 512 bytes/sector) */
            printf("\r\nTotal disk capacity:%d KB\r\n"
                   "Free space :%d KB\r\n",
                   tot_sect >> 1, fre_sect >> 1);
        }
        else
        {
            /* Print free space in unit of KB (assuming 512 bytes/sector) */
            printf("\r\nTotal disk capacity:%d MB\r\n"
                   "Free space :%d MB\r\n",
                   tot_sect >> 11, fre_sect >> 11);
        }
    }

    return fre_sect;
}

/**
 * @brief   file read and write pointer offset
 * @param   offset : The offset relative to the first address
 * @retval  result (see FATFS, FRESULT definition)
 */
uint8_t mf_lseek(uint32_t offset)
{
    return f_lseek(fattester.file, offset);
}

/**
 * @brief  reads the current read/write pointer position of the file
 * @param  None
 * @retval the current location
 */
uint32_t mf_tell(void)
{
    return f_tell(fattester.file);
}

/**
 * @brief   reads the file size
 * @param   None
 * @retval  file size
 */
uint32_t mf_size(void)
{
    return f_size(fattester.file);
}

/**
 * @brief  creates directories
 * @param  path : Directory path + name
 * @retval result (see FATFS, FRESULT definition)
 */
uint8_t mf_mkdir(uint8_t *path)
{
    return f_mkdir((const TCHAR *)path);
}

/**
 * @brief   formatting
 * @param   path : Disk path such as "0:", "1:"
 * @param   opt  : mode,FM_FAT,FM_FAT32,FM_EXFAT,FM_ANY, etc...
 * @param   au   : Cluster size
 * @retval  result (see FATFS, FRESULT definition)
 */
uint8_t mf_fmkfs(uint8_t *path, uint8_t opt, uint16_t au)
{
	return f_mkfs((const TCHAR *)path,  opt, 0, 0, _MAX_SS);    /* Formatting, default parameters, workbuf, minimum _MAX_SS size */
}

/**
 * @brief   Deletes files/directories
 * @param   path : file/directory path + name
 * @retval  result (see FATFS, FRESULT definition)
 */
uint8_t mf_unlink(uint8_t *path)
{
    return  f_unlink((const TCHAR *)path);
}

/**
 * @brief   Change the file/directory name (move the file if the directory is different!)
 * @param   oldname : The previous name
 * @param   newname : New name
 * @retval  result (see FATFS, FRESULT definition)
 */
uint8_t mf_rename(uint8_t *oldname, uint8_t *newname)
{
    return  f_rename((const TCHAR *)oldname, (const TCHAR *)newname);
}

/**
 * @brief   Get the disk character (disk name)
 * @param   path : Disk path such as "0:", "1:"
 * @retval  result (see FATFS, FRESULT definition)
 */
void mf_getlabel(uint8_t *path)
{
    uint8_t buf[20];
    uint32_t sn = 0;
    uint8_t res;
    res = f_getlabel ((const TCHAR *)path, (TCHAR *)buf, (DWORD *)&sn);

    if (res == FR_OK)
    {
        printf("\r\nThe symbol for disk %s is:%s\r\n", path, buf);
        printf("Serial number of disk %s:%X\r\n\r\n", path, sn);
    }
    else
    {
        printf("\r\nFailed to retrieve, error code:%X\r\n", res);
    }
}

/**
 * @brief   Sets the drive character (disk name), Max 11 characters!! , support number and capital letter combination and Chinese characters
 * @param   path: disk number + name, e.g. "0:ALIENTEK", "1:OPENEDV"
 * @retval  result (see FATFS, FRESULT definition)
 */
void mf_setlabel(uint8_t *path)
{
    uint8_t res;
    res = f_setlabel ((const TCHAR *)path);

    if (res == FR_OK)
    {
        printf("\r\nThe disk letter is set successfully:%s\r\n", path);
    }
    else printf("\r\nFailed to set the disk letter, error code:%X\r\n", res);
}

/**
 * @brief   reads a string from a file
 * @param   size : The size to read
 * @retval  result (see FATFS, FRESULT definition)
 */
void mf_gets(uint16_t size)
{
    TCHAR *rbuf;
    rbuf = f_gets((TCHAR *)fattester.fatbuf, size, fattester.file);

    if (*rbuf == 0)return  ; /* No data was read */
    else
    {
        printf("\r\nThe String Readed Is:%s\r\n", rbuf);
    }
}

/**
 * @brief   Writes a character to the file (requires FF_USE_STRFUNC >= 1)
 * @param   c: The character to write
 * @retval  result (see FATFS, FRESULT definition)
 */
uint8_t mf_putc(uint8_t c)
{
    return f_putc((TCHAR)c, fattester.file);
}

/**
 * @brief  Write string to file (requires FF_USE_STRFUNC >= 1)
 * @param  str: The string to write
 * @retval result (see FATFS, FRESULT definition)
 */
uint8_t mf_puts(uint8_t *str)
{
    return f_puts((TCHAR *)str, fattester.file);
}

#endif












