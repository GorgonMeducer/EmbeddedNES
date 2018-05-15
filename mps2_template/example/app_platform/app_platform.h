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

#ifndef __USE_APP_PLATFORM_H__
#define __USE_APP_PLATFORM_H__

/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"

#include <stdio.h>

#include ".\bsp.h"

#include ".\stdout_USART.h"
#include ".\file_io.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

#if DEMO_USE_FILE_IO == ENABLED
#define log_info(...)                               \
        do {                                        \
            printf(__VA_ARGS__);                    \
            stdout_flush();                         \
        } while(false);
#else
#define log_info(...)
#endif
/*============================ TYPES =========================================*/     
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/



/*============================ PROTOTYPES ====================================*/

/*! \brief initialize board specific package
 *  \param none
 *  \retval true hal initialization succeeded.
 *  \retval false hal initialization failed
 */  
extern bool app_platform_init( void );

/* \note please put it into a 1ms timer handler
 */
extern void app_platform_1ms_event_handler(void);

/*! \note retarget stdout to speicifed io stream, if NULL is used, usart will be 
 *        targeted directly
 *  \param ptOut  file_io_stream_t pointer
 *  \return none
 */ 
extern void retarget_stdout(file_io_stream_t *);

/*! \brief flush stdout
 */
extern void stdout_flush(void); 

/*! \brief retarget stdin to speicifed io stream, if NULL is used, usart will be 
 *        targeted directly
 *  \param ptOut  file_io_stream_t pointer
 *  \return none
 */ 
extern void retarget_stdin(file_io_stream_t *);


/*! \brief start performance counter (cycle accurate)
 */
extern void start_counter(void);

/*! \brief stop performance counter (cycle accurate)
 *! \retval cycle elapsed. 
 */
extern int32_t stop_counter(void);



#endif
