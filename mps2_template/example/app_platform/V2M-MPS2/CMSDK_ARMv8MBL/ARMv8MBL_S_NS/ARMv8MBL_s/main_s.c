/*----------------------------------------------------------------------------
 * Name:    main_s.c
 * Purpose: Main function secure mode
 *----------------------------------------------------------------------------*/

#include <arm_cmse.h>
#include "CMSDK_ARMv8MBL.h"                      /* Device header */
#include "Board_LED.h"                           /* ::Board Support:LED */

/* Start address of non-secure application */
#define NONSECURE_START (0x00200000u)

/* typedef for NonSecure callback functions */
typedef int32_t (*NonSecure_fpParam)(uint32_t) __attribute__((cmse_nonsecure_call));
typedef void    (*NonSecure_fpVoid) (void)     __attribute__((cmse_nonsecure_call));


/*----------------------------------------------------------------------------
  NonSecure callback functions
 *----------------------------------------------------------------------------*/
extern NonSecure_fpParam pfNonSecure_LED_On;
       NonSecure_fpParam pfNonSecure_LED_On  = (NonSecure_fpParam)NULL;
extern NonSecure_fpParam pfNonSecure_LED_Off;
       NonSecure_fpParam pfNonSecure_LED_Off = (NonSecure_fpParam)NULL;


/*----------------------------------------------------------------------------
  Secure functions exported to NonSecure application
 *----------------------------------------------------------------------------*/
int32_t Secure_LED_On (uint32_t num) __attribute__((cmse_nonsecure_entry));
int32_t Secure_LED_On (uint32_t num)
{
  return LED_On(num);
}

int32_t Secure_LED_Off (uint32_t num) __attribute__((cmse_nonsecure_entry)) ;
int32_t Secure_LED_Off (uint32_t num)
{
  return LED_Off(num);
}


/*----------------------------------------------------------------------------
  Secure function for NonSecure callbacks exported to NonSecure application
 *----------------------------------------------------------------------------*/
int32_t Secure_LED_On_callback(NonSecure_fpParam callback) __attribute__((cmse_nonsecure_entry));
int32_t Secure_LED_On_callback(NonSecure_fpParam callback)
{
  pfNonSecure_LED_On = callback;
  return 0;
}

int32_t Secure_LED_Off_callback(NonSecure_fpParam callback) __attribute__((cmse_nonsecure_entry));
int32_t Secure_LED_Off_callback(NonSecure_fpParam callback)
{
  pfNonSecure_LED_Off = callback;
  return 0;
}


/*----------------------------------------------------------------------------
  SysTick IRQ Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void);
void SysTick_Handler (void) {
  static uint32_t ticks;

  switch (ticks++) {
    case   0:
      LED_On (0u);
      break;
    case 200:
      LED_Off(0u);
      break;
    case 300:
      if (pfNonSecure_LED_On != NULL)
      {
        pfNonSecure_LED_On(1u);
      }
      break;
    case 500:
      if (pfNonSecure_LED_Off != NULL)
      {
        pfNonSecure_LED_Off(1u);
      }
      break;
    case 600:
      ticks = 0;
      break;

    default:
      if (ticks > 600) {
        ticks = 0;
      }
  }
}


static uint32_t x;
/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/
int main (void)
{
  uint32_t         NonSecure_StackPointer;
  NonSecure_fpVoid NonSecure_ResetHandler;

  NonSecure_StackPointer =                   (*((uint32_t *)(NONSECURE_START + 0u)));
  NonSecure_ResetHandler = (NonSecure_fpVoid)(*((uint32_t *)(NONSECURE_START + 4u)));

  /* exercise some core register from Secure Mode */
  x = __get_MSP();
  x = __get_PSP();
  __TZ_set_MSP_NS(NonSecure_StackPointer);
  x = __TZ_get_MSP_NS();
  __TZ_set_PSP_NS(0x22000000u);
  x = __TZ_get_PSP_NS();

  SystemCoreClockUpdate();

  LED_Initialize ();

  SysTick_Config(SystemCoreClock / 100);  /* Generate interrupt each 10 ms */

  NonSecure_ResetHandler();
}

