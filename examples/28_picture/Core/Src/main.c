/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ALIENTEK
 * @brief       picture code
 * @license     Copyright (C) 2012-2024, ALIENTEK
 ****************************************************************************************************
 * @attention
 *
 * platform     : ALIENTEK M100-STM32F407 board
 * website      : www.alientek.com
 * forum        : www.openedv.com/forum.php
 *
 * change logs  :
 * version      data         notes
 * V1.0         20240409     the first version
 *
 ****************************************************************************************************
 */
 
 /* USER CODE BEGIN Header */

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../ATK_Middlewares/MALLOC/malloc.h"
#include "../../BSP/LED/led.h"
#include "../../BSP/KEY/key.h"
#include "../../BSP/LCD/lcd.h"
#include "../../FatFs/exfuns/exfuns.h"
#include "../../FatFs/exfuns/fattester.h"
#include "ff.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern uint8_t g_sd_error;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/**
 * @brief   Displays information about the experiment
 * @param   None
 * @retval  None
 */
void stm32f407vg_show_mesg(void)
{
	/* The serial port outputs the experimental information */
	printf("\n");
	printf("********************************\r\n");
	printf("STM32F407\r\n");
	printf("FATFS Test\r\n");
	printf("ATOM@ALIENTEK\r\n");
	printf("********************************\r\n");
	printf("\r\n");
}

/**
 * @brief   Gets the number of valid image files in the specified path
 * @param   path : Specifies the path
 * @retval  The number of valid image files
 */
static uint16_t pic_get_tnum(char *path)
{
    uint8_t res;
    uint16_t rval = 0;
    DIR tdir;
    FILINFO *tfileinfo;

    /* Request memory and open the directory */
    tfileinfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));
    res = (uint8_t)f_opendir(&tdir, (const TCHAR *)path);
    if ((res == 0) && tfileinfo)
    {
        while (1)
        {
            res = (uint8_t)f_readdir(&tdir, tfileinfo);
            if ((res != 0) || (tfileinfo->fname[0] == 0))
            {
                break;
            }

            /* Check if it is an image file */
            res = exfuns_file_type(tfileinfo->fname);
            if ((res & 0xF0) == 0x50)
            {
                rval++;
            }
        }
    }

    /* free the memory */
    myfree(SRAMIN, tfileinfo);

    return rval;
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t t = 0;
  uint8_t key;
  uint8_t res;
  DIR picdir;
  uint16_t totpicnum;
  FILINFO *picfileinfo;
  char *pname;
  uint32_t *picoffsettbl;
  uint16_t curindex;
  uint16_t temp;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_FSMC_Init();
  MX_TIM4_Init();
  MX_SDIO_SD_Init();
  MX_SPI2_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();                         /* Initialize LCD */
  norflash_init();
  my_mem_init(SRAMIN);                /* Initialize the internal SRAM memory pool */
  my_mem_init(SRAMCCM);               /* Initialize the CCM memory pool */

  exfuns_init();                      /* Request memory for exfuns */
  stm32f407vg_show_mesg();

  f_mount(fs[0], "0:", 1);            /* mount SD card */
  f_mount(fs[1], "1:", 1);            /* mount NOR Flash */


  lcd_show_string(30, 30, 200, 16, 16, "STM32", RED);
  lcd_show_string(30, 50, 200, 16, 16, "PICTURE TEST", RED);
  lcd_show_string(30, 70, 200, 16, 16, "ATOM@ALIENTEK", RED);
  lcd_show_string(30, 90, 200, 16, 16, "WKUP: PREV", RED);
  lcd_show_string(30, 110, 200, 16, 16, "KEY0: NEXT", RED);

  /* Open the images folder */
  while (f_opendir(&picdir, "0:/PICTURE") != FR_OK)
  {
	lcd_show_string(30, 150, 200, 16, 16, "PICTURE Folder error!", RED);
    HAL_Delay(200);
    lcd_fill(30, 130, 200, 16, WHITE);
    HAL_Delay(200);
  }

  /* Get the number of valid image files */
  totpicnum = pic_get_tnum("0:/PICTURE");
  while (totpicnum == 0)
  {
	lcd_show_string(30, 130, 200, 16, 16, "No image files!", RED);
    HAL_Delay(200);
    lcd_fill(30, 130, 200, 16, WHITE);
    HAL_Delay(200);
  }

  /* request memory */
  picfileinfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));
  pname = (char *)mymalloc(SRAMIN, _MAX_LFN * 2 + 1);
  picoffsettbl = (uint32_t *)mymalloc(SRAMIN, 4 * totpicnum);
  while ((picfileinfo == NULL) || (pname == NULL) || (picoffsettbl == NULL))
  {
	lcd_show_string(30, 130, 200, 16, 16, "memory allocation failure!", RED);
    HAL_Delay(200);
    lcd_fill(30, 130, 200, 16, WHITE);
    HAL_Delay(200);
  }

  /* Open the directory and record the image index */
  res = (uint8_t)f_opendir(&picdir, "0:/PICTURE");
  if (res == 0)
  {
    curindex = 0;
    while (1)
    {
      temp = picdir.dptr;
      res = (uint8_t)f_readdir(&picdir, picfileinfo);
      if ((res != 0) || (picfileinfo->fname[0] == 0))
      {
        break;
      }

      res = exfuns_file_type(picfileinfo->fname);
      if ((res & 0xF0) == 0x50)
      {
        picoffsettbl[curindex] = temp;
        curindex++;
      }
    }
  }

  /* Start displaying images */
  lcd_show_string(30, 150, 200, 16, 16, "Start showing...", RED);
  HAL_Delay(1500);
  piclib_init();
  curindex = 0;
  res = (uint8_t)f_opendir(&picdir, (const TCHAR *)"0:/PICTURE");
  while (res == 0)
  {
    /* Get the next image file information */
    dir_sdi(&picdir, picoffsettbl[curindex]);
    res = (uint8_t)f_readdir(&picdir, picfileinfo);
    if ((res != 0) || (picfileinfo->fname[0] == 0))
    {
       break;
    }

    /* Display images according to the image path */
    strcpy((char *)pname, "0:/PICTURE/");
    strcat((char *)pname, (const char *)picfileinfo->fname);
    lcd_clear(BLACK);
    piclib_ai_load_picfile(pname, 0, 0, lcddev.width, lcddev.height, 1);
    lcd_show_string(2, 2, lcddev.width, 16, 16, (char *)pname, RED);
    while (1)
    {
       key = key_scan(0);
       if (key == KEY0_PRES)
       {
          /* Switch to the previous screen */
          if (curindex != 0)
          {
             curindex--;
          }
          else
          {
             curindex = totpicnum - 1;
          }
          break;
       }
       else if (key == WKUP_PRES)
       {
          /* Switch to the next screen */
          curindex++;
          if (curindex >= totpicnum)
          {
             curindex = 0;
          }
          break;
        }

        if (++t == 20)
        {
           t = 0;
           LED0_TOGGLE();
        }

        HAL_Delay(10);
    }
  }

  /* free the memory */
  myfree(SRAMIN, picfileinfo);
  myfree(SRAMIN, pname);
  myfree(SRAMIN, picoffsettbl);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
