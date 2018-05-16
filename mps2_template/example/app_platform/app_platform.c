/****************************************************************************
*  Copyright 2017 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
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

/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"


#include <stdio.h>
#include "bsp.h"                     

#include ".\stdout_USART.h"
#include ".\file_io.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static struct {
    file_io_stream_t *ptOUT;
    file_io_stream_t *ptIN;
}s_tSTDIO = {0};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

 #if __IS_COMPILER_ARM_COMPILER_6__
 
 /*
 -fms-extensions -Wno-microsoft-anon-tag -Wno-nonportable-include-path -Wno-tautological-constant-out-of-range-compare
  */
 
__asm(".global __use_no_semihosting\n\t");
__asm(".global __ARM_use_no_argv\n\t");

void _sys_exit(int ch)
{
    while(1);
}

void _ttywrch(int ch)
{

}

#include <rt_sys.h>

FILEHANDLE $Sub$$_sys_open(const char *name, int openmode)
{
    return 0;
}

#endif

/*
 static uint32_t wTimer = 0;
    
    int32_t nResult = (wTimer - SysTick->VAL);
    wTimer = SysTick->VAL;
    
    if (nResult < 0) {
        nResult += SystemCoreClock  / 1000;
    }
    log("Refresh Time: %d us\r     ", nResult / 25);
 */
 
volatile static int32_t s_nCycleCounts = 0;

void start_counter(void)
{
    SAFE_ATOM_CODE(
        s_nCycleCounts =  (int32_t)SysTick->VAL - (int32_t)SysTick->LOAD;
    )
}

static void counter_overflow(void)
{
    s_nCycleCounts += SysTick->LOAD;
}

int32_t stop_counter(void)
{
    SAFE_ATOM_CODE(
        s_nCycleCounts += (int32_t)SysTick->LOAD - (int32_t)SysTick->VAL;
    )
    return s_nCycleCounts;
}

/* \note please put it into a 1ms timer handler
 */
void app_platform_1ms_event_handler(void)
{
    bsp_1ms_event_handler();
    counter_overflow();
#if DEMO_USE_FILE_IO == ENABLED
    STREAM_IN_1ms_event_handler();
#endif
}


/*! \note initialize board specific package
 *  \param none
 *  \retval true hal initialization succeeded.
 *  \retval false hal initialization failed
 */  
bool app_platform_init( void )
{
    do {
        bsp_init();
        SystemCoreClockUpdate();

        LED_Initialize();                       /* Initializ LEDs                 */
        Buttons_Initialize();                   /* Initializ Push Buttons         */
#if DEMO_USE_FILE_IO == ENABLED
    #ifdef RTE_Compiler_IO_STDOUT_User
        if (!stdout_init()) {
            break;
        }
    #endif
#endif
        return true;
    } while(false);
    
    return false;
}
  
  
/**
  Put a character to the stdout
 
  \param[in]   ch  Character to output
  \return          The character written, or -1 on write error.
*/
int stdout_putchar (int ch) 
{
#if DEMO_USE_FILE_IO == ENABLED
    if (NULL == s_tSTDIO.ptOUT) {
        while(!STREAM_OUT.Stream.WriteByte(ch));
    } else {
        FILE_IO.Channel.WriteByte(s_tSTDIO.ptOUT, ch);
    }
#endif   
    return ch;
 
}

int stdin_getchar (void)
{   
#if DEMO_USE_FILE_IO == ENABLED
    if (NULL == s_tSTDIO.ptIN) {
        uint8_t chByte;
        while(!STREAM_IN.Stream.ReadByte(&chByte));
        return chByte;
    }
    
    return FILE_IO.Channel.ReadByte(s_tSTDIO.ptIN);
#else
    return -1;
#endif
}

void retarget_stdout(file_io_stream_t *ptOut)
{
#if DEMO_USE_FILE_IO == ENABLED
    s_tSTDIO.ptOUT = ptOut;
#endif
}

void retarget_stdin(file_io_stream_t *ptIn)
{
#if DEMO_USE_FILE_IO == ENABLED
    s_tSTDIO.ptIN = ptIn;
#endif
}

void stdout_flush(void)
{
#if DEMO_USE_FILE_IO == ENABLED
    if (NULL == s_tSTDIO.ptOUT) {
        while(!STREAM_OUT.Stream.Flush());
    } else {
        FILE_IO.Channel.Flush(s_tSTDIO.ptOUT);
    }
#endif
}
  
  

  
/* EOF */
