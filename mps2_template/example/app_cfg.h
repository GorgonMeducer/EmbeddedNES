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


//! \note do not move this pre-processor statement to other places

#ifndef __SYSTEM_APP_CFG_H__
#define __SYSTEM_APP_CFG_H__


/*============================ INCLUDES ======================================*/
/*============================ MACROS ========================================*/

/*----------------------------------------------------------------------------*
 * Application Platform Configuration                                         *
 *----------------------------------------------------------------------------*/
 
 //-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

//  <h> STDOUT (via UART) configuration
//      <o>SerialPort Baudrate
//      <i>Configure the baudrate of the UART which is used as stdout (printf)
#define USART_BAUDRATE                      (115200)

//      <o>Block Input Timeout in ms <0-65535>
//      <i>When timeout, all received bytes will be imported to input stream (STREAM_IN), 0 means disabling the timeout feature.
#define STREAM_IN_RCV_TIMEOUT               (5)

//      <h> Output Stream (STREAM_OUT)
//          <o>The size of output buffer block <8-4096> 
//          <i>Output stream will be transfered in blocks, the size of an output block is defined here.
#define OUTPUT_STREAM_BLOCK_SIZE            (32)
//          <o>The number of output blocks in a dedicated heap <2-65535>
//          <i>All output blocks are allocated from a dedicated heap, the size of the heap is defined here.
#define OUTPUT_STREAM_BLOCK_COUNT           (4)
//      </h>
//      <h> Input Stream (STREAM_IN)
//          <o>The size of input buffer block <8-4096> 
//          <i>Input stream will be received in blocks, the size of an input block is defined here.
#define INPUT_STREAM_BLOCK_SIZE             (32)  
//          <o>The number of input blocks in a dedicated heap <2-65535>
//          <i>All input blocks are allocated from a dedicated heap, the size of the heap is defined here.
#define INPUT_STREAM_BLOCK_COUNT            (8)
//      </h>
//  </h>

//  <h> Multiple Delay Service Configuration
//      <o>Maximum number of delay objects supported <1-65536>
//      <i>A dedicated pool is provided to hold all the delay objects. The pool size determined the maximum allowed delay tasks supported at the same time.
#define DELAY_OBJ_POOL_SIZE                 8
//  </h>

//  <h> IO Stream service configuration
//      <o>The minial block Size in byte <1-65536>
//      <i>blocks with different sizes can be added to block pool which is used by io stream service which might have minimal size requirement for those blocks added.
#define IO_STREAM_MINIMAL_BLOCK_SIZE        8

//      <o>The maximum number of blocks in block pool
//      <i>Specifying the block pool size.
#define IO_STREAM_BLOCK_BUFFER_ITEM_COUNT   18

//      <o>The block size <16-65535>
//      <i>Specifying the block buffer size.
#define IO_STREAM_BLOCK_BUFFER_ITEM_SIZE    1024

//      <o>The maximum number of telegraphs in telegraph pool <1-65535>
//      <i>Specifying the telegraph pool size
#define IO_TELEGRAPH_BUFFER_ITEM_COUNT      8

//      <o>Timeout in  millisecond for each transaction<10-65535>
//      <i>Specifying the transaction timeout
#define IO_STREAM_TELEGRAPH_TIMEOUT         1000

//      <o>The maximum number of channels in channel pool <1-65535>
//      <i>Specifying the maxium number of channels which are allowed to open simultaneously. 
#define IO_CHANNEL_BUFFER_ITEM_COUNT        8

//      <o>Telegraph retrying times <1-15>
//      <i>Specifying how many times a transaction will be retried if it is keeping failing.
#define IO_STREAM_COMM_RETRY_TIMES          3

//      <o>The max string length in byte <1-65536>
//      <i>Strings are used by channels for name and descriptor. The maximum size of string should be specified for preventing buffer-overflow.
#define IO_STREAM_MAX_STRING_LEGNTH         255
//  </h>


//  <h> NES Simulator Configuration
//      <o>Maximum ROM Buffer Size in KB <32-512>
//      <i>A dedicated local buffer is provided to load the NES rom. The minimal size is 32KB
#define NES_ROM_BUFFER_SIZE                 64
//      <o>ROM Path
//      <i>The path of NES rom in host pc.
//#define NES_ROM_PATH                        "..\\..\\LiteNES\\ROMS\\Super Tank (Battle City Pirate) (J) [p1].nes"
//  </h>


#define USE_SERVICE_ES_SIMPLE_FRAME         ENABLED



#define USE_SERVICE_MULTIPLE_DELAY          ENABLED
#define USE_SERVICE_TELEGRAPH_ENGINE        ENABLED
#define USE_SERVICE_BLOCK                   ENABLED
#define USE_SERVICE_BLOCK_QUEUE             ENABLED
#define USE_SERVICE_STREAM_TO_BLOCK         ENABLED


#define DEMO_MULTIPLE_DELAY                 DISABLED
#define DEMO_FRAME_USE_BLOCK_MODE           ENABLED

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/


#endif
/* EOF */

