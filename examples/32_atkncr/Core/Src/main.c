/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ALIENTEK
 * @brief       atkncr code
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
#include "../../ATK_Middlewares/ATKNCR/atk_ncr.h"
#include "../../ATK_Middlewares/MALLOC/malloc.h"
#include "../../BSP/IIC/myiic.h"
#include "../../BSP/24CXX/24cxx.h"
#include "../../BSP/LED/led.h"
#include "../../BSP/KEY/key.h"
#include "../../BSP/LCD/lcd.h"
#include "../../FatFs/exfuns/exfuns.h"
#include "../../FatFs/exfuns/fattester.h"
#include "ff.h"
#include "../../BSP/DELAY/delay.h"
#include "../../BSP/TOUCH/touch.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* Handwritten track record */
static atk_ncr_point ncr_input_buf[200];

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

/**
 * @brief   Draw a thick line
 * @param   x1: Starting X coordinate
 * @param   y1: Starting Y coordinate
 * @param   x2: Ending X coordinate
 * @param   y2: Ending Y coordinate
 * @param   size: Thickness of the line
 * @param   color: Color of the line
 * @retval  None
 */
static void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color)
{
    uint16_t t;
    int xerr = 0;
    int yerr = 0;
    int delta_x;
    int delta_y;
    int distance;
    int incx;
    int incy;
    int row;
    int col;

    if ((x1 < size) || (x2 < size) || (y1 < size) || (y2 < size))
    {
        return;
    }

    /* Calculate coordinate increments */
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    /* Set the direction for horizontal single steps */
    if (delta_x > 0)
    {

        incx = 1;
    }
    else if (delta_x == 0)
    {
        incx = 0;
    }
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    /* Set the direction for vertical single steps */
    if (delta_y > 0)
    {
        incy = 1;
    }
    else if (delta_y == 0)
    {
        incy = 0;
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    /* Choose the primary increment axis for coordinates */
    if (delta_x > delta_y)
    {
        distance = delta_x;
    }
    else
    {
        distance = delta_y;
    }

    /* Draw a line */
    for (t=0; t<=(distance + 1); t++)
    {
        lcd_fill_circle(row, col, size, color);
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
    uint32_t t = 0;
    uint8_t tcnt;
    uint8_t key;
    uint8_t mode = 4;
    uint16_t lastpos[2];
    uint16_t pcnt = 0;
    char sbuf[10];
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
  tp_dev.init();                      /* 初始化触摸屏 */
  my_mem_init(SRAMIN);                /* Initialize the internal SRAM memory pool */
  my_mem_init(SRAMCCM);               /* Initialize the CCM memory pool */
  exfuns_init();                      /* Request memory for exfuns */
  alientek_ncr_init();                /* Initialize handwriting recognition */
  stm32f407vg_show_mesg();

  f_mount(fs[0], "0:", 1);            /* mount SD card */
  f_mount(fs[1], "1:", 1);            /* mount NOR Flash */

  RESTART:
    lcd_show_string(30, 10, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 30, 200, 16, 16, "ATKNCR TEST", RED);
    lcd_show_string(30, 50, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 70, 200, 16, 16, "WKUP: Adjust KEY0: Mode", RED);
    lcd_show_string(30, 90, 200, 16, 16, "Identify the result:", RED);

    lcd_draw_rectangle(19, 110, lcddev.width - 20, lcddev.height - 5, RED);
    lcd_show_string(96, 130, 200, 16, 16, "Handwriting area", BLUE);
    tcnt = 100;
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
  	  key = key_scan(0);
  	  if (key == WKUP_PRES)
  	  {
  	      /* Touch calibration of capacitive screen */
  	      if ((tp_dev.touchtype & 0x80) == 0)
  	      {
  	         tp_adjust();
  	         goto RESTART;
  	      }
  	  }
  	  else if (key == KEY0_PRES)
  	  {
  	      /* Switch recognition mode */
  	      lcd_fill(20, 115, 219, 315, WHITE);
  	      tcnt = 100;
  	      if (++mode > 4)
  	      {
  	         mode = 1;
  	      }

  	      if (mode == 1)
  	      {
  	    	  lcd_show_string(80, 207, 200, 16, 16, "Recognize digits only", BLUE);
  	      }
  	      else if (mode == 2)
  	      {
  	    	  lcd_show_string(64, 207, 200, 16, 16, "Recognize capital letters only", BLUE);
  	      }
  	      else if (mode == 3)
  	      {
  	    	  lcd_show_string(64, 207, 200, 16, 16, "Lowercase letters only", BLUE);
  	      }
  	      else if (mode == 4)
  	      {
  	    	  lcd_show_string(88, 207, 200, 16, 16, "All recognition", BLUE);
  	      }
  	  }

  	  tp_dev.scan(0);
  	  if (tp_dev.sta & TP_PRES_DOWN)
  	  {
  	      /* There are touches, and the touch trace is displayed and recorded */
  	      tcnt = 0;

  	      if (((tp_dev.x[0] < (lcddev.width - 20 - 2)) && (tp_dev.x[0] >= (20 + 2))) &&
  	           ((tp_dev.y[0] < (lcddev.height - 5 - 2)) && (tp_dev.y[0] >= (115 + 2))))
  	      {
  	          if (lastpos[0] == 0xFFFF)
  	          {
  	              lastpos[0] = tp_dev.x[0];
  	              lastpos[1] = tp_dev.y[0];
  	          }

  	          lcd_draw_bline(lastpos[0], lastpos[1], tp_dev.x[0], tp_dev.y[0], 2, BLUE);
  	          lastpos[0] = tp_dev.x[0];
  	          lastpos[1] = tp_dev.y[0];
  	          if (pcnt < 200)
  	          {
  	              if (pcnt != 0)
  	              {
  	                  if ((ncr_input_buf[pcnt - 1].y != tp_dev.y[0]) &&
  	                      (ncr_input_buf[pcnt - 1].x != tp_dev.x[0]))
  	                  {
  	                      ncr_input_buf[pcnt].x = tp_dev.x[0];
  	                      ncr_input_buf[pcnt].y = tp_dev.y[0];
  	                      pcnt++;
  	                  }
  	              }
  	              else
  	              {
  	                  ncr_input_buf[pcnt].x = tp_dev.x[0];
  	                  ncr_input_buf[pcnt].y = tp_dev.y[0];
  	                  pcnt++;
  	              }
  	          }
  	      }
  	   }
  	   else
  	   {
  	       /* The touch is released for handwriting recognition */
  	       lastpos[0] = 0xFFFF;
  	       tcnt++;
  	       if (tcnt == 40)
  	       {
  	          if (pcnt != 0)
  	          {
  	              printf("Total points:%d\r\n", pcnt);
  	              alientek_ncr(ncr_input_buf, pcnt, 6, mode, sbuf);
  	              printf("identify the result:%s\r\n", sbuf);
  	              pcnt = 0;
  	               lcd_show_string(60 + 72, 90, 200, 16, 16, sbuf, BLUE);
  	           }
  	           lcd_fill(20, 115, lcddev.width - 20 - 1, lcddev.height - 5 - 1, WHITE);
  	       }
  	    }

  	t++;
  	if (t % 20 == 0)
  	{
  	   LED0_TOGGLE(); /* flashing LED0 indicates that the system is running */
  	}

  	HAL_Delay(10);    /* delay 10ms */
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
