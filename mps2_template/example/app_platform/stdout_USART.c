/*-----------------------------------------------------------------------------
 * Name:    stdout_USART.c
 * Purpose: STDOUT USART Template
 * Rev.:    1.0.0
 *-----------------------------------------------------------------------------*/
 
/* Copyright (c) 2013 - 2015 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/
 
/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"
#include <string.h>
#include "Driver_USART.h"
#include ".\bsp.h"

/*============================ MACROS ========================================*/

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------
 



#ifndef USART_BAUDRATE
#   define USART_BAUDRATE          115200
#endif

 
#define _USART_Driver_(n)  Driver_USART##n
#define  USART_Driver_(n) _USART_Driver_(n)

#define ptrUSART       (&USART_Driver_(USART_DRV_NUM))


#ifndef STREAM_IN_RCV_TIMEOUT
#   warning No defined macro STREAM_IN_RCV_TIMEOUT, default value 10 is used.
#   define STREAM_IN_RCV_TIMEOUT            (10)
#endif

#ifndef OUTPUT_STREAM_BLOCK_SIZE
#   warning No defined macro OUTPUT_STREAM_BLOCK_SIZE, default value 32 is used.
#   define OUTPUT_STREAM_BLOCK_SIZE         (32)
#endif
#ifndef OUTPUT_STREAM_BLOCK_COUNT  
#   warning No defined macro OUTPUT_STREAM_BLOCK_COUNT, default value 4 is used.
#   define OUTPUT_STREAM_BLOCK_COUNT        (4)
#endif

#ifndef INPUT_STREAM_BLOCK_SIZE
#   warning No defined macro INPUT_STREAM_BLOCK_SIZE, default value 32 is used.
#   define INPUT_STREAM_BLOCK_SIZE          (32)
#endif
#ifndef INPUT_STREAM_BLOCK_COUNT  
#   warning No defined macro INPUT_STREAM_BLOCK_COUNT, default value 8 is used.
#   define INPUT_STREAM_BLOCK_COUNT         (8)
#endif


/*============================ MACROFIED FUNCTIONS ===========================*/


    
/*============================ TYPES =========================================*/ 

//! \note define a buffer for output stream
//! @{
DEF_OUTPUT_STREAM_BUFFER(       STREAM_OUT, OUTPUT_STREAM_BLOCK_SIZE)

END_DEF_OUTPUT_STREAM_BUFFER(   STREAM_OUT)

//! \note add an adapter for serial port
STREAM_OUT_SERIAL_PORT_ADAPTER( STREAM_OUT, OUTPUT_STREAM_BLOCK_COUNT)
//! @}

//! \note define a buffer for input stream
//! @{
DEF_INPUT_STREAM_BUFFER(STREAM_IN, INPUT_STREAM_BLOCK_SIZE)

END_DEF_INPUT_STREAM_BUFFER(STREAM_IN)

//! \note add an adapter for serial port
STREAM_IN_SERIAL_PORT_ADAPTER(STREAM_IN, INPUT_STREAM_BLOCK_COUNT)
//! @}


/*------------------------------------------------------------------------------*
 * Implement Serial Port input interfaces required by STREAM_OUT adapter        *
 *------------------------------------------------------------------------------*/
USE_SERIAL_PORT_INPUT_ADAPTER(USART_DRV_NUM)


/*------------------------------------------------------------------------------*
 * Implement Serial Port output interfaces required by STREAM_OUT adapter       *
 *------------------------------------------------------------------------------*/
USE_SERIAL_PORT_OUTPUT_ADAPTER(USART_DRV_NUM)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
extern ARM_DRIVER_USART  USART_Driver_(USART_DRV_NUM);

/*============================ IMPLEMENTATION ================================*/


#if DEMO_USE_FILE_IO == ENABLED
/*! \note initialize usart for stdout
 *  \param none
 *  \retval true    initialization succeeded.
 *  \retval false   initialization failed
 */  
bool stdout_init (void) 
{    
    do {
        int32_t status;

        STREAM_OUT_output_stream_adapter_init();
        STREAM_IN_input_stream_adapter_init();

        status = ptrUSART->Initialize(NULL /*&UART0_Signal_Handler*/);
        if (status != ARM_DRIVER_OK) { 
            break; 
        }

        status = ptrUSART->PowerControl(ARM_POWER_FULL);
        if (status != ARM_DRIVER_OK) { 
            break; 
        }

        status = ptrUSART->Control( ARM_USART_MODE_ASYNCHRONOUS     |
                                    ARM_USART_DATA_BITS_8           |
                                    ARM_USART_PARITY_NONE           |
                                    ARM_USART_STOP_BITS_1           |
                                    ARM_USART_FLOW_CONTROL_NONE,
                                    USART_BAUDRATE                  );
        if (status != ARM_DRIVER_OK) { 
            break; 
        }

        status = ptrUSART->Control( ARM_USART_CONTROL_TX ,ENABLED   );
        if (status != ARM_DRIVER_OK) { 
            break; 
        }
        
        status = ptrUSART->Control( ARM_USART_CONTROL_RX ,ENABLED   );
        if (status != ARM_DRIVER_OK) { 
            break; 
        }
        
        
        return true;
    } while(false);

    return false;
}
#endif

    
    
    

