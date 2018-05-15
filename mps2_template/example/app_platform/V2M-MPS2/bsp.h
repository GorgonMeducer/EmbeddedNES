/****************************************************************************
*  Copyright 2018 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

#ifndef __USE_V2M_MPS2_BSP_H__
#define __USE_V2M_MPS2_BSP_H__

/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"

#include "Device.h"

#include "RTE_Components.h"             // Component selection
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Buttons.h"               // ::Board Support:Button
#include "Board_Touch.h"                // ::Board Support:Touchscreen
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.SAM4E-EK::Board Support:Graphic LCD

/*============================ MACROS ========================================*/

// <h>STDOUT USART Interface
 
//   <o>Connect to hardware via Driver_USART# <0-255>
//   <i>Select driver control block for USART interface
#define USART_DRV_NUM           0
// </h>

/*============================ MACROFIED FUNCTIONS ===========================*/

#define __USE_SERIAL_PORT_INPUT_ADAPTER(__NUM)                          \
                                                                        \
void STREAM_IN_serial_port_enable_rx_cpl_interrupt(void)                \
{                                                                       \
    CMSDK_UART##__NUM->CTRL |= CMSDK_UART_CTRL_RXIRQEN_Msk;             \
}                                                                       \
                                                                        \
void STREAM_IN_serial_port_disable_rx_cpl_interrupt(void)               \
{                                                                       \
    CMSDK_UART##__NUM->CTRL &= ~CMSDK_UART_CTRL_RXIRQEN_Msk;            \
}                                                                       \
                                                                        \
uint8_t STREAM_IN_serial_port_get_byte(void)                            \
{                                                                       \
    return CMSDK_UART##__NUM->DATA;                                     \
}                                                                       \
/* this function is called instead of the original UART0RX_Handler() */ \
void USART##__NUM##_RX_CPL_Handler(void)                                \
{                                                                       \
    /*! clear interrupt flag */                                         \
    CMSDK_UART##__NUM->INTCLEAR = CMSDK_UART##__NUM->INTSTATUS;         \
    STREAM_IN_insert_serial_port_rx_cpl_event_handler();                \
}                           

#define USE_SERIAL_PORT_INPUT_ADAPTER(__NUM)                            \
            __USE_SERIAL_PORT_INPUT_ADAPTER(__NUM)  
    
#define __USE_SERIAL_PORT_OUTPUT_ADAPTER(__NUM)                         \
                                                                        \
void STREAM_OUT_serial_port_enable_tx_cpl_interrupt(void)               \
{                                                                       \
    CMSDK_UART##__NUM->CTRL |= CMSDK_UART_CTRL_TXIRQEN_Msk;             \
}                                                                       \
                                                                        \
void STREAM_OUT_serial_port_disbale_tx_cpl_interrupt(void)              \
{                                                                       \
    CMSDK_UART##__NUM->CTRL &= ~CMSDK_UART_CTRL_TXIRQEN_Msk;            \
}                                                                       \
                                                                        \
void STREAM_OUT_serial_port_fill_byte(uint8_t chByte)                   \
{                                                                       \
    CMSDK_UART##__NUM->DATA = chByte;                                   \
}                                                                       \
                                                                        \
/* this function is called instead of the original UART0TX_Handler() */ \
void USART##__NUM##_TX_CPL_Handler(void)                                \
{                                                                       \
    /*! clear interrupt flag  */                                        \
    CMSDK_UART##__NUM->INTCLEAR = CMSDK_UART##__NUM->INTSTATUS;         \
    /*! implement our own version of uart tx interrupt */               \
                                                                        \
    STREAM_OUT_insert_serial_port_tx_cpl_event_handler();               \
}


#define USE_SERIAL_PORT_OUTPUT_ADAPTER(__NUM)                           \
            __USE_SERIAL_PORT_OUTPUT_ADAPTER(__NUM)                  

/*============================ TYPES =========================================*/     
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

extern void disable_blocking_style(void);

extern void enable_blocking_style(void);

extern bool is_blocking_style_enabled(void);

extern bool bsp_init(void);

extern void bsp_1ms_event_handler(void);
#endif
