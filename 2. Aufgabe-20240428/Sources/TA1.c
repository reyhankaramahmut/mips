#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"

// For hysteresis
#define CNT_MAX 5 // Maximum count for the hysteresis

// Arrays to store the pin masks, bit masks and events for the buttons
const UChar bit_mask[6] = {BIT1, BIT0, BIT0, BIT1, BIT2, BIT3};
const TEvent event[6] = {EVENT_BTN1, EVENT_BTN2, EVENT_BTN3, EVENT_BTN4, EVENT_BTN5, EVENT_BTN6};

// Arrays to store the counters and states for the buttons
static UChar cnt_state[6];
static UChar index; // Index for the currently selected button

// Function to initialize Timer A1
#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void)
{
    // Configuration of Timer A1
    CLRBIT(TA1CTL, MC0 | MC1    // Stop mode
            | TAIE              // Disable interrupt
            | TAIFG);           // Clear interrupt flag
    CLRBIT(TA1CCTL0, CM1 | CM0  // No capture mode
            | CAP               // Compare mode
            | CCIE              // Disable interrupt
            | CCIFG);           // Clear interrupt flag

    TA1CCR0  = 20-1;            // Set compare register
    TA1EX0   = TAIDEX_7;        // Set extension register
    TA1CTL   = TASSEL__ACLK     // Set clock source to ACLK (614.4 kHz)
            | MC__UP            // Up mode
            | ID__8             // Set frequency divider to 8
            | TACLR             // Clear and start timer
            | TAIE;             // Enable interrupt
}

// Interrupt routine for Timer A1
#pragma vector = TIMER1_A1_VECTOR
__interrupt Void TIMER1_A1_ISR(Void)
{
    Bool btn;

    if(index <= 1)
        btn = TSTBIT(P1IN, bit_mask[index]);
    else
        btn = TSTBIT(P3IN, bit_mask[index]);


    UChar state = cnt_state[index] & 0x01;
    UChar cnt = cnt_state[index] >> 1;

    if(btn) // Is the button pressed?
    {
        if(cnt LT CNT_MAX) // Is the counter less than the maximum (prevents cnt from exceeding max)
        {
            cnt++;
            if(cnt EQ (CNT_MAX) && state == 0) // Is the new counter (+1) equal to cnt_max?
            {
                    state = 1;
                    Event_set(event[index]); // Set the event
                    __low_power_mode_off_on_exit(); // Exit the Low Power Mode after leaving the ISR
            }
        }
    }
    else // Is the button not pressed?
    {
        if(cnt> 0) // As long as the counter is greater than 0
            cnt--; // Decrease it by 1
        if(cnt EQ 0)
            state = 0;
    }

    cnt_state[index] = (cnt << 1) | state;

    // Switch the button for the next interrupt
    if(index EQ 5)
        index = 0;
    else
        index++;

    // Clear the timer interrupt flag
    CLRBIT(TA1CTL, TAIFG);
}
