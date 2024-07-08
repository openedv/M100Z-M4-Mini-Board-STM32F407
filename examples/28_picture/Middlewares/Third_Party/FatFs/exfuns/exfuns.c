/**
 ****************************************************************************************************
 * @file        exfuns.c
 * @author      ALIENTEK
 * @brief       exfuns code
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
#include "../../ATK_Middlewares/MALLOC/malloc.h"
#include "usart.h"
#include "exfuns.h"
#include "fattester.h"


#define FILE_MAX_TYPE_NUM       7       /* At most FILE MAX TYPE NUM large classes */
#define FILE_MAX_SUBT_NUM       7       /* At most FILE MAX SUBT NUM small classes */

/* List of file types */
char *const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM] =
{
    {"BIN"},            /* BIN file */
    {"LRC"},            /* LRC file */
    {"NES", "SMS"},     /* NES/SMS file*/
    {"TXT", "C", "H"},  /* Text file */
    {"WAV", "MP3", "OGG", "FLAC", "AAC", "WMA", "MID"},   /* Supported music files */
    {"BMP", "JPG", "JPEG", "GIF"},  /* picture file */
    {"AVI"},            /* video file */
};
    
/******************************************************************************************/
/* Public file area, when using malloc */

/* Logical disk workspace (memory must be requested for fs before any FATFS-related functions can be called) */
FATFS *fs[_VOLUMES];

/******************************************************************************************/


/**
 * @brief   requests memory for exfuns
 * @param   None
 * @retval  0, success; 1. Failure.
 */
uint8_t exfuns_init(void)
{
    uint8_t i;
    uint8_t res = 0;

    for (i = 0; i < _VOLUMES; i++)
    {
        fs[i] = (FATFS *)mymalloc(SRAMIN, sizeof(FATFS));   /* Request memory for disk i workspace */

        if (!fs[i])break;
    }
    
#if USE_FATTESTER == 1  /* If file system testing is enabled */
    res = mf_init();    /* Initialize file system test (request memory) */
#endif
    
    if (i == _VOLUMES && res == 0)
    {
        return 0;      /* The application has one failure, namely failure. */
    }
    else 
    {
        return 1;
    }
}

/**
 * @brief   converts lowercase letters to uppercase letters and leaves them unchanged if they are numbers.
 * @param   c: The letter to convert
 * @retval  The converted letter, in uppercase
 */
uint8_t exfuns_char_upper(uint8_t c)
{
    if (c < 'A')return c;   /* The numbers, stay the same. */

    if (c >= 'a')
    {
        return c - 0x20;    /* It's going to be capitalized. */
    }
    else
    {
        return c;           /* Capital letters, stay the same */
    }
}

/**
 * @brief  The type of report file
 * @param  fname: The filename
 * @retval file type
 * @arg    0XFF, the number of the unrecognized file type.
 * @arg    others, where the top four digits indicate the major class and the bottom four digits indicate the minor class.
 */
uint8_t exfuns_file_type(char *fname)
{
    uint8_t tbuf[5];
    char *attr = 0;   /* name suffix */
    uint8_t i = 0, j;

    while (i < 250)
    {
        i++;

        if (*fname == '\0')break;   /* I'm moving to the end */

        fname++;
    }

    if (i == 250)return 0XFF;   /* Wrong string.. */

    for (i = 0; i < 5; i++)     /* Get the suffix name */
    {
        fname--;

        if (*fname == '.')
        {
            fname++;
            attr = fname;
            break;
        }
    }

    if (attr == 0)return 0XFF;

    strcpy((char *)tbuf, (const char *)attr);       /* copy */

    for (i = 0; i < 4; i++)tbuf[i] = exfuns_char_upper(tbuf[i]);    /* All uppercase */

    for (i = 0; i < FILE_MAX_TYPE_NUM; i++)         /* Broad category comparison */
    {
        for (j = 0; j < FILE_MAX_SUBT_NUM; j++)     /* Subclass comparison */
        {
            if (*FILE_TYPE_TBL[i][j] == 0)break;    /* There are no more comparable members of the group. */

            if (strcmp((const char *)FILE_TYPE_TBL[i][j], (const char *)tbuf) == 0) /* Found it. */
            {
                return (i << 4) | j;
            }
        }
    }

    return 0XFF;    /* didn't find */
}

/**
 * @brief    Gets the remaining disk capacity
 * @param    pdrv: disk number ("0:"~"9:")
 * @param    total: Total capacity (KB)
 * @param    free: Free (KB)
 * @retval   0, normal; Other, error code
 */
uint8_t exfuns_get_free(char *pdrv, uint32_t *total, uint32_t *free)
{
    FATFS *fs1;
    uint8_t res;
    uint32_t fre_clust = 0, fre_sect = 0, tot_sect = 0;
    
    /* Get the disk information and the number of free clusters */
    res = (uint32_t)f_getfree((const TCHAR *)pdrv, (DWORD *)&fre_clust, &fs1);

    if (res == 0)
    {
        tot_sect = (fs1->n_fatent - 2) * fs1->csize;    /* The total number of sectors is obtained */
        fre_sect = fre_clust * fs1->csize;              /* The number of free sectors is obtained */
#if _MAX_SS!=512  /* A sector size other than 512 bytes is converted to 512 bytes */
        tot_sect *= fs1->ssize / 512;
        fre_sect *= fs1->ssize / 512;
#endif
        *total = tot_sect >> 1;     /* The unit is KB. */
        *free = fre_sect >> 1;      /* The unit is KB. */
    }

    return res;
}

/**
 * @brief  file copy
 * @note   copy psrc file to pdst.
 * Note:   The file size should not exceed 4GB.
 *
 * @param  fcpymsg: Function pointer to display information when copying
 *           pname:  file/folder name
 *             pct: Percentage
 *            mode:
 *            bit0: Update filename
 *            bit1: Update percentage pct
 *            bit2: Update folders
 *          Others: Reserved
 *    Return value: 0, normal; 1. Force exit.
 *
 * @param   psrc: source file
 * @param   pdst: The object file
 * @param   totsize: Total size (when totsize is 0, it means only a single file copy)
 * @param   cpdsize: The copied size.
 * @param   fwmode: File writing mode
 * @arg        0: Does not overwrite the original file
 * @arg        1: Overwrites the original file
 *
 * @retval result
 * @arg        0, normal
 * @arg        0XFF, force exit
 * @arg        Other, error code
 */
uint8_t exfuns_file_copy(uint8_t(*fcpymsg)(uint8_t *pname, uint8_t pct, uint8_t mode), uint8_t *psrc, uint8_t *pdst, 
                                      uint32_t totsize, uint32_t cpdsize, uint8_t fwmode)
{
    uint8_t res;
    uint16_t br = 0;
    uint16_t bw = 0;
    FIL *fsrc = 0;
    FIL *fdst = 0;
    uint8_t *fbuf = 0;
    uint8_t curpct = 0;
    unsigned long long lcpdsize = cpdsize;
    
    fsrc = (FIL *)mymalloc(SRAMIN, sizeof(FIL));    /* request memory */
    fdst = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    fbuf = (uint8_t *)mymalloc(SRAMIN, 8192);

    if (fsrc == NULL || fdst == NULL || fbuf == NULL)
    {
        res = 100;  /* The previous values are left to fatfs */
    }
    else
    {
        if (fwmode == 0)
        {
            fwmode = FA_CREATE_NEW;     /* Not covered */
        }
        else 
        {
            fwmode = FA_CREATE_ALWAYS;  /* Overwrite an existing file */
        }
        
        res = f_open(fsrc, (const TCHAR *)psrc, FA_READ | FA_OPEN_EXISTING);        /* Open a read-only file */

        if (res == 0)res = f_open(fdst, (const TCHAR *)pdst, FA_WRITE | fwmode);    /* The first one is opened successfully, and then the second one is opened */

        if (res == 0)           /* Both opened successfully */
        {
            if (totsize == 0)   /* Just a single file copy */
            {
            	totsize = fsrc->obj.objsize;
                lcpdsize = 0;
                curpct = 0;
            }
            else
            {
                curpct = (lcpdsize * 100) / totsize;            /* Get the new percentage */
            }
            
            fcpymsg(psrc, curpct, 0X02);                        /* Update percentage */

            while (res == 0)    /* Start copying */
            {
                res = f_read(fsrc, fbuf, 8192, (UINT *)&br);    /* The source reads 512 bytes */

                if (res || br == 0)break;

                res = f_write(fdst, fbuf, (UINT)br, (UINT *)&bw);/* Writing to the destination file */
                lcpdsize += bw;

                if (curpct != (lcpdsize * 100) / totsize)       /* Whether percentages need to be updated */
                {
                    curpct = (lcpdsize * 100) / totsize;

                    if (fcpymsg(psrc, curpct, 0X02))            /* Update percentage */
                    {
                        res = 0XFF;                             /* force to exit */
                        break;
                    }
                }

                if (res || bw < br)break;
            }

            f_close(fsrc);
            f_close(fdst);
        }
    }

    myfree(SRAMIN, fsrc); /* free the memory */
    myfree(SRAMIN, fdst);
    myfree(SRAMIN, fbuf);
    return res;
}

/**
 * @brief   gets the folder under the path
 * @note    removes all paths, leaving only the folder name.
 * @param   pname: The detailed path
 * @retval  0, the path is just a volume label.
 *          Other, folder name first address
 */
uint8_t *exfuns_get_src_dname(uint8_t *pname)
{
    uint16_t temp = 0;

    while (*pname != 0)
    {
        pname++;
        temp++;
    }

    if (temp < 4)return 0;

    while ((*pname != 0x5c) && (*pname != 0x2f))pname--;    /* Go to the penultimate "\" or "/" */

    return ++pname;
}

/**
 * @brief  gets the folder size
 * @note   The folder size should not exceed 4GB.
 * @param  pname: The detailed path
 * @retval 0, the folder size is 0, or an error occurred while reading.
 *         Other, folder size
 */
uint32_t exfuns_get_folder_size(uint8_t *fdname)
{
#define MAX_PATHNAME_DEPTH  512 + 1     /* Maximum target file path + filename depth */
    uint8_t res = 0;
    DIR *fddir = 0;         /* catalogue */
    FILINFO *finfo = 0;     /* file information */
    uint8_t *pathname = 0;  /* Destination folder path + filename */
    uint16_t pathlen = 0;   /* Target path length */
    uint32_t fdsize = 0;

    fddir = (DIR *)mymalloc(SRAMIN, sizeof(DIR));   /* request memory */
    finfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));

    if (fddir == NULL || finfo == NULL)res = 100;

    if (res == 0)
    {
        pathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);

        if (pathname == NULL)res = 101;

        if (res == 0)
        {
            pathname[0] = 0;
            strcat((char *)pathname, (const char *)fdname);     /* Copy path */
            res = f_opendir(fddir, (const TCHAR *)fdname);      /* Open the source directory */

            if (res == 0)   /* Opened directory successfully */
            {
                while (res == 0)   /* Start copying the contents of the folder */
                {
                    res = f_readdir(fddir, finfo);                  /* Read a file in a directory */

                    if (res != FR_OK || finfo->fname[0] == 0)break; /* Error/at the end, exit */

                    if (finfo->fname[0] == '.')continue;            /* Ignore the parent directory */

                    if (finfo->fattrib & 0X10)   /* Is a subdirectory (file properties,0X20, archive; 0X10, subdirectory;) */
                    {
                        pathlen = strlen((const char *)pathname);   /* Get the length of the current path */
                        strcat((char *)pathname, (const char *)"/");/* Put forward slashes */
                        strcat((char *)pathname, (const char *)finfo->fname);   /* Source path followed by subdirectory name */
                        //printf("\r\nsub folder:%s\r\n",pathname);      /* Type the subdirectory name */
                        fdsize += exfuns_get_folder_size(pathname);      /* Get the subdirectory size and call recursively */
                        pathname[pathlen] = 0;                           /* Adding a terminator */
                    }
                    else
                    {
                        fdsize += finfo->fsize; /* Not a directory, simply add the size of the file */
                    }
                }
            }

            myfree(SRAMIN, pathname);
        }
    }

    myfree(SRAMIN, fddir);
    myfree(SRAMIN, finfo);

    if (res)
    {
        return 0;
    }
    else 
    {
        return fdsize;
    }
}

/**
 * @brief  folder copy
 * @note   Copy psrc folder to pdst folder.
 * Note:   The file size should not exceed 4GB.
 *
 * @param  fcpymsg: Function pointer to display information when copying
 *           pname: file/folder name
 *             pct: Percentage
 *            mode:
 *            bit0: Update filename
 *            bit1: Update percentage pct
 *            bit2: Update folders
 *          Others: Reserved
 *    Return value: 0, normal; 1. Force exit.
 *
 * @param  psrc    : the source folder
 * @param  pdst    : The destination folder
 * @param  totsize : Total size (when totsize is 0, it means only a single file copy)
 * @param  cpdsize : The copied size.
 * @param  fwmode  : File writing mode
 * @arg    0: Does not overwrite the original file
 * @arg    1: Overwrites the original file
 *
 * @retval result
 * @arg    0, normal
 * @arg    0XFF, force exit
 * @arg    Other, error code
 */
uint8_t exfuns_folder_copy(uint8_t(*fcpymsg)(uint8_t *pname, uint8_t pct, uint8_t mode), uint8_t *psrc, uint8_t *pdst, 
                           uint32_t *totsize, uint32_t *cpdsize, uint8_t fwmode)
{
#define MAX_PATHNAME_DEPTH 512 + 1  /* Maximum target file path + filename depth */
    uint8_t res = 0;
    DIR *srcdir = 0;    /* source directory */
    DIR *dstdir = 0;    /* source directory */
    FILINFO *finfo = 0; /* file information */
    uint8_t *fn = 0;    /* long filename */

    uint8_t *dstpathname = 0;   /* Destination folder path + filename */
    uint8_t *srcpathname = 0;   /* Source folder path + filename */

    uint16_t dstpathlen = 0;    /* Target path length */
    uint16_t srcpathlen = 0;    /* Source path length */


    srcdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR));  /* request memory */
    dstdir = (DIR *)mymalloc(SRAMIN, sizeof(DIR));
    finfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));

    if (srcdir == NULL || dstdir == NULL || finfo == NULL)res = 100;

    if (res == 0)
    {
        dstpathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);
        srcpathname = mymalloc(SRAMIN, MAX_PATHNAME_DEPTH);

        if (dstpathname == NULL || srcpathname == NULL)res = 101;

        if (res == 0)
        {
            dstpathname[0] = 0;
            srcpathname[0] = 0;
            strcat((char *)srcpathname, (const char *)psrc);    /* Copy the original source file path */
            strcat((char *)dstpathname, (const char *)pdst);    /* Copy the original object file path */
            res = f_opendir(srcdir, (const TCHAR *)psrc);       /* Open the source directory */

            if (res == 0)   /* Opened directory successfully */
            {
                strcat((char *)dstpathname, (const char *)"/"); /* Add forward slashes */
                fn = exfuns_get_src_dname(psrc);

                if (fn == 0)   /* Label copy */
                {
                    dstpathlen = strlen((const char *)dstpathname);
                    dstpathname[dstpathlen] = psrc[0];  /* Record label */
                    dstpathname[dstpathlen + 1] = 0;    /* end label */
                }
                else strcat((char *)dstpathname, (const char *)fn); /* Add the filename */

                fcpymsg(fn, 0, 0X04);   /* Update folder names */
                res = f_mkdir((const TCHAR *)dstpathname);  /* If the folder already exists, it won't be created. If it doesn't exist, create a new folder */

                if (res == FR_EXIST)res = 0;

                while (res == 0)   /* Start copying the contents of the folder */
                {
                    res = f_readdir(srcdir, finfo);         /* Read a file in a directory */

                    if (res != FR_OK || finfo->fname[0] == 0)break; /* Error/at the end, exit */

                    if (finfo->fname[0] == '.')continue;    /* Ignore the parent directory */

                    fn = (uint8_t *)finfo->fname;           /* Get the filename */
                    dstpathlen = strlen((const char *)dstpathname); /* Get the length of the current target path */
                    srcpathlen = strlen((const char *)srcpathname); /* The source path length is obtained */

                    strcat((char *)srcpathname, (const char *)"/"); /* Source path with slash */

                    if (finfo->fattrib & 0X10)  /* Is a subdirectory (file properties,0X20, archive; 0X10, subdirectory;) */
                    {
                        strcat((char *)srcpathname, (const char *)fn);  /* Source path followed by subdirectory name */
                        res = exfuns_folder_copy(fcpymsg, srcpathname, dstpathname, totsize, cpdsize, fwmode);   /* Copying folders */
                    }
                    else     /* non-directory */
                    {
                        strcat((char *)dstpathname, (const char *)"/"); /* The target path is followed by a slash */
                        strcat((char *)dstpathname, (const char *)fn);  /* Destination path plus filename */
                        strcat((char *)srcpathname, (const char *)fn);  /* Source path plus filename */
                        fcpymsg(fn, 0, 0X01);       /* Update filenames */
                        res = exfuns_file_copy(fcpymsg, srcpathname, dstpathname, *totsize, *cpdsize, fwmode);  /* Copying folders */
                        *cpdsize += finfo->fsize;   /* Increase the file size by one */
                    }

                    srcpathname[srcpathlen] = 0;    /* Adding a terminator */
                    dstpathname[dstpathlen] = 0;    /* Adding a terminator */
                }
            }

            myfree(SRAMIN, dstpathname);
            myfree(SRAMIN, srcpathname);
        }
    }

    myfree(SRAMIN, srcdir);
    myfree(SRAMIN, dstdir);
    myfree(SRAMIN, finfo);
    return res;
}



















