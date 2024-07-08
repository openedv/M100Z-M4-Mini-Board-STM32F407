## Fatfs example<a name="brief"></a>


### 1 Brief
The function of this example is to use FATFS file system to manage the data in SD card and norflash.
### 2 Hardware Hookup
The hardware resources used in this example are:
+ LED0 - PE5
+ USART1 - PA9/PA10
+ KEY - KEY0(PA15)
+ Micro SD card
+ NORFLASH
+ ALIENTEK  2.8/3.5/4.3/7 inch TFTLCD module

The FATFS used in this example is a software library, so there is no corresponding connection graph.

### 3 STM32CubeIDE Configuration

We copy the project from **26_sd** and name both the project and the.ioc file **27_fatfs**. Next we start the configuration by double-clicking the **27_fatfs.ioc** file. For this example, we also need to configure the NORFLASH driver, which is covered in the SPI section.

Click **Middlewares and Software Packs -> FATFS**, the specific configuration is shown below:

<img src="../../1_docs/3_figures/27_fatfs/01_config.png">

The specific FATFS parameter configuration is as follows:

<img src="../../1_docs/3_figures/27_fatfs/02_config.png">

To avoid a stack overflow, we need to set the stack size larger.

<img src="../../1_docs/3_figures/27_fatfs/03_heap.png">

Click **File > Save**, and you will be asked to generate code.Click **Yes**.The resulting project file is shown below:

<img src="../../1_docs/3_figures/27_fatfs/04_file.png">


##### code

###### diskio.c

We're going to change **diskio.c** to something that works for us, but note that each time we regenerate the code here, diskio.c will be replaced with the original code state, so we need to remember to add the changes back.

You can open the project to see the specific code, the file we mainly changed a few functions, respectively ``disk_initialize``,  ``disk_read``, ``disk_write``, ``disk_ioctl``.

1) **disk_initialize** : initialize the specified number of disk, disk specified storage area. 
2) **disk_read** : read sector data from the disk drive. The function takes four parameters:
3) **disk_wirte** : writes sector data to the disk drive.
4) **disk_ioctl** : control device specified features and miscellaneous functions in addition to read/write

###### exfuns.c
There are a few more files to add: exfuns.c, exfuns.h, fattester.c, fattester.h.You can open the project to see the specific code.

<img src="../../1_docs/3_figures/27_fatfs/05_exfuns.png">

###### main.c
```c#
int main(void)
{
  /* USER CODE BEGIN 1 */
  FRESULT res;
  uint32_t total;
  uint32_t free;
  uint8_t t;
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
  my_mem_init(SRAMIN);                /* Initialize the internal SRAM memory pool */
  my_mem_init(SRAMCCM);               /* Initialize the CCM memory pool */
  stm32f407vg_show_mesg();
  lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
  lcd_show_string(30, 70, 200, 16, 16, "FATFS TEST", RED);
  lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
  lcd_show_string(30, 110, 200, 16, 16, "Use USMART for test", RED);

  while (g_sd_error == 1)
  {
  	 g_sd_error = 0;
  	 MX_SDIO_SD_Init();
     lcd_show_string(30, 130, 200, 16, 16, "SD Card Error!", RED);
     HAL_Delay(500);
     lcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
     HAL_Delay(500);
     LED1_TOGGLE();
  }

  exfuns_init();

  /* mount SD card */
  f_mount(fs[0], "0:", 1);

  /* mount NOR Flash, if NOR Flash does not have a file system, it needs to be formatted */
  res = f_mount(fs[1], "1:", 1);
  if (res == FR_NO_FILESYSTEM)
  {
    lcd_show_string(30, 130, 200, 16, 16, "Flash Disk Formatting...", RED);
    res = f_mkfs("1:", 0, 0, 0, _MAX_SS);
    if (res == FR_OK)
    {
      f_setlabel((const TCHAR *)"1:ALIENTEK");
      lcd_show_string(30, 130, 200, 16, 16, "Flash Disk Format Finish", RED);
    }
    else
    {
      lcd_show_string(30, 130, 200, 16, 16, "Flash Disk Format Error ", RED);
    }
    HAL_Delay(1000);
  }
  lcd_fill(30, 130, 239, 145, WHITE);

  /* Gets the SD card capacity */
  while (exfuns_get_free("0", &total, &free) != 0)
  {
    lcd_show_string(30, 130, 200, 16, 16, "SD Card FatFs Error!", RED);
    HAL_Delay(200);
    lcd_fill(30, 130, 240, 150 + 16, WHITE);
    HAL_Delay(200);
    LED1_TOGGLE();
  }

  lcd_show_string(30, 130, 200, 16, 16, "FATFS OK!", BLUE);
  lcd_show_string(30, 150, 200, 16, 16, "SD Total Size:     MB", BLUE);
  lcd_show_string(30, 170, 200, 16, 16, "SD Free Size:     MB", BLUE);
  lcd_show_num(142, 150, total >> 10, 5, 16, BLUE);
  lcd_show_num(134, 170, free >> 10, 5, 16, BLUE);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
```


### 4 Running
#### 4.1 Compile & Download
After the compilation is complete, connect the DAP and the Mini Board, and then connect to the computer together to download the program to the Mini Board.
#### 4.2 Phenomenon
Press the **RESET** button to begin running the program on your Mini Board, observe the LED0 flashing on the Mini Board, open the serial port and the host computer ATK-XCOM can see the prompt information of the example, indicating that the code download is successful. 

We take a microSD card to test, you can see the screen will show the following picture:

<img src="../../1_docs/3_figures/27_fatfs/06_lcd.png">

By opening the serial debug assistant, we can call the FATFS test functions we added earlier.

<img src="../../1_docs/3_figures/27_fatfs/07_lcd.png">

Other functions can be tested in a similar way. Note that 0 is for SD card and 1 is for SPI FLASH. In addition, mf_unlink function, when deleting the folder, must ensure that the folder is empty, can be deleted normally, otherwise can not be deleted.

[jump to title](#brief)
