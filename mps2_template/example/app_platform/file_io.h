/*********s*******************************************************************
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

#ifndef __USE_FILE_IO_H__
#define __USE_FILE_IO_H__

/*============================ INCLUDES ======================================*/
#include <stdint.h>
#include <stdbool.h>


/*============================ MACROS ========================================*/

//! \name service item size
//! \note DO NOT MODIFY ANY OF THESE MACROS
//! @{
#define DELAY_ITEM_MASK_SIZE                (0x14)
#define BLOCK_ITEM_SIZE                     (0x0C)
#define TELEGRAPH_MASK_SIZE                 (0x24)
#define STREAM_MASK_SIZE                    (0x84)
//! @}


/*============================ MACROFIED FUNCTIONS ===========================*/
#define FILE_IO_CFG(...)                                \
    do {                                                \
        file_io_cfg_t tCFG = {                          \
            __VA_ARGS__                                 \
        };                                              \
                                                        \
        FILE_IO.Init(&tCFG);                            \
    } while(false)
    

#define __REG_EX_PATTERN(__PATTERN)         "RegEx:"##__PATTERN
#define REG_EX_PATTERN(__PATTERN)           __REG_EX_PATTERN(__PATTERN)

/*============================ TYPES =========================================*/ 

typedef struct {
    uint8_t chMask[DELAY_ITEM_MASK_SIZE];
} file_io_delay_item_t;

typedef struct {
    uint8_t chMask[TELEGRAPH_MASK_SIZE];
} file_io_telegraph_t;

typedef struct {
    uint8_t chMask[BLOCK_ITEM_SIZE];
} file_io_block_t;

typedef struct {
    uint8_t chMask[STREAM_MASK_SIZE];
} file_io_stream_t;


#ifndef __UTILITIES_COMMUNICATE_H__
//! \name stream
//! @{
typedef struct {
    union {
        uint8_t *pchBuffer;         //!< stream buffer
        uint8_t *pchSrc;
        void *pObj;
    };
    uint_fast32_t wSize;       //!< stream size
} mem_block_t;
//! @}


//! \name interface: byte pipe
//! @{
typedef struct {
    //!< read a byte
    bool (*ReadByte)(uint8_t *pchByte);
    //!< write a byte
    bool (*WriteByte)(uint_fast8_t chByte);
    
    bool (*Flush)(void);
} i_byte_pipe_t;
//! @}
#endif

typedef struct {

    struct {
        //!< read a byte
        bool (*ReadByte)(uint8_t *pchByte);
        //!< write a byte
        bool (*WriteByte)(uint_fast8_t chByte);
        
        bool (*Flush)(void);
    } LowLevelInterface; 
    
    mem_block_t         tChannelBuffer;
    mem_block_t         tBlockBuffer;
    uint32_t            wBlockItemSize;
    mem_block_t         tTelegraphBuffer; 
    mem_block_t         tDelayBuffer;
    uint8_t             chBlockPerChannel;
} file_io_cfg_t;

enum {
    FILE_IO_INPUT           = 0,
    FILE_IO_OUTPUT          = 1 << 7,
    FILE_IO_BINARY_STREAM   = 0,
    FILE_IO_TEXT_STREAM     = 1 << 12,
};

typedef struct {
    union {
        uint16_t hwSettings;
        struct {
            uint16_t                    :7;                                     
            uint16_t        bIsOutput   :1;                                     //!< readonly bit, indicates the direction of current channel (NOTE: One channel can have only one direction)
            uint16_t                    :4;
            uint16_t        bIsText     :1;                                     //!< readonly bit, indicates whether target stream is a pure text based stream
            uint16_t                    :3;
        };
    };
    
    const char *pchName;                                                        //!< Stream Name
    const char *pchDescriptor;                                                  //!< Stream Descriptor
    
} file_io_stream_cfg_t;

typedef union {
    struct {
        uint16_t        IsConnected         : 1;                                //!< whether the channel is connected
        uint16_t                            : 11;                                                               
        uint16_t        IsFlushed           : 1;                                //!< whether output stream flushed
        uint16_t                            : 2;
        uint16_t        IsIOErrorDetected   : 1;                                //!< whether any io error detected
    };
    uint16_t tValue;
} file_io_stream_status_t;

//! \brief io stream serice interface
//! @{
typedef struct  
{
    bool            (*Init)             (   file_io_cfg_t *ptCFG  );
    struct {
        void        (*TimerTickService) (void);
    } Dependent;
    
    struct {
        file_io_stream_t *(*Open)       (   const char *pchName, const char *pchDescriptor, uint16_t wFeature);
        bool            (*Close)            (   file_io_stream_t *ptChannel);
        
        int_fast32_t    (*ReadByte)         (   file_io_stream_t *ptObj);
        bool            (*WriteByte)        (   file_io_stream_t *ptObj, 
                                            uint_fast8_t chByte);   
        int_fast32_t    (*Read)             (   file_io_stream_t *ptObj, 
                                            uint8_t *pchSrc, 
                                            uint_fast32_t wSize);
        bool            (*Write)            (   file_io_stream_t *ptObj, 
                                            uint8_t *pchSrc, 
                                            uint_fast32_t wSize);
        int_fast32_t    (*ReadLine)         (   file_io_stream_t *ptObj, 
                                            uint8_t *pchSrc, 
                                            uint_fast32_t wSize);
        bool            (*WriteLine)        (   file_io_stream_t *ptObj, 
                                            uint8_t *pchSrc, 
                                            uint_fast32_t wSize);
        bool            (*Flush)            (   file_io_stream_t *ptObj);
        file_io_stream_status_t  
                        (*Status)           (   file_io_stream_t *ptObj);
        bool            (*EndOfStream)      (   file_io_stream_t *ptObj);
    } Channel;
} i_file_io_t;
//! @}

typedef struct {
    int_fast32_t (*IsMatch) (   const char *pchStr, 
                                const char *pchPattern);

    int_fast32_t (*Extract) (   const char *pchStr,
                                const char *pchPattern, 
                                uint_fast8_t chGroup, 
                                uint8_t *pchSrc, 
                                uint_fast32_t wSize);
} i_regex_t;
    
/*============================ GLOBAL VARIABLES ==============================*/
extern const i_file_io_t   FILE_IO;

extern const i_regex_t     RegEx;

/*============================ PROTOTYPES ====================================*/

#endif
