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
#include ".\app_platform\app_platform.h"

#include "fce.h"

#include <assert.h>
#include <string.h>
/*============================ MACROS ========================================*/

#ifndef DELAY_OBJ_POOL_SIZE
#   warning No defined DELAY_OBJ_POOL_SIZE, default value 4 is used
#   define DELAY_OBJ_POOL_SIZE                              (4)
#endif

#ifndef IO_STREAM_BLOCK_BUFFER_ITEM_COUNT
#   warning No defined IO_STREAM_BLOCK_BUFFER_ITEM_COUNT, default value 8 is used
#   define IO_STREAM_BLOCK_BUFFER_ITEM_COUNT                (8)
#endif

#ifndef IO_STREAM_BLOCK_BUFFER_ITEM_SIZE
#   warning No defined IO_STREAM_BLOCK_BUFFER_ITEM_SIZE, default value 64 is used
#   define IO_STREAM_BLOCK_BUFFER_ITEM_SIZE                 (64)
#endif

#ifndef IO_TELEGRAPH_BUFFER_ITEM_COUNT
#   warning No defined IO_TELEGRAPH_BUFFER_ITEM_COUNT, default value 4 is used
#   define IO_TELEGRAPH_BUFFER_ITEM_COUNT                   (4)
#endif

#ifndef IO_CHANNEL_BUFFER_ITEM_COUNT
#   warning No defined IO_CHANNEL_BUFFER_ITEM_COUNT, default value 8 is used
#   define IO_CHANNEL_BUFFER_ITEM_COUNT                   (8)
#endif

#if IO_STREAM_BLOCK_BUFFER_ITEM_COUNT < (IO_CHANNEL_BUFFER_ITEM_COUNT * 2 + 1) 
#error No sufficent blocks to support specified numbers of channels, \
        The minimal number should be (IO_CHANNEL_BUFFER_ITEM_COUNT * 2 + 1)
#endif

#ifndef NES_ROM_BUFFER_SIZE
#   warning No defined NES_ROM_BUFFER_SIZE, default value 32 is used
#   define NES_ROM_BUFFER_SIZE                              32
#endif

#ifndef NES_ROM_PATH
#   warning No defined NES_ROM_PATH, default "..\\..\\LiteNES\\ROMS\\Super Mario Bros (E).nes" is used.
#   define NES_ROM_PATH "..\\..\\LiteNES\\ROMS\\Super Mario Bros (E).nes"
#endif

#ifndef NES_DEFAULT_ROM_NUMBER
#   define NES_DEFAULT_ROM_NUMBER           1
#endif
#define __DEFAULT_ROM(__N)          (uint8_t *)NES_ROM_##__N, NES_ROM_##__N##_Length
#define __NES_DEFAULT_ROM(__N)      __DEFAULT_ROM(__N)
#define NES_DEFAULT_ROM             __NES_DEFAULT_ROM(NES_DEFAULT_ROM_NUMBER)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/  
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

/*----------------------------------------------------------------------------
  SysTick / Timer0 IRQ Handler
 *----------------------------------------------------------------------------*/


void SysTick_Handler (void) 
{
    static volatile uint32_t            s_wMSTicks = 0; 
    /*! 1ms timer event handler */
    s_wMSTicks++;
    
    if (!(s_wMSTicks % 1000)) {
        static volatile uint16_t wValue = 0;

        //printf("%s [%08x]\r\n", "Hello world!", wValue++);
        
        //STREAM_OUT.Stream.Flush();
    }
#if DEMO_USE_FILE_IO == ENABLED
    FILE_IO.Dependent.TimerTickService();
#endif
    /*! call application platform 1ms event handler */
    app_platform_1ms_event_handler();
}

static void system_init(void)
{
    if (!app_platform_init()) {
        NVIC_SystemReset();
    }

    SysTick_Config(SystemCoreClock  / 1000);  //!< Generate interrupt every 1 ms 
}
#if DEMO_USE_FILE_IO == ENABLED
static void file_io_service_init(void)
{
    NO_INIT static file_io_delay_item_t s_tDelayObjPool[DELAY_OBJ_POOL_SIZE];
    NO_INIT static struct {
        file_io_block_t tHeader;
        uint8_t chBuffer[IO_STREAM_BLOCK_BUFFER_ITEM_SIZE+8];
    } s_tBlockBuffer[IO_STREAM_BLOCK_BUFFER_ITEM_COUNT];
    
    NO_INIT static 
        file_io_telegraph_t s_tTelegraphBuffer[IO_TELEGRAPH_BUFFER_ITEM_COUNT];
    NO_INIT static 
        file_io_stream_t s_tChannelBuffer[IO_CHANNEL_BUFFER_ITEM_COUNT];


    FILE_IO_CFG (
        {   (STREAM_IN.Stream.ReadByte),
            (STREAM_OUT.Stream.WriteByte),
            (STREAM_OUT.Stream.Flush)
        },
        (mem_block_t){  (uint8_t *)s_tChannelBuffer,  
                        sizeof(s_tChannelBuffer)
                     },
        (mem_block_t){  (uint8_t *)s_tBlockBuffer,  
                        sizeof(s_tBlockBuffer)
                     },
                     
        IO_STREAM_BLOCK_BUFFER_ITEM_SIZE+8,
        (mem_block_t){  (uint8_t *)s_tTelegraphBuffer,  
                        sizeof(s_tTelegraphBuffer)
                     },
                     
        (mem_block_t){  (uint8_t *)s_tDelayObjPool,  
                        sizeof(s_tDelayObjPool)
                     },
        
        ((( IO_STREAM_BLOCK_BUFFER_ITEM_COUNT 
            - 1                                                                 //! receive buffer
            - MIN(  IO_TELEGRAPH_BUFFER_ITEM_COUNT,                             //! transmit buffer
                    IO_CHANNEL_BUFFER_ITEM_COUNT)
            ) / IO_CHANNEL_BUFFER_ITEM_COUNT))
                    
    );
}
#endif
static void app_init(void)
{   
#if DEMO_USE_FILE_IO == ENABLED
    file_io_service_init();
#endif
}

/*
private void show_progress(void)
{
    static const uint8_t c_chIcons[] = {"-\\|/-\\|/"};
    static uint8_t s_chCount = 0;
    static uint8_t s_chIndex = 0;
    
    s_chCount++;
    if (!(s_chCount & 0x07)) {
        log("\b%c", c_chIcons[s_chIndex++]);
        s_chIndex &= 0x07;
    }
}
*/

/*
For compiler 5
--wchar32  --loop_optimization_level=2 --no_unaligned_access

For compiler 6

 */
/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/
#if DEMO_USE_FILE_IO == ENABLED
NO_INIT static uint8_t s_cROMBuffer[NES_ROM_BUFFER_SIZE * 1024];

static int_fast32_t load_nes_rom(uint8_t *pchBuffer, uint32_t wSize)
{
    if (NULL == pchBuffer || wSize < (32*1024)) {
        return false;
    }
    uint32_t wTotalSize = 0;
    log_info("Loading NES ROM...");
    file_io_stream_t * ptInput = FILE_IO.Channel.Open(
                            "Input",
                            "PATH:" NES_ROM_PATH,
                            FILE_IO_INPUT          |
                            FILE_IO_BINARY_STREAM
                         );

    if (NULL == ptInput) {
        return -1;
    }
    
    while (!FILE_IO.Channel.EndOfStream(ptInput)) {
        
        //! read command output
        int32_t nSize = FILE_IO.Channel.Read(ptInput, pchBuffer, wSize);
        
        if (nSize == -1) {
            log_info("Error\r\n");
            break;
        } else if (0 == nSize) {
            continue;
        }
        
        pchBuffer+=nSize;
        wSize -= nSize;
        wTotalSize += nSize;
    }
    
    FILE_IO.Channel.Close(ptInput);
    log_info("OK\r\n ROM SIZE: %d Bytes\r\n", wTotalSize);
    
    
    return wTotalSize;
}
#endif
//! \name default roms
//! @{
extern const uint8_t NES_ROM_1[];               //!< city tank
extern const uint32_t NES_ROM_1_Length;
extern const uint8_t NES_ROM_2[];               //!< road fighter
extern const uint32_t NES_ROM_2_Length;
extern const uint8_t NES_ROM_3[];               //!< super mario bro
extern const uint32_t NES_ROM_3_Length;
extern const uint8_t NES_ROM_4[];               //!< Contra(USA)
extern const uint32_t NES_ROM_4_Length;
//! @}

int main (void) 
{
    system_init();
#if DEMO_USE_FILE_IO == ENABLED
    app_init();
#endif
    

#if DEMO_USE_FILE_IO == ENABLED
    file_io_stream_t *ptLog = FILE_IO.Channel.Open(
                            "Log",
                            "STDOUT",
                            FILE_IO_OUTPUT          |
                            FILE_IO_TEXT_STREAM
                         );
    if (NULL != ptLog) {
        retarget_stdout(ptLog);
    }
#endif
    do {
        bool bLoadingSuccess = false;
#if DEMO_USE_FILE_IO == ENABLED
        do {
            if (NULL == ptLog) {
                break;
            }
            
            int_fast32_t nSize = load_nes_rom(s_cROMBuffer, sizeof(s_cROMBuffer));
            if (nSize < 0) {
                break;
            }
            
            if (fce_load_rom((uint8_t *)s_cROMBuffer, nSize) != 0){
                break;
            }
            
            bLoadingSuccess = true;
        } while(false);
#endif
        if (!bLoadingSuccess) {
            //! use default
            if (fce_load_rom(NES_DEFAULT_ROM) != 0){
                break;
            }
        }        
        
        log_info("Initialise NES Simulator...\r\n")
        fce_init();
        log_info("Game Start...\r\n")
        fce_run();
#if DEMO_USE_FILE_IO == ENABLED
        FILE_IO.Channel.Close(ptLog);
#endif
        while(1);
    } while(false);
    
    log_info("Error: Invalid or unsupported rom.\r\n")
#if DEMO_USE_FILE_IO == ENABLED
    FILE_IO.Channel.Close(ptLog);
#endif
    while (true) {
    }

}

