## 1 Repository overview

This repository is structured as follows:

+ A [1_docs](./1_docs) folder, containing related documentation, includes schematic diagrams, data sheets, pictures of each example configuration, pin elicitation diagrams, and pin assignment tables.
+ A [2_tools](./2_tools) folder, containing Development tools. Such as Flash download software, USB virtual serial port driver, and serial port terminal, etc
+ An [example](./examples) folder, containing examples of different peripherals.
+ A [libraries](./libraries) folder, containing HAL library for STM32F4.


## 2 Board Resources

Here aims to introduce the **hardware and software resources** of the M100Z-M4 Mini Board, including the introduction of chip resources, the introduction of experimental platform hardware resources, and the supporting resource use guide. By studying this manual, the reader will have a comprehensive understanding of the resources of the Mini Board, which will be very helpful for subsequent program design.

It is intended to provide readers with a comprehensive introduction to the M100Z-M4 Mini Board resources. It is highly recommended that you read this manual before learning the examples!

### 2.1 Hardware Description

<img src="./1_docs/3_figures/image/01_M100.png">

M100Z-M4 Mini Board selects **STM32F407VGT6** as MCU, the chip integrates 192KB SRAM, 1MB FLASH, 12 16-bit timers, 2 32-bit timers, 6 UARTs, 1 USB, 3 units, 16-channels ADC, 2 12-bit DACs, 1 RTC, 1 SDIO, 1 FSMC, and 82 general-purpose I/O ports.

+ **EEPROM**:The EEPROM chip, with a capacity of 2Kb, which is equivalent to 256 bytes. It is used to store important data that cannot be lost during power loss, such as system settings parameters/touch screen calibration data, etc. With this, it is convenient to achieve non-volatile data storage.
+ **NOR FLASH**:The SPI FLASH chip expanded externally on the mini board, with a capacity of 128Mbit, which is equivalent to 16M bytes. It can be used to store user data, meeting the demand for large-capacity data storage. Of course, if you find that 16M bytes are not sufficient, you can store the data on an external TF card.
+ **USB slave**: this interface can not only be used as a USB slave communication interface, but also has a power supply function, which can supply power for the Mini Board. USB_Slave can be used to connect the computer to realize the USB virtual serial port experiment.
+ **Reset button**: used to reset the Mini Board.
+ **TF card interface**:The TF card interface driven via SDIO. The TF card capacity selection range is very wide (up to TB level). With this interface, it can meet the demand for massive data storage.
+ **TFTLCD interface**:This is the onboard LCD module interface on the mini board, which is connected to the TFTLCD module via FPC ribbon cable and an adapter board. This interface is compatible with the entire series of TFTLCD modules from Elecrow, including 2.4-inch, 3.5-inch, 4.3-inch, and 7-inch TFTLCD modules, and supports both resistive and capacitive touch functionalities.

+ **Serial port 1 debug interface**: used for simulation debugging, downloading code, etc.



### 2.2 Software Resources

There are up to **49** examples provided. Before learning each example, carefully read the **README.md**, which includes a brief introduction to the example function, hardware hookup, STM32CubeIDE configuration instructions, and running phenomena. Here are all the examples.

| Example Name                      | Example Name                      | Example Name                      |  
| --------------------------------- |  ----------------------           |  ----------------------           |
| 00_f407                           | 11_tftlcd                         | 21_spi                            |
| 01_led                            | 12_usmart                         | 22_touch                          | 
| 02_key                            | 13_rtc                            | 23_flash_eeprom                   |
| 03_exti                           | 14_rng                            | 24_camera                         |  
| 04_uart                           | 15_1_lowpower_pvd                 | 25_malloc                         |
| 05_iwdg                           | 15_2_lowpower_sleep               | 26_sd                             |
| 06_wwdg                           | 15_3_lowpower_stop                | 27_fatfs                          | 
| 07_btim                           | 15_4_lowpower_standby             | 28_pictures                       | 
| 08_1_gtim_interrupt               | 16_dma                            | 29_photo                          |
| 08_2_gtim_pwm                     | 17_1_adc_single_channel           | 30_1_dsp_basicmath                |
| 08_3_gtim_capture                 | 17_2_adc_dma_single_channel       | 30_2_dsp_fft                      |
| 08_4_gtim_count                   | 17_3_adc_dma_multi_channel        | 31_1_enable_Julia                 |
| 09_1_atim_npwm                    | 18_adc_temperature                | 31_2_disable_Julia                |
| 09_2_atim_compare                 | 19_1_dac_output                   | 32_atkncr                         |
| 09_3_atim_cplm_pwm                | 19_2_dac_triangular_wave          | 33_1_iap_bootloader               | 
| 09_4_atim_pwm_in                  | 19_3_dac_sine_wave                | 34_1_usb_card_reader              |
| 10_oled                           | 20_iic                            | 35_freertos_demo                  |

## 3 How to use STM32CubeIDE
If you have not set up **STM32CubeIDE** or can't use it, you will need to do so following the [steps outlined in this tutorial](./1_docs/STM32CubeIDE_Usage_Guide.md).

## 4 Accessories
When using the MIni Board, you can use the following accessories.
 
| LCD module         | Dap Debugger   | USB to Serial Port module  | Digital Oscilloscope
| ------------------ | ----------------|---------------------------------------|------------------------------------------------ |
| <a href="https://github.com/openedv/LCD-module_2.8-inch_ATK-MD0280"><img src="./1_docs/3_figures/image/01_MD0280.png" width="250" height="200" /></a>     |    <a href="https://github.com/openedv/alientek-tools_DAP_Debugger"><img src="./1_docs/3_figures/image/Mini_HSDAP.png" width="200" height="100" /></a>     |    <a href="https://github.com/openedv/USB-to-Serial-Port-module-ATK-MO340P"><img src="./1_docs/3_figures/image/USB.png" width="280" /></a>  |  <a href="https://github.com/openedv/Digital-Oscilloscope_DS100"><img src="./1_docs/3_figures/image/digital.png" width="300" /></a>










