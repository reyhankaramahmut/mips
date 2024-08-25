/*
 * UCA0.c
 *
 *  Created on: Jun 25, 2024
 *      Author: Robin
 */

#include <msp430.h>
#include "..\base.h"
#include "event.h"
#include "uca0.h"
#include "stdio.h"

LOCAL const Char *ptr;
LOCAL UInt i;
LOCAL Char ch;
GLOBAL Char rx_buf[DIGISIZE + 1];

#pragma FUNC_ALWAYS_INLINE(UCA0_init)
GLOBAL Void UCA0_init(Void)
{
    SETBIT(UCA0CTLW0, UCSWRST); // UCA0 software reset

    UCA0CTLW1 = 0x0002; // deglitch time approximately 100 ns
    UCA0BRW = 2;        // set clock prescaler for 14400 baud
    UCA0MCTLW = (0xD6 << 8) | (0x0A << 4) | UCOS16; // second modulation stage, first modulation stage, and enable 16 times oversampling

    UCA0CTLW0 = UCPEN    // enable parity
                | UCPAR  // even parity
                | 0      // LSB first
                | 0      // 8-bit-data
                | 0      // one stop bit
                | UCMODE_0  // UART mode
                | 0      // Asynchronous mode
                | UCSSEL__ACLK // select clock source: ACLK
                | UCRXEIE      // error interrupt enable
                | UCBRKIE;     // break interrupt enable

    CLRBIT(UCA0CTLW0, UCSWRST); // release the UCA0 for operation

    UCA0IE = UCRXIE; // Only enable Receive Interrupt

    ch = '\0';
}


#pragma vector = USCI_A0_VECTOR
__interrupt Void UCA0_ISR(Void)
{
    switch (__even_in_range(UCA0IV, 0x04))
    {
    case 0x02: // Vector 2: Receive buffer full
        //ch = UCA0RXBUF;

        if (TSTBIT(UCA0STATW, UCBRK ))
        {
            set_error(BREAK_ERROR); // Set Break error
            Char dummy = UCA0RXBUF;  // Dummy read to clear the buffer
            return;
        }

        if (TSTBIT(UCA0STATW, UCRXERR))     // -------------------- receive error
        {
           Char dummy = UCA0RXBUF; // dummy read
           set_error(FROVPAR_ERROR);
           return;
        }


        ch = UCA0RXBUF; // Read character

        if (between('0', ch, '9'))
        {
            if (i < DIGISIZE)
            {
                rx_buf[i++] = ch;
            }
            else
            {
                i = 0;
                set_error(BUFFER_ERROR);
            }
        }
        else if (ch EQ '\r')
        {
            if (i == DIGISIZE)
            {
                rx_buf[i] = '\0';
                i = 0;
                Event_set(EVENT_RXD);
                set_error(NO_ERROR);
            }
            else
            {
                i = 0;
                set_error(BUFFER_ERROR);
            }
        }
        else
        {
            i = 0;
            set_error(CHARACTOR_ERROR);
        }
        break;

    case 0x04: // Vector 4: Transmit buffer empty
        Event_clr(EVENT_PRINT);


        if (TSTBIT(UCA0STATW, UCBRK))
        {
            Char dummy = UCA0RXBUF;  // Dummy read to clear the buffer
            set_error(BREAK_ERROR);  // Set Break error
            return;
        }

        if (TSTBIT(UCA0STATW, UCRXERR))   // -------------------- receive error
        {
           Char dummy = UCA0RXBUF; // dummy read
           set_error(FROVPAR_ERROR);
           return;
        }

        if (*ptr NE '\0')
        {
            UCA0TXBUF = *ptr++;
        }
        else
        {
            CLRBIT(UCA0IE, UCTXIE); // Transmit interrupt disable
            Char dummy = UCA0RXBUF; // Dummy read to clear the buffer
            set_error(NO_ERROR);
            SETBIT(UCA0IE, UCRXIE); // Receive interrupt enable
            //Event_clr(EVENT_PRINT);
        }
        break;
    }
}


GLOBAL Int UCA0_printf(const Char *str)
{
    if (str EQ NULL || Event_tst(EVENT_PRINT))
    {
        return -1;
    }
    Event_set(EVENT_PRINT);

    ptr = str;
    SETBIT(UCA0IFG, UCTXIFG); // Set UCTXIFG
    SETBIT(UCA0IE, UCTXIE);   // Enable transmit interrupt

    return 0;
}
