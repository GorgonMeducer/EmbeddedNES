/*
 * Copyright (c) 2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*-----------------------------------------------------------------------------
 * File     system_Yamin.c
 * Brief    CMSIS Device System File for
 *          Yamin device (AN540)
 * Version  1.0.0
 *----------------------------------------------------------------------------*/

#if defined (YAMIN)
  #include "Yamin.h"

  #if defined (__ARM_FEATURE_CMSE) &&  (__ARM_FEATURE_CMSE == 3U)
    #include "partition_Yamin.h"
  #endif
#else
  #error device not specified!
#endif


/*----------------------------------------------------------------------------
  Externals
 *----------------------------------------------------------------------------*/
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  extern uint32_t __VECTOR_TABLE;
#endif

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;


/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
  SystemCoreClock = SYSTEM_CLOCK;
}

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
void SystemInit (void)
{

#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  SCB->VTOR = (uint32_t)(&__VECTOR_TABLE);
#endif

#if (defined (__FPU_USED) && (__FPU_USED == 1U)) || \
    (defined (__MVE_USED) && (__MVE_USED == 1U))
  SCB->CPACR |= ((3U << 10U*2U) |           /* enable CP10 Full Access */
                 (3U << 11U*2U)  );         /* enable CP11 Full Access */
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
  SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

#if defined (__ARM_FEATURE_CMSE) && (__ARM_FEATURE_CMSE == 3U)
  /* Enable BusFault, UsageFault, MemManageFault and SecureFault to ease diagnostic */
  SCB->SHCSR |= (SCB_SHCSR_USGFAULTENA_Msk  |
                 SCB_SHCSR_BUSFAULTENA_Msk  |
                 SCB_SHCSR_MEMFAULTENA_Msk  |
                 SCB_SHCSR_SECUREFAULTENA_Msk);

  /* BFSR register setting to enable precise errors */
  SCB->CFSR |= SCB_CFSR_PRECISERR_Msk;

  /* configure unsecure code area: ITCM 512K 0x00080000 - 0x00100000 */
  // blk_cfg = ITGU->CFG & 0xF;             /* = 0x7 */
  // blk_size = 1UL << (blk_cfg + 5U);      /* = 0x1000 (4K) */ 
  ITGU->LUT[4] = 0xFFFFFFFF;
  ITGU->LUT[5] = 0xFFFFFFFF;
  ITGU->LUT[6] = 0xFFFFFFFF;
  ITGU->LUT[7] = 0xFFFFFFFF;

  /* configure unsecure data area: DTCM 512K 0x20080000 - 0x20100000 */
  // blk_cfg = DTGU->CFG & 0xF;             /* = 0x7 */
  // blk_size = 1UL << (blk_cfg + 5U);      /* = 0x1000 (4K) */ 
  DTGU->LUT[4] = 0xFFFFFFFF;
  DTGU->LUT[5] = 0xFFFFFFFF;
  DTGU->LUT[6] = 0xFFFFFFFF;
  DTGU->LUT[7] = 0xFFFFFFFF;

  /* Non Secure Callable Configuration for IDAU (NSCCFG register) */
  SPCTRL->NSCCFG |= 1U; /* Configure CODE region (0x1000_0000 to 0x1FFF_FFFF) Non-secure Callable */
//SPCTRL->NSCCFG |= 2U; /* Configure RAM  region (0x3000_0000 to 0x3FFF_FFFF) Non-secure Callable */


  /* configure PPC --------------- */
#if !defined (__USE_SECURE)
  /* Allow Non-secure access for some registers */
  SPCTRL->APBNSPPCEXP[2U] |= ((1U <<  0) |         /* FPGA - SCC register */
                              (1U <<  2) |         /* FPGA - IO register */
                              (1U <<  3) |         /* USART 0 register */
                              (1U << 10)  );       /* CLCD register */
#endif

  TZ_SAU_Setup();
#endif

  SystemCoreClock = SYSTEM_CLOCK;
}
