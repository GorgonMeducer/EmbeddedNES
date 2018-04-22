
/*-----------------------------------------------------------------------------
 * Name:    GLCD_V2M-MPS2.c
 * Purpose: Graphic LCD interface (240x320 pixels) for Graphic LCD with
 *          SPI interface for V2M-MPS2 evaluation board
 * Rev.:    1.0.3
 *----------------------------------------------------------------------------*/

/* Copyright (c) 2013 - 2016 ARM LIMITED

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

#include <stddef.h>
#include "SMM_MPS2.h"                   // Keil::Board Support:V2M-MPS2:Common
#include "GLCD_Config.h"
#include "Board_GLCD.h"
#include "../../app_platform.h"

#define UNUSED(x) (void)(x)             /* macro to get rid of 'unused parameter' warning */ 

/*********************** Hardware specific configuration **********************/

/* SPI Interface                                                              */
#define PIN_CS      (1ul << 0)

/* SPI_SR - bit definitions                                                   */
#define TFE         0x01
#define TNF         0x02
#define RNE         0x04
#define BSY         0x10


/*--------------- Graphic LCD interface hardware definitions -----------------*/

/* Pin CS setting to 0 or 1                                                   */

#define debug_log(...)          //printf(__VA_ARGS__)

#define USE_BLOCKING_TRANSFER       ENABLED
#define USE_INTERRUPT               DISABLED
#if USE_BLOCKING_TRANSFER == ENABLED

#if USE_INTERRUPT == DISABLED
#define LCD_CS(x)   do {\
                        if (x) {\
                            /*while (!(MPS2_SSP0->SR & (TFE)))*/;\
                            while ((MPS2_SSP0->SR & (BSY)));\
                        }\
                        ((x) ? (MPS2_FPGAIO->MISC |= PIN_CS)    : (MPS2_FPGAIO->MISC &= ~PIN_CS));\
                    } while(0)
#else

#define LCD_CS(x)   lcd_action_cs(x)
#endif
#else
#define LCD_CS(...)     lcd_cs(__VA_ARGS__)
#endif

#define SPI_START   (0x70)              /* Start byte for SPI transfer        */
#define SPI_RD      (0x01)              /* WR bit 1 within start              */
#define SPI_WR      (0x00)              /* WR bit 0 within start              */
#define SPI_DATA    (0x02)              /* RS bit 1 within start byte         */
#define SPI_INDEX   (0x00)              /* RS bit 0 within start byte         */


/************************ Global variables ************************************/

static uint16_t   foreground_color = GLCD_COLOR_BLACK;
static uint16_t   background_color = GLCD_COLOR_WHITE;
static GLCD_FONT *active_font      = NULL;

//static uint8_t    Himax;
#if (GLCD_SWAP_XY == 0)
static uint32_t   Scroll;
#endif

#define LCD_STREAM_BLOCK_SIZE               8
#define LCD_STREAM_BLOCK_COUNT              512
#define LCD_BEHAVIOUR_HEAP_SIZE             128
#define LCD_ACTION_QUEUE_SIZE               (65536 * 10)

#if USE_INTERRUPT == ENABLED

typedef struct {
    INHERIT(__single_list_node_t)
    bool bIsSet;
    uint_fast16_t hwCount;
}lcd_cs_behaviour_item_t;


DEF_EPOOL(LCD_Behaviour, lcd_cs_behaviour_item_t)

END_DEF_EPOOL(LCD_Behaviour)

typedef struct {
    EPOOL(LCD_Behaviour)    tLCDCSBehaviourPool;
    uint_fast16_t           hwProducerCounter;
    uint_fast16_t           hwConsumerCounter;
    lcd_cs_behaviour_item_t *ptHead;
    lcd_cs_behaviour_item_t *ptTail;
}lcd_accelerator_t;

lcd_accelerator_t s_tLCDA;

//! \note define a buffer for output stream
//! @{
DEF_OUTPUT_STREAM_BUFFER(       LCD_STREAM_OUT, LCD_STREAM_BLOCK_SIZE)

END_DEF_OUTPUT_STREAM_BUFFER(   LCD_STREAM_OUT)

DEF_SAFE_QUEUE_U16(ActionQueue, uint32_t, bool)
END_DEF_SAFE_QUEUE_U16(ActionQueue);



#define __SPI_STREAM_OUT_SERIAL_PORT_ADAPTER(__NAME, __BLOCK_COUNT)                 \
    extern void __NAME##_serial_port_enable_tx_cpl_interrupt(void);             \
    extern void __NAME##_serial_port_disbale_tx_cpl_interrupt(void);            \
    extern void __NAME##_serial_port_fill_byte(uint8_t chByte);                 \
                                                                                \
    NO_INIT private volatile struct {                                           \
        __NAME##_stream_buffer_block_t *ptBlock;                                \
        uint8_t *pchBuffer;                                                     \
        uint_fast16_t hwSize;                                                   \
        uint_fast16_t hwIndex;                                                  \
        uint_fast16_t hwTimeoutCounter;                                         \
    } s_t##__NAME##StreamOutService;                                            \
    private void __NAME##_request_send(void)                                    \
    {                                                                           \
        s_t##__NAME##StreamOutService.ptBlock = __NAME##.Block.Exchange(        \
            s_t##__NAME##StreamOutService.ptBlock);                             \
        __NAME##_stream_buffer_block_t *ptBlock =                               \
            s_t##__NAME##StreamOutService.ptBlock;                              \
        if (NULL != ptBlock) {                                                  \
            s_t##__NAME##StreamOutService.pchBuffer =                           \
                (uint8_t *)BLOCK.Buffer.Get(ref_obj_as((*ptBlock), block_t));   \
            s_t##__NAME##StreamOutService.hwSize =                              \
                BLOCK.Size.Get(ref_obj_as((*ptBlock), block_t));                \
            s_t##__NAME##StreamOutService.hwIndex = 0;                          \
                                                                                \
            __NAME##_serial_port_enable_tx_cpl_interrupt();                     \
        }                                                                       \
    }                                                                           \
    void __NAME##_insert_serial_port_tx_cpl_event_handler(void)                 \
    {                                                                           \
        if (    (NULL == s_t##__NAME##StreamOutService.pchBuffer)               \
            ||  (0 == s_t##__NAME##StreamOutService.hwSize)) {                  \
            /* it appears output service is cancelled */                        \
            __NAME##_serial_port_disbale_tx_cpl_interrupt();                    \
            return ;                                                            \
        }                                                                       \
                                                                                \
        if (    s_t##__NAME##StreamOutService.hwIndex                           \
            >=  s_t##__NAME##StreamOutService.hwSize) {                         \
            __NAME##_serial_port_disbale_tx_cpl_interrupt();                    \
            __NAME##_request_send();                                            \
        } else {                                                                \
            __NAME##_serial_port_fill_byte(                                     \
                s_t##__NAME##StreamOutService.pchBuffer[                        \
                    s_t##__NAME##StreamOutService.hwIndex++]);                  \
        }                                                                       \
    }                                                                           \
    private void __NAME##_output_stream_req_send_event_handler(                 \
                stream_buffer_t *ptObj)                                         \
    {                                                                           \
        if (NULL == ptObj) {                                                    \
            return ;                                                            \
        }                                                                       \
                                                                                \
        __NAME##_request_send();                                                \
    }                                                                           \
    void __NAME##_output_stream_adapter_init(void)                              \
    {                                                                           \
        NO_INIT private __NAME##_stream_buffer_block_t                          \
                        s_tBlocks[__BLOCK_COUNT];                               \
        OUTPUT_STREAM_BUFFER_CFG(                                               \
            __NAME,                                                             \
            &__NAME##_output_stream_req_send_event_handler                      \
        );                                                                      \
                                                                                \
        __NAME.AddBuffer(s_tBlocks, sizeof(s_tBlocks));                         \
        memset((void *)&s_t##__NAME##StreamOutService, 0,                       \
            sizeof(s_t##__NAME##StreamOutService));                             \
    }
    
#define SPI_STREAM_OUT_SERIAL_PORT_ADAPTER(__NAME, __BLOCK_COUNT)                   \
            __SPI_STREAM_OUT_SERIAL_PORT_ADAPTER(__NAME, (__BLOCK_COUNT))



//! \note add an adapter for serial port
SPI_STREAM_OUT_SERIAL_PORT_ADAPTER( LCD_STREAM_OUT, LCD_STREAM_BLOCK_COUNT)
//! @}



static void LCD_STREAM_OUT_serial_port_disbale_tx_cpl_interrupt(void)
{
    SAFE_ATOM_CODE (
        __NVIC_DisableIRQ(SPI_0_1_IRQn);
        //__NVIC_ClearPendingIRQ(SPI_0_1_IRQn);
    )
}
static void LCD_STREAM_OUT_serial_port_enable_tx_cpl_interrupt(void)
{
    SAFE_ATOM_CODE (
        //__NVIC_ClearPendingIRQ(SPI_0_1_IRQn);
        __NVIC_EnableIRQ(SPI_0_1_IRQn);
    )
}
static void LCD_STREAM_OUT_serial_port_fill_byte(uint8_t chByte)
{
    s_tLCDA.hwConsumerCounter++;
    
    #if 1
    //while(!(MPS2_SSP0->SR & TNF));
    while((MPS2_SSP0->SR & BSY));
    MPS2_SSP0->DR = chByte;
    //while(!(MPS2_SSP0->SR & TFE));
    //while((MPS2_SSP0->SR & BSY));
    #else
      //debug_log(" <%02x>", chByte);
      MPS2_SSP0->DR = chByte;
      while (!(MPS2_SSP0->SR & RNE));            /* Wait for send to finish            */
      chByte = ((uint8_t)MPS2_SSP0->DR);
    #endif
    
    debug_log("<%02x>", chByte);
}

#if USE_BLOCKING_TRANSFER == ENABLED
static QUEUE(ActionQueue) s_tAQueue;

static void action_queue_init(void)
{
    static NO_INIT uint16_t s_hwActionQueueSize[LCD_ACTION_QUEUE_SIZE];
    
    QUEUE_INIT(ActionQueue, &s_tAQueue, s_hwActionQueueSize, LCD_ACTION_QUEUE_SIZE);
}



static volatile uint8_t s_chByte = 0;

void SPI_0_1_Handler(void)
{
    uint16_t hwData=0;
    if (DEQUEUE(ActionQueue, &s_tAQueue, &hwData)) {
    
        switch(hwData) {
            case 0x8000:
                debug_log("L");
                MPS2_FPGAIO->MISC &= ~PIN_CS;
                break;
            case 0xc000:
                //while (!(MPS2_SSP0->SR & (TFE)));
                //while ((MPS2_SSP0->SR & (BSY)));
                debug_log("H\r\n");
                MPS2_FPGAIO->MISC |= PIN_CS;
                break;
            default:
                debug_log("<%02x>", hwData);
                //LCD_STREAM_OUT_serial_port_fill_byte(hwData);
                MPS2_SSP0->DR = hwData;
                break;
        }
    
    } else {
        __NOP();
        __NVIC_DisableIRQ(SPI_0_1_IRQn);
    }
    
    //LCD_STREAM_OUT_serial_port_fill_byte(s_chByte);
    
}

void lcd_action_cs(bool bIsSet)
{
    if (!bIsSet) {
        ENQUEUE(ActionQueue, &s_tAQueue, 0x8000);
    } else {
        ENQUEUE(ActionQueue, &s_tAQueue, 0xC000);
        __NVIC_EnableIRQ(SPI_0_1_IRQn);
        //while(GET_QUEUE_COUNT(ActionQueue, &s_tAQueue));
    }
}

#endif



static void lcd_behaviour_list_init(void)
{
    static NO_INIT lcd_cs_behaviour_item_t s_tListBuffer[LCD_BEHAVIOUR_HEAP_SIZE];
    
    memset(&s_tLCDA, 0, sizeof(s_tLCDA));
    
    EPOOL_ADD_HEAP(LCD_Behaviour,   &s_tLCDA.tLCDCSBehaviourPool, 
                                    &s_tListBuffer, sizeof(s_tListBuffer));                    
    
}



static void lcd_cs(bool bIsSet)
{

    if (is_blocking_style_enabled()) {
        if (bIsSet) {
            while (!(MPS2_SSP0->SR & (TFE)));
            while ((MPS2_SSP0->SR & (BSY)));
            MPS2_FPGAIO->MISC |= PIN_CS;
            debug_log("H\r\n");
        } else {
            MPS2_FPGAIO->MISC &= ~PIN_CS;
            debug_log("L ");
        }
    
        return ;
    } 

    lcd_cs_behaviour_item_t *ptThis = NULL;
    
    do {
        ptThis = EPOOL_NEW(LCD_Behaviour, &s_tLCDA.tLCDCSBehaviourPool);
        //LCD_STREAM_OUT.Stream.Flush();
    } while(NULL == ptThis);
/*
    if (bIsSet) {
        debug_log("R_H\r\n");
    } else {
        debug_log("R_L");
    }
*/
    this.bIsSet = bIsSet;
    this.hwCount = s_tLCDA.hwProducerCounter;
    s_tLCDA.hwProducerCounter = 0;
    
    SAFE_ATOM_CODE(
        LIST_QUEUE_ENQUEUE(s_tLCDA.ptHead, s_tLCDA.ptTail, ptThis);
    )
}

#if USE_BLOCKING_TRANSFER != ENABLED
void SPI_0_1_Handler(void)
{
    while (NULL != s_tLCDA.ptHead) {
        lcd_cs_behaviour_item_t *ptThis = s_tLCDA.ptHead;
        
        if (s_tLCDA.hwConsumerCounter == this.hwCount) {
            s_tLCDA.hwConsumerCounter = 0;
            if (this.bIsSet) {
                //while (!(MPS2_SSP0->SR & (TFE)));
                while ((MPS2_SSP0->SR & (BSY)))
                MPS2_FPGAIO->MISC |= PIN_CS;  
                debug_log(" H\r\n");
            } else { 
                while ((MPS2_SSP0->SR & (BSY)));
                MPS2_FPGAIO->MISC &= ~PIN_CS; 
                debug_log("L ");
            }
            SAFE_ATOM_CODE(
                LIST_QUEUE_DEQUEUE(s_tLCDA.ptHead, s_tLCDA.ptTail, ptThis);
            )
            
            EPOOL_FREE(LCD_Behaviour, &s_tLCDA.tLCDCSBehaviourPool, ptThis);
        } else {
            break;
        }
    }

    LCD_STREAM_OUT_insert_serial_port_tx_cpl_event_handler();
}
#endif

#endif

/**
  \fn          void delay_ms (int ms)
  \brief       Delay execution for a specified number of milliseconds
  \param[in]   ms     number of while loop iterations
*/
static void delay_ms (int ms) {
  debug_log("delay %d ms\r\n", ms);
  ms *= (SystemCoreClock/10000);
  while (ms--) { __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); __NOP(); }
}



/************************ Local auxiliary functions ***************************/


/**
  \fn          uint8_t spi_tran (uint8_t byte)
  \brief       Transfer 1 byte over the serial communication
  \param[in]   byte  data to transmit
  \returns     byte read while sending
*/
static void spi_tran (uint8_t byte) 
{
#if USE_BLOCKING_TRANSFER == ENABLED

#if USE_INTERRUPT == DISABLED
  
  while(!(MPS2_SSP0->SR & TNF));
  MPS2_SSP0->DR = byte;
  debug_log(" <%02x>", byte);
  
#else
  if (!ENQUEUE(ActionQueue, &s_tAQueue, byte)) {
      __NVIC_EnableIRQ(SPI_0_1_IRQn);
      while(!ENQUEUE(ActionQueue, &s_tAQueue, byte));
  }
#endif
#else
    do {
        if (is_blocking_style_enabled()) {
            while(!(MPS2_SSP0->SR & TNF));
            MPS2_SSP0->DR = byte;
            debug_log(" <%02x>", byte);
            break;
        } 

        while(!LCD_STREAM_OUT.Stream.WriteByte(byte)) {
            __NOP();
        }
        //debug_log(" [%02x]", byte);
        s_tLCDA.hwProducerCounter++;
    } while(false);
#endif

}

/**
  \fn          void wr_cmd (uint8_t cmd)
  \brief       Write a command to the LCD controller
  \param[in]   cmd  Command to write
*/
static void wr_cmd (uint8_t cmd) {
  LCD_CS(0);
  spi_tran(SPI_START | SPI_WR | SPI_INDEX);   /* Write : RS = 0, RW = 0       */
  spi_tran(0);
  spi_tran(cmd);
  LCD_CS(1);
#if USE_BLOCKING_TRANSFER != ENABLED
  while(!LCD_STREAM_OUT.Stream.Flush());
#endif
}

/**
  \fn          void wr_dat (uint16_t dat)
  \brief       Write data to the LCD controller
  \param[in]   dat  Data to write
*/
static void wr_dat (uint16_t dat) {
  LCD_CS(0);
  spi_tran(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
  spi_tran((dat >>   8));                     /* Write D8..D15                */
  spi_tran((dat & 0xFF));                     /* Write D0..D7                 */
  LCD_CS(1);
#if USE_BLOCKING_TRANSFER != ENABLED
  while(!LCD_STREAM_OUT.Stream.Flush());
#endif
}

/**
  \fn          void wr_dat_start (void)
  \brief       Start writing data to LCD controller
*/
static void wr_dat_start (void) {
  LCD_CS(0);
  spi_tran(SPI_START | SPI_WR | SPI_DATA);    /* Write : RS = 1, RW = 0       */
}

/**
  \fn          void wr_dat_stop (void)
  \brief       Stop writing data to LCD controller
*/
static __inline void wr_dat_stop (void) {
  LCD_CS(1);
#if USE_BLOCKING_TRANSFER != ENABLED
  while(!LCD_STREAM_OUT.Stream.Flush());
#endif
}

/**
  \fn          void wr_dat_only (uint16_t dat)
  \brief       Write data to the LCD controller (without start/stop sequence)
  \param[in]   dat  Data to write
*/
static void wr_dat_only (uint16_t dat) {
  spi_tran((dat >> 8) & 0xFF);                     /* Write D8..D15                */
  spi_tran( dat       & 0xFF);                     /* Write D0..D7                 */
}

/**
  \fn          uint16_t rd_dat (void)
  \brief       Read data from the LCD controller
  \returns     Data read
*/
//static __inline uint16_t rd_dat (void) {
//  /* not available for Himax */
//  return (0);
//}

/**
  \fn          void wr_reg (uint8_t reg, uint16_t val)
  \brief       Write a value to the LCD register
  \param[in]   reg  Register to be written
  \param[in]   val  Value to write to the register
*/
static void wr_reg (uint8_t reg, uint16_t val) {
  wr_cmd(reg);
  wr_dat(val);
  
}

/**
  \fn          uint16_t rd_reg (uint8_t reg)
  \brief       Read a value from the LCD register
  \param[in]   reg  Register to be read
  \returns     Value read from the register
*/
//static __inline uint16_t rd_reg (uint8_t reg) {
//  wr_cmd(reg);
//  return (rd_dat());
//}


/**
  \fn          void GLCD_SetWindow (uint32_t x, uint32_t y, uint32_t w, uint32_t h)
  \brief       Set draw window region
  \param[in]   x  Horizontal position in pixel
  \param[in]   y  Vertical position in pixel
  \param[in]   w  Window width in pixel
  \param[in]   h  Window height in pixel
*/
static void GLCD_SetWindow (uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
  uint32_t xe, ye;

//  if (Himax) {
    xe = x+w-1;
    ye = y+h-1;

    wr_reg(0x02, (uint16_t)(x  >>    8));           /* Column address start MSB           */
    wr_reg(0x03, (uint16_t)(x  &  0xFF));           /* Column address start LSB           */
    wr_reg(0x04, (uint16_t)(xe >>    8));           /* Column address end MSB             */
    wr_reg(0x05, (uint16_t)(xe &  0xFF));           /* Column address end LSB             */

    wr_reg(0x06, (uint16_t)(y  >>    8));           /* Row address start MSB              */
    wr_reg(0x07, (uint16_t)(y  &  0xFF));           /* Row address start LSB              */
    wr_reg(0x08, (uint16_t)(ye >>    8));           /* Row address end MSB                */
    wr_reg(0x09, (uint16_t)(ye &  0xFF));           /* Row address end LSB                */
//  }
}




/************************ Exported functions **********************************/

/**
  \fn          int32_t GLCD_Initialize (void)
  \brief       Initialize Graphic LCD (HX8347-D LCD Controller)
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_Initialize (void) {


  
  /* Configure SSP used for LCD                                               */
  MPS2_SSP0->CR1   =   0;                 /* Synchronous serial port disnable */
  MPS2_SSP0->DMACR =   0;                 /* Disable FIFO DMA                 */
  MPS2_SSP0->IMSC  =   MPS2_SSP_IMSC_TRANS_FIFO_INT_MSK     |
                       0;                 /* Mask all FIFO/IRQ interrupts     */
  MPS2_SSP0->ICR   = ((1ul <<  0) |       /* Clear SSPRORINTR interrupt       */
                      (1ul <<  1) );      /* Clear SSPRTINTR interrupt        */
  MPS2_SSP0->CR0   = ((7ul <<  0) |       /* 8 bit data size                  */
                      (0ul <<  4) |       /* Motorola frame format            */
                      (0ul <<  6) |       /* CPOL = 0                         */
                      (0ul <<  7) |       /* CPHA = 0                         */
                      (0ul <<  8) );      /* Set serial clock rate            */
  MPS2_SSP0->CPSR  =  (2ul <<  0);        /* set SSP clk to 6MHz (6.6MHz max) */
  MPS2_SSP0->CR1   = ((1ul <<  1) |       /* Synchronous serial port enable   */
                      (0ul <<  2) );      /* Device configured as master      */

#if USE_BLOCKING_TRANSFER != ENABLED
    //! initialise SPI stream2block mode
    do {
        LCD_STREAM_OUT_output_stream_adapter_init();
        lcd_behaviour_list_init();
        
        __NVIC_SetPriority(SPI_0_1_IRQn, 0xFE);
        
        enable_blocking_style();
    } while(false);
#else

#   if USE_INTERRUPT == ENABLED

    __NVIC_DisableIRQ(SPI_0_1_IRQn);
    action_queue_init();
#   endif
#endif

    disable_blocking_style();
  /* CLCD screen setup (Default CLCD screen interface state)                  */
  MPS2_FPGAIO->MISC |=  ((1ul <<  0) |   /* deassert nCS0 */
                         (1ul <<  3)  ); /* deassert Reset */
  MPS2_FPGAIO->MISC &= ~((1ul <<  4) |   /* deassert RS */
                         (1ul <<  5)    /* deassert RD */
                          /* | (1ul <<  6)*/ );  /* switch off backlight */

  // Reset CLCD screen
  MPS2_FPGAIO->MISC &= ~(1ul <<  3);     /* assert Reset */
	delay_ms(1);
  MPS2_FPGAIO->MISC |=  (1ul <<  3);     /* deassert Reset */
	delay_ms(10);

//  if (driverCode == 0x47) {             /* LCD with HX8347-D LCD Controller   */
//    Himax = 1;                          /* Set Himax LCD controller flag      */
    /* Driving ability settings ----------------------------------------------*/
    wr_reg(0xEA, 0x00);                 /* Power control internal used (1)    */
    wr_reg(0xEB, 0x20);                 /* Power control internal used (2)    */
    wr_reg(0xEC, 0x0C);                 /* Source control internal used (1)   */
    wr_reg(0xED, 0xC7);                 /* Source control internal used (2)   */
    wr_reg(0xE8, 0x38);                 /* Source output period Normal mode   */
    wr_reg(0xE9, 0x10);                 /* Source output period Idle mode     */
    wr_reg(0xF1, 0x01);                 /* RGB 18-bit interface ;0x0110       */
    wr_reg(0xF2, 0x10);
    
    
    /* Adjust the Gamma Curve ------------------------------------------------*/
    wr_reg(0x40, 0x01);
    wr_reg(0x41, 0x00);
    wr_reg(0x42, 0x00);
    wr_reg(0x43, 0x10);
    wr_reg(0x44, 0x0E);
    wr_reg(0x45, 0x24);
    wr_reg(0x46, 0x04);
    wr_reg(0x47, 0x50);
    wr_reg(0x48, 0x02);
    wr_reg(0x49, 0x13);
    wr_reg(0x4A, 0x19);
    wr_reg(0x4B, 0x19);
    wr_reg(0x4C, 0x16);
    
    wr_reg(0x50, 0x1B);
    wr_reg(0x51, 0x31);
    wr_reg(0x52, 0x2F);
    wr_reg(0x53, 0x3F);
    wr_reg(0x54, 0x3F);
    wr_reg(0x55, 0x3E);
    wr_reg(0x56, 0x2F);
    wr_reg(0x57, 0x7B);
    wr_reg(0x58, 0x09);
    wr_reg(0x59, 0x06);
    wr_reg(0x5A, 0x06);
    wr_reg(0x5B, 0x0C);
    wr_reg(0x5C, 0x1D);
    wr_reg(0x5D, 0xCC);
    //
    
    /* Power voltage setting -------------------------------------------------*/
    wr_reg(0x1B, 0x1B);
    wr_reg(0x1A, 0x01);
    wr_reg(0x24, 0x2F);
    wr_reg(0x25, 0x57);
    wr_reg(0x23, 0x88);
    
    /* Power on setting ------------------------------------------------------*/
    wr_reg(0x18, 0x36);                 /* Internal oscillator frequency adj  */
    wr_reg(0x19, 0x01);                 /* Enable internal oscillator         */
    wr_reg(0x01, 0x00);                 /* Normal mode, no scrool             */
    wr_reg(0x1F, 0x88);                 /* Power control 6 - DDVDH Off        */
    delay_ms(200);
    wr_reg(0x1F, 0x82);                 /* Power control 6 - Step-up: 3 x VCI */
    delay_ms(50);
    wr_reg(0x1F, 0x92);                 /* Power control 6 - Step-up: On      */
    delay_ms(50);
    wr_reg(0x1F, 0xD2);                 /* Power control 6 - VCOML active     */
    delay_ms(50);
    
    //disable_blocking_style();
    
    /* Color selection -------------------------------------------------------*/
    wr_reg(0x17, 0x55);                 /* RGB, System interface: 16 Bit/Pixel*/
    wr_reg(0x00, 0x00);                 /* Scrolling off, no standby          */

    /* Interface config ------------------------------------------------------*/
    wr_reg(0x2F, 0x11);                 /* LCD Drive: 1-line inversion        */
    wr_reg(0x31, 0x00);
    wr_reg(0x32, 0x00);                 /* DPL=0, HSPL=0, VSPL=0, EPL=0       */

    
    /* Display on setting ----------------------------------------------------*/
    wr_reg(0x28, 0x38);                 /* PT(0,0) active, VGL/VGL            */
    delay_ms(200);
    wr_reg(0x28, 0x3C);                 /* Display active, VGL/VGL            */

    
    //disable_blocking_style();

    /* Miror and Swap                                                         */
    wr_reg (0x16, ((GLCD_MIRROR_Y << 7) |   /* MY                             */
                   (GLCD_MIRROR_X << 6) |   /* MX                             */
                   (GLCD_SWAP_XY  << 5) |   /* MV                             */
                   (0 << 4) |               /* ML                             */
                   (1 << 3)));              /* BGR                            */

    

    /* Display scrolling settings --------------------------------------------*/
    wr_reg(0x0E, 0x00);                 /* TFA MSB                            */
    wr_reg(0x0F, 0x00);                 /* TFA LSB                            */
    wr_reg(0x10, 320 >> 8);             /* VSA MSB                            */
    wr_reg(0x11, 320 &  0xFF);          /* VSA LSB                            */
    wr_reg(0x12, 0x00);                 /* BFA MSB                            */
    wr_reg(0x13, 0x00);                 /* BFA LSB                            */
//  }

    //while(!LCD_STREAM_OUT.Stream.Flush());
    stdout_flush();
    
    
  MPS2_FPGAIO->MISC |= (1ul <<  6);    /* turn on backlight                  */

    

  return 0;
}

/**
  \fn          int32_t GLCD_Uninitialize (void)
  \brief       De-initialize Graphic LCD
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_Uninitialize (void) {

  return 0;
}

/**
  \fn          int32_t GLCD_SetForegroundColor (uint32_t color)
  \brief       Set foreground color
  \param[in]   color  Color value
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_SetForegroundColor (uint32_t color) {
  foreground_color = (uint16_t)color;
  return 0;
}

/**
  \fn          int32_t GLCD_SetBackgroundColor (uint32_t color)
  \brief       Set background color
  \param[in]   color  Color value
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_SetBackgroundColor (uint32_t color) {
  background_color = (uint16_t)color;
  return 0;
}

/**
  \fn          int32_t GLCD_ClearScreen (void)
  \brief       Clear screen (with active background color)
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_ClearScreen (void) {
  uint32_t i;
  
  //disable_blocking_style();
  GLCD_SetWindow(0, 0, GLCD_WIDTH, GLCD_HEIGHT);

  wr_cmd(0x22);
  wr_dat_start();
  for (i = 0; i < (GLCD_WIDTH * GLCD_HEIGHT); i++) {
    wr_dat_only(background_color);
  }
  wr_dat_stop();
  //enable_blocking_style();
  return 0;
}

/**
  \fn          int32_t GLCD_SetFont (GLCD_FONT *font)
  \brief       Set active font
  \param[in]   font   Pointer to font structure
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_SetFont (GLCD_FONT *font) {
  active_font = font;
  return 0;
}

/**
  \fn          int32_t GLCD_DrawPixel (uint32_t x, uint32_t y)
  \brief       Draw pixel (in active foreground color)
  \param[in]   x  x position in pixels (0 = left corner)
  \param[in]   y  y position in pixels (0 = upper corner)
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawPixel (uint32_t x, uint32_t y) {
#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

//  if (Himax) {
    wr_reg(0x02, (uint16_t)(x >>    8));            /* Column address start MSB           */
    wr_reg(0x03, (uint16_t)(x &  0xFF));            /* Column address start LSB           */
    wr_reg(0x04, (uint16_t)(x >>    8));            /* Column address end MSB             */
    wr_reg(0x05, (uint16_t)(x &  0xFF));            /* Column address end LSB             */

    wr_reg(0x06, (uint16_t)(y >>    8));            /* Row address start MSB              */
    wr_reg(0x07, (uint16_t)(y &  0xFF));            /* Row address start LSB              */
    wr_reg(0x08, (uint16_t)(y >>    8));            /* Row address end MSB                */
    wr_reg(0x09, (uint16_t)(y &  0xFF));            /* Row address end LSB                */
//  }

  wr_cmd(0x22);
  wr_dat(foreground_color);

  return 0;
}

/**
  \fn          int32_t GLCD_DrawHLine (uint32_t x, uint32_t y, uint32_t length)
  \brief       Draw horizontal line (in active foreground color)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   length Line length
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawHLine (uint32_t x, uint32_t y, uint32_t length) {
#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  GLCD_SetWindow(x, y, length, 1);
  wr_cmd(0x22);
  wr_dat_start();

  while (length--) {
    wr_dat_only(foreground_color);
  }
  wr_dat_stop();
  return 0;
}

/**
  \fn          int32_t GLCD_DrawVLine (uint32_t x, uint32_t y, uint32_t length)
  \brief       Draw vertical line (in active foreground color)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   length Line length in pixels
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawVLine (uint32_t x, uint32_t y, uint32_t length) {
#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  GLCD_SetWindow(x, y, 1, length);
  wr_cmd(0x22);
  wr_dat_start();

  while (length--) {
    wr_dat_only(foreground_color);
  }
  wr_dat_stop();
  return 0;
}

/**
  \fn          int32_t GLCD_DrawRectangle (uint32_t x, uint32_t y, uint32_t width, uint32_t height)
  \brief       Draw rectangle (in active foreground color)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   width  Rectangle width in pixels
  \param[in]   height Rectangle height in pixels
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawRectangle (uint32_t x, uint32_t y, uint32_t width, uint32_t height) {

  GLCD_DrawHLine (x,         y,          width);
  GLCD_DrawHLine (x,         y + height, width);
  GLCD_DrawVLine (x,         y,          height);
  GLCD_DrawVLine (x + width, y,          height);

  return 0;
}

/**
  \fn          int32_t GLCD_DrawChar (uint32_t x, uint32_t y, int32_t ch)
  \brief       Draw character (in active foreground color)
  \param[in]   x      x position in pixels (0 = left corner)
  \param[in]   y      y position in pixels (0 = upper corner)
  \param[in]   ch     Character
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawChar (uint32_t x, uint32_t y, int32_t ch) {
  uint32_t i, j;
  uint32_t wb;
  const uint8_t *ptr_ch_bmp;

#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  if (active_font == NULL) return -1;

  ch        -= active_font->offset;
  wb         = (active_font->width + 7)/8;
  ptr_ch_bmp = active_font->bitmap + ((uint32_t)ch * wb * active_font->height);

  GLCD_SetWindow(x, y, active_font->width, active_font->height);

  wr_cmd(0x22);
  wr_dat_start();

  for (i = 0; i < active_font->height; i++) {
    for (j = 0; j < active_font->width; j++) {
      wr_dat_only(((*ptr_ch_bmp >> (j & 7)) & 1) ? foreground_color : background_color);
      if (((j & 7) == 7) && (j != (active_font->width - 1))) ptr_ch_bmp++;
    }
    ptr_ch_bmp++;
  }
  wr_dat_stop();
  return 0;
}

/**
  \fn          int32_t GLCD_DrawString (uint32_t x, uint32_t y, const char *str)
  \brief       Draw string (in active foreground color)
  \param[in]   x      x position in pixels (0 = left corner)
  \param[in]   y      y position in pixels (0 = upper corner)
  \param[in]   str    Null-terminated String
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawString (uint32_t x, uint32_t y, const char *str) {

  while (*str) { GLCD_DrawChar(x, y, *str++); x += active_font->width; }

  return 0;
}


/**
  \fn          int32_t GLCD_DrawBargraph (uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t val)
  \brief       Draw bargraph (in active foreground color)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   width  Full bargraph width in pixels
  \param[in]   height Full bargraph height in pixels
  \param[in]   val    Active bargraph value (maximum value is 100)
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawBargraph (uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t val) {
  uint32_t i, j;

  val = (val * width) / 100;            /* Scale value                        */

#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  GLCD_SetWindow(x, y, width, height);

  wr_cmd(0x22);
  wr_dat_start();
  for (i = 0; i < height; i++) {
    for (j = 0; j < width; j++) {
      if(j >= val) {
        wr_dat_only(background_color);
      } else {
        wr_dat_only(foreground_color);
      }
    }
  }
  wr_dat_stop();

  return 0;
}

/**
  \fn          int32_t GLCD_DrawBitmap (uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t *bitmap)
  \brief       Draw bitmap (bitmap from BMP file without header)
  \param[in]   x      Start x position in pixels (0 = left corner)
  \param[in]   y      Start y position in pixels (0 = upper corner)
  \param[in]   width  Bitmap width in pixels
  \param[in]   height Bitmap height in pixels
  \param[in]   bitmap Bitmap data
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_DrawBitmap (uint32_t x, uint32_t y, uint32_t width, uint32_t height, const uint8_t *bitmap) {
  int32_t   i, j;
  const uint16_t *ptr_bmp;

  ptr_bmp = (const uint16_t *)((const void *)bitmap);

#if (GLCD_SWAP_XY == 0)
  y = (y + Scroll) % GLCD_HEIGHT;
#endif

  GLCD_SetWindow(x, y, width, height);

  wr_cmd(0x22);
  wr_dat_start();
  for (i = (int32_t)((height-1)*width); i > -1; i -= width) {
    for (j = 0; j < (int32_t)width; j++) {
      wr_dat_only(ptr_bmp[i+j]);
    }
  }
  wr_dat_stop();

  return 0;
}

/**
  \fn          int32_t GLCD_VScroll (uint32_t dy)
  \brief       Scroll picture on display vertically
  \param[in]   dy  Scroll size in pixels
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t GLCD_VScroll (uint32_t dy) {
#if (GLCD_SWAP_XY == 0)
  uint32_t i;

  Scroll = (Scroll + dy) % GLCD_HEIGHT;

//  if (Himax) {
    wr_reg(0x01, 0x08);
    wr_reg(0x14, Scroll >> 8);          /* VSP MSB                            */
    wr_reg(0x15, Scroll & 0xFF);        /* VSP LSB                            */
//  }

  /* Clear new space */
  GLCD_SetWindow(0, Scroll, GLCD_WIDTH, dy);

  wr_cmd(0x22);
  wr_dat_start();
  for(i = 0; i < (GLCD_WIDTH * dy); i++) {
    wr_dat_only(background_color);
  }
  wr_dat_stop();
  return 0;
#else
  UNUSED(dy);
  return -1;
#endif
}

/**
  \fn          int32_t GLCD_FrameBufferAccess (bool enable)
  \brief       Enable or disable direct access to FrameBuffer
  \param[in]   enable Enable or disable request
                - true (1): enable
                - false(0): disable
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
int32_t  GLCD_FrameBufferAccess (bool enable) {
  UNUSED(enable);

  GLCD_SetWindow(0, 0, GLCD_WIDTH, GLCD_HEIGHT);

  wr_reg (0x20, 0x0000);
  wr_reg (0x21, 0x0000);
  wr_cmd (0x22);

  return 0;
}

/**
  \fn          uint32_t GLCD_FrameBufferAddress (void)
  \brief       Retrieve FrameBuffer address
  \returns     FrameBuffer address
*/
uint32_t GLCD_FrameBufferAddress (void) {
  return NULL;   /* not available with himax */
}
