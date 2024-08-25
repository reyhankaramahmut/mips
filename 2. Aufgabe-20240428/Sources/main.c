#include <msp430.h> 
#include "..\base.h"
#include "Handler.h"
#include "event.h"
#include "TA0.h"
#include "TA1.h"
#include "UCA1.h"

GLOBAL Int _system_pre_init(Void) {
   // stop watchdog timer
   WDTCTL = WDTPW + WDTHOLD;
   return 0;
}

// DCO mit 8.0 MHz
// XT1 mit 4.9152 MHz
// ACLK  := XT1 / 8  = 614.4 kHz
// SMCLK := DCO / 32 = 250.0 kHz
// MCLK  := DCO / 1  = 8.0 MHz
#pragma FUNC_ALWAYS_INLINE(CS_init)
LOCAL Void CS_init(Void) {
   CSCTL0 = CSKEY;         // enable clock system
   CSCTL1 = DCOFSEL_3;     // DCO frequency = 8.0 MHz
                           // select clock sources
   CSCTL2 = SELA__XT1CLK   // ACLK  <- XT1
          | SELS__DCOCLK   // SMCLK <- DCO
          | SELM__DCOCLK;  // MCLK  <- DCO
                           // set frequency divider
   CSCTL3 = DIVA__8        // ACLK  : /8  = 614.4 kHz
          | DIVS__16       // SMCLK : /16 = 500.0 kHz
          | DIVM__1;       // MCLK  : /1  =   8.0 MHz

   CSCTL4 = XT2OFF         // XT2 disabled
          | XTS            // XT1 HF mode
          | XT1DRIVE_0;    // XT1 low power, no bypass

   CSCTL0_H = 0;           // disable clock system
}

#define VAL_16BIT(arg1, arg2) ((unsigned)(((arg1) << 8) | (arg2)))

#pragma FUNC_ALWAYS_INLINE(GPIO_init)
LOCAL Void GPIO_init(Void) {
   // Port 1: Pin 0 => input,  BTN1
   // Port 1: Pin 1 => input,  BTN2
   // Port 1: Pin 2 => output, LED2, idle Low
   // Port 2: Pin 7 => output, LED1, idle Low
   // Port 2: Pin 3 => output, SPI.CS, idle High
   // Port 2: Pin 4, 5 and 6 => SPI
   // Port 3: Pin 4 => output, TEST
   // Port 3: Pin 5 => output, TEST

   //                   Port2       Port1
   //               Bit 76543210    76543210
   PAOUT  = VAL_16BIT(0b00000000, 0b00000000); // clear all outputs
   PADIR  = VAL_16BIT(0b10001000, 0b00000100); // direction, set outputs
   PAIFG  = VAL_16BIT(0b00000000, 0b00000000); // clear all interrupt flags
   PAIE   = VAL_16BIT(0b00000000, 0b00000000); // disable all GPIO interrupts
   PASEL0 = VAL_16BIT(0b00000000, 0b00000000);
   PASEL1 = VAL_16BIT(0b01110000, 0b00000000);
   PAREN  = VAL_16BIT(0b00000000, 0b00000000); // without pull up

   //                   Port4       Port3
   //               Bit 76543210    76543210
   PBOUT  = VAL_16BIT(0b00000000, 0b00000000); // clear all outputs
   PBDIR  = VAL_16BIT(0b00000000, 0b00110000); // direction, set outputs
   PBIFG  = VAL_16BIT(0b00000000, 0b00000000); // clear all interrupt flags
   PBIE   = VAL_16BIT(0b00000000, 0b00000000); // disable all GPIO interrupts
   PBSEL0 = VAL_16BIT(0b00000000, 0b00000000);
   PBSEL1 = VAL_16BIT(0b00000000, 0b00000000);
   PBREN  = VAL_16BIT(0b00000000, 0b00000000); // without pull up

}

GLOBAL Void main(Void) {
   Int cnt = MUSTER1;

   CS_init();     // set up Clock System
   GPIO_init();   // set up Ports
   Event_init();
   TA0_init();    // set up Timer A0
   TA1_init();    // set up Timer A1
   UCA1_init();   // set up SPI
   Handler_init();

   while(TRUE) {
      Event_wait();
      if (Event_tst(EVENT_BTN2)) {
         Event_clr(EVENT_BTN2);
         if (++cnt GT MUSTER6) {
            cnt = MUSTER1;
         }
         set_blink_muster(cnt);
      }
      if (Event_tst(EVENT_BTN1)) {
         Event_clr(EVENT_BTN1);
         TGLBIT(P2OUT, BIT7);
      }

      // wenn die drei Handler korrekt implementiert sind,
      // kann man ihre Reihnefolge hier beliebig �ndern
      Button_Handler();
      Number_Handler();
      AS1108_Handler();

      if (Event_err()) {
         SETBIT(P1OUT, BIT2); // LED on
      }
   }
}

