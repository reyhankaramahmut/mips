#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA1.h"
#include "TA0.h"

#define DIGISIZE 4
#define BASE 8 // base of the number system can be selected between 2 and 16

// data type of a constant function pointer
typedef Void (* VoidFunc)(Void);

LOCAL UChar button_index; // identify the external BCD Button that was pressed
LOCAL UChar bcd_cnt[DIGISIZE]; // BCD counter

// functional prototypes
LOCAL Void State0(Void);
LOCAL Void State1(Void);

LOCAL VoidFunc state; // function pointer to the current state function
LOCAL UChar idx; // index for the BCD counter

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
        }
    }
}

GLOBAL Void AS1108_Handler(Void)
{
    (*state)();
}

// ---------------------------------------------------------------------------- Initialisation
GLOBAL Void Handler_init(Void)
{
    state = State0; // initial state
    idx = 1; // initial index

    for(UChar i = 0; i < DIGISIZE; i++)
    {
        bcd_cnt[i] = 0; // Initial BCD counter
    }
}
