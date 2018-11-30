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


#ifndef __USE_ARM_COMPILER_H__
#define __USE_ARM_COMPILER_H__

/*============================ INCLUDES ======================================*/
#include <cmsis_compiler.h>

#if __IS_COMPILER_IAR__
#   include <intrinsics.h>
#endif

/*============================ MACROS ========================================*/

#ifndef DEF_REG
#define DEF_REG                     \
        union  {                    \
            struct {
#endif
    
#ifndef END_DEF_REG
#define END_DEF_REG(__NAME)         \
            };                      \
            reg32_t Value;          \
        }__NAME;
#endif

#ifndef __REG_MACRO__
#define __REG_MACRO__
#endif


#ifndef REG_RSVD_0x10
#define REG_RSVD_0x10                   \
    reg32_t                     : 32;   \
    reg32_t                     : 32;   \
    reg32_t                     : 32;   \
    reg32_t                     : 32;   
#endif
#ifndef REG_RSVD_0x80       
#define REG_RSVD_0x80                   \
            REG_RSVD_0x10               \
            REG_RSVD_0x10               \
            REG_RSVD_0x10               \
            REG_RSVD_0x10               \
            REG_RSVD_0x10               \
            REG_RSVD_0x10               \
            REG_RSVD_0x10               \
            REG_RSVD_0x10
#endif

#ifndef REG_RSVD_0x100                 
#define REG_RSVD_0x100                  \
            REG_RSVD_0x80               \
            REG_RSVD_0x80
#endif

#ifndef REG_RSVD_0x800
#define REG_RSVD_0x800                  \
            REG_RSVD_0x100              \
            REG_RSVD_0x100              \
            REG_RSVD_0x100              \
            REG_RSVD_0x100              \
            REG_RSVD_0x100              \
            REG_RSVD_0x100              \
            REG_RSVD_0x100              \
            REG_RSVD_0x100
#endif


//! \brief The mcu memory align mode
# define MCU_MEM_ALIGN_SIZE             sizeof(int)

#ifndef __volatile__
#define __volatile__                    volatile
#endif

//! \brief 1 cycle nop operation
#ifndef NOP
    #define NOP()                       __asm__ __volatile__ ("nop");
#endif


//! \brief none standard memory types
#if __IS_COMPILER_IAR__
#   define ROM_FLASH            _Pragma(__STR(location=".rom.flash")) const
#   define ROM_EEPROM           _Pragma(__STR(location=".rom.eeprom")) const
#   define NO_INIT              __no_init
#   define ROOT                 __root
#   define INLINE               inline
#   define NO_INLINE            noinline
#   define ALWAYS_INLINE        __attribute__((always_inline))
#   define WEAK                 __weak
#   define RAMFUNC              __ramfunc
#   define __asm__              __asm
#   define __ALIGN(__N)         _Pragma(__STR(data_alignment=__N))
#   define __AT_ADDR(__ADDR)    @ __ADDR
#   define __SECTION(__SEC)     _Pragma(__STR(location=__SEC))
#   define __WEAK_ALIAS(__ORIGIN, __ALIAS) \
                                _Pragma(__STR(weak __ORIGIN=__ALIAS))
#   define PACKED               __packed
#   define UNALIGNED            __packed
#   define TRANSPARENT_UNION    __attribute__((transparent_union))

#elif __IS_COMPILER_ARM_COMPILER_5__
#   define ROM_FLASH            __attribute__(( section( ".rom.flash"))) const
#   define ROM_EEPROM           __attribute__(( section( ".rom.eeprom"))) const
#   define NO_INIT              __attribute__( ( section( ".bss.noinit"),zero_init) )
#   define ROOT                 __attribute__((used))    
#   define INLINE               __inline
#   define NO_INLINE            __attribute__((noinline))
#   define ALWAYS_INLINE        __attribute__((always_inline))
#   define WEAK                 __attribute__((weak))
#   define RAMFUNC              __attribute__((section (".textrw")))
#   define __asm__              __asm
#   define __ALIGN(__N)         __attribute__((aligned (__N))) 
#   define __AT_ADDR(__ADDR)    __attribute__((at(__ADDR)))
#   define __SECTION(__SEC)     __attribute__((section (__SEC)))
#   define __WEAK_ALIAS(__ORIGIN, __ALIAS) \
                                __attribute__((weakref(__STR(__ALIAS))))
                                
#   define PACKED               __packed
#   define UNALIGNED            __packed
#   define TRANSPARENT_UNION    __attribute__((transparent_union))

#elif __IS_COMPILER_ARM_COMPILER_6__
#   define ROM_FLASH            __attribute__(( section( ".rom.flash"))) const
#   define ROM_EEPROM           __attribute__(( section( ".rom.eeprom"))) const
#   define NO_INIT              __attribute__( ( section( ".bss.noinit")) )
#   define ROOT                 __attribute__((used))    
#   define INLINE               __inline
#   define NO_INLINE            __attribute__((noinline))
#   define ALWAYS_INLINE        __attribute__((always_inline))
#   define WEAK                 __attribute__((weak))
#   define RAMFUNC              __attribute__((section (".textrw")))
#   define __asm__              __asm
#   define __ALIGN(__N)         __attribute__((aligned (__N))) 
#   define __AT_ADDR(__ADDR)    __attribute__((section (".ARM.__at_" #__ADDR)))
#   define __SECTION(__SEC)     __attribute__((section (__SEC)))
#   define __WEAK_ALIAS(__ORIGIN, __ALIAS) \
                                __attribute__((weakref(__STR(__ALIAS))))

#   define PACKED               __attribute__((packed))
#   define UNALIGNED            __unaligned
#   define TRANSPARENT_UNION    __attribute__((transparent_union))

#else  /*__IS_COMPILER_GCC__: Using GCC as default for those GCC compliant compilers*/
#   define ROM_FLASH            __attribute__(( section( ".rom.flash"))) const
#   define ROM_EEPROM           __attribute__(( section( ".rom.eeprom"))) const
#   define NO_INIT              __attribute__(( section( ".bss.noinit")))
#   define ROOT                 __attribute__((used))    
#   define INLINE               inline
#   define NO_INLINE            __attribute__((noinline))
#   define ALWAYS_INLINE        __attribute__((always_inline))
#   define WEAK                 __attribute__((weak))
#   define RAMFUNC              __attribute__((section (".textrw")))
#   define __asm__              __asm
#   define __ALIGN(__N)         __attribute__((aligned (__N)))
#   define __AT_ADDR(__ADDR)    __attribute__((at(__ADDR))) 
#   define __SECTION(__SEC)     __attribute__((section (__SEC)))
#   define __WEAK_ALIAS(__ORIGIN, __ALIAS) \
                                __attribute__((weakref(__STR(__ALIAS))))

#   define PACKED               __attribute__((packed))
#   define UNALIGNED            __attribute__((packed))
#   define TRANSPARENT_UNION    __attribute__((transparent_union))

#endif

#define WEAK_ALIAS(__ORIGIN, __ALIAS)   \
                            __WEAK_ALIAS(__ORIGIN, __ALIAS)
#define AT_ADDR(__ADDR)     __AT_ADDR(__ADDR)
#define ALIGN(__N)          __ALIGN(__N)
#define SECTION(__SEC)      __SECTION(__SEC)

/*----------------------------------------------------------------------------*
 * Signal & Interrupt Definition                                              *
 *----------------------------------------------------------------------------*/

  /*!< Macro to enable all interrupts. */
#if __IS_COMPILER_IAR__
#   define ENABLE_GLOBAL_INTERRUPT()            __enable_interrupt()
#else
#   define ENABLE_GLOBAL_INTERRUPT()            __enable_irq()
#endif

  /*!< Macro to disable all interrupts. */
#if __IS_COMPILER_IAR__
#   define DISABLE_GLOBAL_INTERRUPT()           ____disable_irq()

static ALWAYS_INLINE uint32_t ____disable_irq(void) 
{
    uint32_t wPRIMASK = __get_interrupt_state();
    __disable_irq();
    return wPRIMASK & 0x1;
}

#elif __IS_COMPILER_ARM_COMPILER_5__
#   define DISABLE_GLOBAL_INTERRUPT()           __disable_irq()
#elif __IS_COMPILER_ARM_COMPILER_6__
#   define DISABLE_GLOBAL_INTERRUPT()           __disable_irq()
#elif __IS_COMPILER_GCC_
#   define DISABLE_GLOBAL_INTERRUPT()           __disable_irq()
#else /* for other compilers, using gcc assembly syntax to implement */

#   define DISABLE_GLOBAL_INTERRUPT()           ____disable_irq()

static ALWAYS_INLINE uint32_t ____disable_irq(void) 
{
    uint32_t cpsr;

    __asm__ __volatile__("mrs %[cpsr], primask\n"
                        "cpsid i\n"
                        : [cpsr] "=r"(cpsr));
    return cpsr & 0x1;
}
#endif

#if __IS_COMPILER_IAR__
#   define GET_GLOBAL_INTERRUPT_STATE()         __get_interrupt_state()
#   define SET_GLOBAL_INTERRUPT_STATE(__STATE)  __set_interrupt_state(__STATE)
typedef __istate_t   istate_t;
#elif __IS_COMPILER_ARM_COMPILER_5__ || __IS_COMPILER_ARM_COMPILER_6__
#   define GET_GLOBAL_INTERRUPT_STATE()         __get_PRIMASK()
#   define SET_GLOBAL_INTERRUPT_STATE(__STATE)  __set_PRIMASK(__STATE)
typedef int   istate_t;
#elif __IS_COMPILER_GCC__
#   define GET_GLOBAL_INTERRUPT_STATE()         __get_PRIMASK()
#   define SET_GLOBAL_INTERRUPT_STATE(__STATE)  __set_PRIMASK(__STATE)
typedef uint32_t   istate_t;
#else
typedef uint32_t   istate_t;
#   define GET_GLOBAL_INTERRUPT_STATE()         ____get_PRIMASK()

/**
  \brief   Get Priority Mask
  \details Returns the current state of the priority mask bit from the Priority Mask Register.
  \return               Priority Mask value
 */
__attribute__((always_inline)) static inline uint32_t ____get_PRIMASK(void)
{
    unsigned int result;

    __asm__ volatile ("MRS %0, primask" : "=r" (result) );
    return(result);
}

#   define SET_GLOBAL_INTERRUPT_STATE(__STATE)  ____set_PRIMASK(__STATE)

/**
  \brief   Set Priority Mask
  \details Assigns the given value to the Priority Mask Register.
  \param [in]    priMask  Priority Mask
 */
__attribute__((always_inline)) static inline void ____set_PRIMASK(uint32_t priMask)
{
    __asm__ volatile ("MSR primask, %0" : : "r" (priMask) : "memory");
}
#endif

/*============================ TYPES =========================================*/
/*============================ INCLUDES ======================================*/

/*----------------------------------------------------------------------------*
 * Device Dependent Compiler Files                                            *
 *----------------------------------------------------------------------------*/
#if     defined(__CORTEX_M0__)
#include "cortex_m0_compiler.h"
#elif   defined(__CORTEX_M0P__)
#include "cortex_m0p_compiler.h"
#elif   defined(__CORTEX_M1__)
#include "cortex_m1_compiler.h"
#elif   defined(__CORTEX_M3__)
#include "cortex_m3_compiler.h"
#elif   defined(__CORTEX_M4__)
#include "cortex_m4_compiler.h"
#elif   defined(__CORTEX_M7__)
#include "cortex_m7_compiler.h"
#elif   defined(__CORTEX_M23__)
#include "cortex_m23_compiler.h"
#elif   defined(__CORTEX_M33__)
#include "cortex_m33_compiler.h"
#else

//! \brief The mcu memory endian mode
# define __BIG_ENDIAN__                 false

/*ARM Cortex M4 implementation for interrupt priority shift*/
# define ARM_INTERRUPT_LEVEL_BITS       4

#endif

/*!  \note using the ANSI-C99 standard type,if the file stdint.h dose not exit
 *!        you should define it all by yourself.
 *!
 */
#include ".\app_type.h"

//! \brief for interrupt 
#include ".\signal.h"

#endif
