/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    Blinky.c
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2015 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "Device.h"                     // Keil::Board Support:V2M-MPS2:Common
#include "Board_LED.h"                  // ::Board Support:LED
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.V2M-MPS2::Board Support:Graphic LCD

extern GLCD_FONT     GLCD_Font_16x24;

osThreadId tid_phaseA;                  /* Thread id of thread: phase_a      */
osThreadId tid_phaseB;                  /* Thread id of thread: phase_b      */
osThreadId tid_phaseC;                  /* Thread id of thread: phase_c      */
osThreadId tid_phaseD;                  /* Thread id of thread: phase_d      */
osThreadId tid_clock;                   /* Thread id of thread: clock        */
osThreadId tid_lcd;                     /* Thread id of thread: lcd          */

osMutexId mut_GLCD;                     /* Mutex to controll GLCD access     */

#define LED_A      7
#define LED_B      6
#define LED_C      5
#define LED_D      4
#define LED_CLK    0


/*----------------------------------------------------------------------------
 *      Switch LED on
 *---------------------------------------------------------------------------*/
void Switch_On (uint32_t ledNum) {

  LED_On (ledNum);
  osMutexWait(mut_GLCD, osWaitForever);
#ifdef __USE_LCD
  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  GLCD_SetForegroundColor(GLCD_COLOR_GREEN);
  GLCD_DrawChar((9+ledNum)*16, 6*24, 0x80+1);     /* Circle Full               */
#endif
  osMutexRelease(mut_GLCD);
}

/*----------------------------------------------------------------------------
 *      Switch LED off
 *---------------------------------------------------------------------------*/
void Switch_Off (uint32_t ledNum) {

  LED_Off(ledNum);
  osMutexWait(mut_GLCD, osWaitForever);
#ifdef __USE_LCD
  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  GLCD_SetForegroundColor(GLCD_COLOR_LIGHT_GREY);
  GLCD_DrawChar((9+ledNum)*16, 6*24, 0x80+0);     /* Circle Full               */
#endif
  osMutexRelease(mut_GLCD);
}


/*----------------------------------------------------------------------------
 *      Function 'signal_func' called from multiple threads
 *---------------------------------------------------------------------------*/
void signal_func (osThreadId tid)  {
  osSignalSet(tid_clock, 0x0100);           /* set signal to clock thread    */
  osDelay(500);                             /* delay 500ms                   */
  osSignalSet(tid_clock, 0x0100);           /* set signal to clock thread    */
  osDelay(500);                             /* delay 500ms                   */
  osSignalSet(tid, 0x0001);                 /* set signal to thread 'thread' */
  osDelay(500);                             /* delay 500ms                   */
}

/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
void phaseA (void const *argument) {
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_A);
    signal_func(tid_phaseB);                /* call common signal function   */
    Switch_Off(LED_A);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
void phaseB (void const *argument) {
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_B);
    signal_func(tid_phaseC);                /* call common signal function   */
    Switch_Off(LED_B);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 3 'phaseC': Phase C output
 *---------------------------------------------------------------------------*/
void phaseC (void const *argument) {
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_C);
    signal_func(tid_phaseD);                /* call common signal function   */
    Switch_Off(LED_C);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 4 'phaseD': Phase D output
 *---------------------------------------------------------------------------*/
void phaseD (void const *argument) {
  for (;;) {
    osSignalWait(0x0001, osWaitForever);    /* wait for an event flag 0x0001 */
    Switch_On (LED_D);
    signal_func(tid_phaseA);                /* call common signal function   */
    Switch_Off(LED_D);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
void clock (void const *argument) {
  for (;;) {
    osSignalWait(0x0100, osWaitForever);    /* wait for an event flag 0x0100 */
    Switch_On (LED_CLK);
    osDelay(80);                            /* delay 80ms                    */
    Switch_Off(LED_CLK);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 6 'lcd': LCD Control thread
 *---------------------------------------------------------------------------*/
void lcd (void const *argument) {

  for (;;) {
    osMutexWait(mut_GLCD, osWaitForever);
#ifdef __USE_LCD
  GLCD_SetBackgroundColor(GLCD_COLOR_BLUE);
  GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
  GLCD_DrawString(0*16, 0*24, "   V2M-MPS2 Demo    ");
  GLCD_DrawString(0*16, 1*24, "      Blinky        ");
  GLCD_DrawString(0*16, 2*24, "   www.keil.com     ");
#endif
    osMutexRelease(mut_GLCD);
    osDelay(4000);

    osMutexWait(mut_GLCD, osWaitForever);
#ifdef __USE_LCD
  GLCD_SetBackgroundColor(GLCD_COLOR_BLUE);
  GLCD_SetForegroundColor(GLCD_COLOR_RED);
  GLCD_DrawString(0*16, 0*24, "   V2M-MPS2 Demo    ");
  GLCD_DrawString(0*16, 1*24, "      Blinky        ");
  GLCD_DrawString(0*16, 2*24, "   www.keil.com     ");
#endif
    osMutexRelease(mut_GLCD);
    osDelay(4000);
  }
}


osMutexDef(mut_GLCD);

osThreadDef(phaseA, osPriorityNormal, 1, 0);
osThreadDef(phaseB, osPriorityNormal, 1, 0);
osThreadDef(phaseC, osPriorityNormal, 1, 0);
osThreadDef(phaseD, osPriorityNormal, 1, 0);
osThreadDef(clock,  osPriorityNormal, 1, 0);
osThreadDef(lcd,    osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
int main (void) {

  SystemCoreClockUpdate();
  LED_Initialize();                         /* Initialize the LEDs           */

#ifdef __USE_LCD
  GLCD_Initialize();                        /* Initialize the GLCD           */

  GLCD_SetFont(&GLCD_Font_16x24);
  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  GLCD_ClearScreen();

  GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
  GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
  switch ((SCB->CPUID >> 4) & 0xFFF) {
    case 0xC20:
      GLCD_DrawString(0*16, 4*24, "   ARM Cortex-M0    ");
      break;
    case 0xC60:
      GLCD_DrawString(0*16, 4*24, "   ARM Cortex-M0+   ");
      break;
    case 0xC21:
      GLCD_DrawString(0*16, 4*24, "   ARM Cortex-M1    ");
      break;
    case 0xC23:
      GLCD_DrawString(0*16, 4*24, "   ARM Cortex-M3    ");
      break;
    case 0xC24:
      GLCD_DrawString(0*16, 4*24, "   ARM Cortex-M4    ");
      break;
    case 0xC27:
      GLCD_DrawString(0*16, 4*24, "   ARM Cortex-M7    ");
      break;
    default:
      GLCD_DrawString(0*16, 4*24, "  unknown Cortex-M  ");
      break;
  }
#endif

  mut_GLCD = osMutexCreate(osMutex(mut_GLCD));

  tid_phaseA = osThreadCreate(osThread(phaseA), NULL);
  tid_phaseB = osThreadCreate(osThread(phaseB), NULL);
  tid_phaseC = osThreadCreate(osThread(phaseC), NULL);
  tid_phaseD = osThreadCreate(osThread(phaseD), NULL);
  tid_clock  = osThreadCreate(osThread(clock),  NULL);
  tid_lcd    = osThreadCreate(osThread(lcd),    NULL);

  osSignalSet(tid_phaseA, 0x0001);          /* set signal to phaseA thread   */

  for (;;) {                                /* main must not be terminated!  */
    osDelay(1000);
  }
}
