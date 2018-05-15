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


#include "Device.h"

#include "RTE_Components.h"             // Component selection
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_Touch.h"                // ::Board Support:Touchscreen
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // 

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

static volatile bool s_bBlockingStyle = true;


void disable_blocking_style(void)
{
    SAFE_ATOM_CODE(
        s_bBlockingStyle = false;
        //__NVIC_EnableIRQ(SPI_0_1_IRQn);
    )
}

void enable_blocking_style(void)
{
    SAFE_ATOM_CODE(

        __NVIC_DisableIRQ(SPI_0_1_IRQn);
        __NVIC_ClearPendingIRQ(SPI_0_1_IRQn);
    
        s_bBlockingStyle = true;
    )
}

bool is_blocking_style_enabled(void)
{
    return s_bBlockingStyle;
}

bool bsp_init(void)
{
    return true;
}

void bsp_1ms_event_handler(void)
{}
  
/* EOF */
