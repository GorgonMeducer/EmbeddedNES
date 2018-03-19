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

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

/*! \brief crc16
 *! \param pwCRCValue CRC Variable
 *! \param chData target byte
 *! \return CRC16 result
 */
uint16_t crc16_check(uint16_t *pwCRCValue,uint8_t chData)
{
    uint16_t wCRC = (*pwCRCValue);
    
    chData ^= (uint8_t)((uint16_t)wCRC & 0x00FF); 
    chData ^= chData << 4; 
    (*pwCRCValue) = ((((uint16_t)chData << 8) | (wCRC >> 8)) ^ (uint8_t)(chData >> 4)  
                 ^ ((uint16_t)chData << 3)); 
                 
    return (*pwCRCValue);
}


/* EOF */
