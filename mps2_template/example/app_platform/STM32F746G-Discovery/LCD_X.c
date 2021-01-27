/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2019  SEGGER Microcontroller GmbH                *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.50 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information
Licensor:                 SEGGER Software GmbH
Licensed to:              ARM Ltd, 110 Fulbourn Road, CB1 9NJ Cambridge, UK
Licensed SEGGER software: emWin
License number:           GUI-00181
License model:            LES-SLA-20007, Agreement, effective since October 1st 2011 
Licensed product:         MDK-ARM Professional
Licensed platform:        ARM7/9, Cortex-M/R4
Licensed number of seats: -
----------------------------------------------------------------------
File        : LCD_X.c
Purpose     : Port routines for STM32F746 RGB
----------------------------------------------------------------------
*/

#include "GUI.h"

/*********************************************************************
*
*       Hardware configuration
*
**********************************************************************
*/

#include "stm32f7xx_hal.h"

/*********************************************************************
*
*       Exported code
*
*********************************************************************
*/
extern void LCD_X_Init      (void);
extern void LCD_X_DisplayOn (void);
extern void LCD_X_DisplayOff(void);

/*********************************************************************
*
*       LCD_X_Init
*
* Purpose:
*   This routine should be called from your application program
*   to set port pins to their initial values
*/
void LCD_X_Init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_PeriphCLKInitTypeDef RCC_PeriphClkInitStructure;

  /* Enable GPIOs clock */
  __GPIOE_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();
  __GPIOI_CLK_ENABLE();
  __GPIOJ_CLK_ENABLE();
  __GPIOK_CLK_ENABLE();

  /* GPIOs configuration */
  /*
   +------------------+-------------------+-------------------+
   +                   LCD pins assignment                    +
   +------------------+-------------------+-------------------+
   |  LCD_R0 <-> PI15 |  LCD_G0 <-> PJ7   |  LCD_B0 <-> PE4   |
   |  LCD_R1 <-> PJ0  |  LCD_G1 <-> PJ8   |  LCD_B1 <-> PJ13  |
   |  LCD_R2 <-> PJ1  |  LCD_G2 <-> PJ9   |  LCD_B2 <-> PJ14  |
   |  LCD_R3 <-> PJ2  |  LCD_G3 <-> PJ10  |  LCD_B3 <-> PJ15  |
   |  LCD_R4 <-> PJ3  |  LCD_G4 <-> PJ11  |  LCD_B4 <-> PG12  |
   |  LCD_R5 <-> PJ4  |  LCD_G5 <-> PK0   |  LCD_B5 <-> PK4   |
   |  LCD_R6 <-> PJ5  |  LCD_G6 <-> PK1   |  LCD_B6 <-> PK5   |
   |  LCD_R7 <-> PJ6  |  LCD_G7 <-> PK2   |  LCD_B7 <-> PK6   |
   ------------------------------------------------------------
   |  LCD_HSYNC <-> PI10         |  LCD_VSYNC <-> PI9         |
   |  LCD_CLK   <-> PI14         |  LCD_DE    <-> PK7         |
   |  LCD_DISP  <-> PI12 (GPIO)  |  LCD_INT   <-> PI13        |
   ------------------------------------------------------------
   |  LCD_SCL <-> PH7 (I2C3 SCL) | LCD_SDA <-> PH8 (I2C3 SDA) |
   ------------------------------------------------------------
   |  LCD_BL_CTRL <-> PK3 (GPIO) |
   -------------------------------
  */
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;

  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;

  /* GPIOE configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* GPIOI configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStructure);

  /* GPIOJ configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3  |
                           GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7  |
                           GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_11 |
                                         GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStructure);

  /* GPIOK configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  |
                           GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7;
  HAL_GPIO_Init(GPIOK, &GPIO_InitStructure);

  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;

  /* GPIOI PI12 configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStructure);

  /* GPIOK PK3 configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOK, &GPIO_InitStructure);

  /* LCD clock configuration 
       PLLSAI_VCO Input = HSE_VALUE / PLL_M = 1MHz
       PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192MHz
       PLLLCDCLK = PLLSAI_VCO Output / PLLSAIR = 192/5 = 38.4MHz
       LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_4 = 38.4/4 = 9.6MHz
  */
  RCC_PeriphClkInitStructure.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  RCC_PeriphClkInitStructure.PLLSAI.PLLSAIN = 192;
  RCC_PeriphClkInitStructure.PLLSAI.PLLSAIR = 5;
  RCC_PeriphClkInitStructure.PLLSAIDivR = RCC_PLLSAIDIVR_4;
  HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInitStructure); 
}

/*********************************************************************
*
*       LCD_X_DisplayOn
*/
void LCD_X_DisplayOn(void) {
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3,  GPIO_PIN_SET);
}

/*********************************************************************
*
*       LCD_X_DisplayOff
*/
void LCD_X_DisplayOff(void) {
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3,  GPIO_PIN_RESET);
}

/*************************** End of file ****************************/
