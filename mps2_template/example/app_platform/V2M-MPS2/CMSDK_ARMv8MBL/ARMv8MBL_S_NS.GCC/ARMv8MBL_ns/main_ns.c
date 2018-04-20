/*----------------------------------------------------------------------------
 * Name:    main_ns.c
 * Purpose: Main function non-secure mode
 *----------------------------------------------------------------------------*/

#include <arm_cmse.h>
#include "CMSDK_ARMv8MBL.h"                        /* Device header */
#include "Board_LED.h"                             /* ::Board Support:LED */
#include "..\ARMv8MBL_s\Secure_Functions.h"        /* Secure Code Entry Points */


/*----------------------------------------------------------------------------
  NonSecure functions used for callbacks
 *----------------------------------------------------------------------------*/
int32_t NonSecure_LED_On(uint32_t num);
int32_t NonSecure_LED_On(uint32_t num)
{
  return LED_On(num);
}

int32_t NonSecure_LED_Off(uint32_t num);
int32_t NonSecure_LED_Off(uint32_t num)
{
  return LED_Off(num);
}


/*----------------------------------------------------------------------------
  SysTick IRQ Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void);
void SysTick_Handler (void) {
  static uint32_t ticks;

  switch (ticks++) {
    case   0:
      LED_On(7u);
      break;
    case 200:
      Secure_LED_On(6u);
      break;
    case 300:
      LED_Off(7u);
      break;
    case 500:
      Secure_LED_Off(6u);
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
  uint32_t i;

  /* exercise some core register from Non Secure Mode */
  x = __get_MSP();
  x = __get_PSP();

  /* register NonSecure callbacks in Secure application */
  Secure_LED_On_callback(NonSecure_LED_On);
  Secure_LED_Off_callback(NonSecure_LED_Off);

#if 0
  LED_Initialize ();                    /* already done in S part */
#endif

  SystemCoreClockUpdate();
  SysTick_Config(SystemCoreClock/100);  /* Generate interrupt each 10 ms */

  while (1) {
    LED_On (5u);
    for (i = 0; i < 0x100000; i++) __NOP();
    LED_Off(5u);
    for (i = 0; i < 0x100000; i++) __NOP();
    Secure_LED_On (4u);
    for (i = 0; i < 0x100000; i++) __NOP();
    Secure_LED_Off(4u);
    for (i = 0; i < 0x100000; i++) __NOP();
  }
}
