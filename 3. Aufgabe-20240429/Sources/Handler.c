#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA0.h"
#include "UCA1.h"
#include "TA0.h"

#define DIGISIZE 4
#define BASE 8 // base of the number system can be selected between 2 and 16

// data type of a constant function pointer
typedef Void (* VoidFunc)(Void);

LOCAL UChar button_index; // identify the external BCD Button that was pressed
LOCAL UChar bcd_cnt[DIGISIZE]; // BCD counter
LOCAL Char  bcd_uart[DIGISIZE + 3]; // BCD counter array for UART TX (2 additional chars for '\r' and '\n')

// functional prototypes
LOCAL Void State0(Void);
LOCAL Void State1(Void);

LOCAL VoidFunc state; // function pointer to the current state function
LOCAL UChar idx; // index for the BCD counter

LOCAL UChar error;                   // error variable for UART


// ---------------------------------------------------------------------------- Button Handling
static void BCD_Button_Handler(TEvent arg, UChar bcd_button)
{
    if(Event_tst(arg))
    {
        Event_clr(arg); // clear the regarding button event
        button_index = bcd_button; // set the button index
        Event_set(EVENT_UPDATE_CNT); // set event for updating BCD
    }
}


GLOBAL Void Button_Handler(Void)
{
    BCD_Button_Handler(EVENT_BTN3, 0);
    BCD_Button_Handler(EVENT_BTN4, 1);
    BCD_Button_Handler(EVENT_BTN5, 2);
    BCD_Button_Handler(EVENT_BTN6, 3);
}

// ---------------------------------------------------------------------------- Number Handling
GLOBAL Void Number_Handler(Void)
{
    //UInt overflow;

    if(Event_tst(EVENT_UPDATE_CNT))
    {
        Event_clr(EVENT_UPDATE_CNT);

        // checking the PIN is cleaner than handling a second variable
        if(!TSTBIT(P2OUT, BIT7)) // increment
        {
            bcd_cnt[button_index]++;
            //overflow = 0;
        }
        else // decrement
        {
            bcd_cnt[button_index]--;
            //overflow = BASE-1;
        }

        if(bcd_cnt[button_index] GE BASE)
        {
            bcd_cnt[button_index] = (!TSTBIT(P2OUT,BIT7)) ? 0 : BASE - 1;
            button_index++;
            if(button_index GE DIGISIZE)
            {
                button_index = 0;
            }
            else
            {
                Event_set(EVENT_UPDATE_CNT);
            }
        }

        // if there is no more EVENT_UPDATE_CNT pending display can updated
        if(!Event_tst(EVENT_UPDATE_CNT))
        {
            Event_set(EVENT_UPDATE_BCD);
        }
    }
}

// ---------------------------------------------------------------------------- BCD Handling
static void State0(void)
{
    if (Event_tst(EVENT_UPDATE_BCD))
    {
        Event_clr(EVENT_UPDATE_BCD);
        idx = 1;
        state = State1;
        Event_set(EVENT_DONE_BCD);
    }
}


LOCAL Void State1(Void)
{

    if (Event_tst(EVENT_DONE_BCD))
    {
        Event_clr(EVENT_DONE_BCD);
        if (idx LE DIGISIZE)
        {
            UChar ch = bcd_cnt[idx - 1];
            UCA1_emit(idx, ch);
            idx++;
        }
        else
        {
            state = State0;
            Event_set(EVENT_TXD);
        }
    }
}


GLOBAL Void AS1108_Handler(Void)
{
    (*state)();
}


GLOBAL Void get_bcd_cnt(Void)
{
    // returns the BCD counter as a string with UART line ending (LSB first) e.g. "1234\r\n"
    // the string is stored in bcd_uart
    for(int i = 0; i < 4; i++)
    {
        bcd_uart[i] = bcd_cnt[3-i] + '0';
    }
    /*bcd_uart[0] = bcd_cnt[3] + '0';
    bcd_uart[1] = bcd_cnt[2] + '0';
    bcd_uart[2] = bcd_cnt[1] + '0';
    bcd_uart[3] = bcd_cnt[0] + '0';*/
    bcd_uart[4] = '\r';
    bcd_uart[5] = '\n';
    bcd_uart[6] = '\0';
}
// ---------------------------------------------------------------------------- UCA0 Handler

GLOBAL Void UART_Handler(Void)
{
    if(Event_tst(EVENT_RXD))
    {
        Event_clr(EVENT_RXD);

        for(int i = 0; i < 4; i++)
        {
            bcd_cnt[i] = rx_buf[3-i] - '0';
        }
        /*bcd_cnt[0] = rx_buf[3] - '0';
        bcd_cnt[1] = rx_buf[2] - '0';
        bcd_cnt[2] = rx_buf[1] - '0';
        bcd_cnt[3] = rx_buf[0] - '0';*/
        Event_set(EVENT_UPDATE_BCD);
    }

    if(Event_tst(EVENT_TXD))
    {
        Event_clr(EVENT_TXD);
        get_bcd_cnt();
        UCA0_printf(bcd_uart);
    }
}

//----------------------------------------------------------------------------- Error Handler

GLOBAL Void Error_Handler(Void)
{
    if(Event_tst(EVENT_ERR))
    {
        Event_clr(EVENT_ERR);

        // error handling
        set_blink_muster(error - 1);
    }
}

GLOBAL Void set_error(UChar err)
{
    if(err == NO_ERROR || error > err)
    {
        error = err;
        Event_set(EVENT_ERR);
    }
}

// ---------------------------------------------------------------------------- Initialisation
GLOBAL Void Handler_init(Void)
{
    state = State0; // initial state
    idx = 1; // initial index

    //bcd_cnt[0] = 0;
    //bcd_cnt[1] = 0;
    //bcd_cnt[2] = 0;
    //bcd_cnt[3] = 0;

    for(UChar i = 0; i < DIGISIZE; i++)
    {
        bcd_cnt[i] = 0; // Initial BCD counter
    }
}
