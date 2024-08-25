#include <msp430.h>
#include "..\base.h"
#include "TA0.h"
#include "event.h"
#include <stdio.h>

// Variablen f�r Blinkmuster
static UChar pattern_index_new_selected; // Neu ausgew�hltes Muster
static UChar pattern_index; // Ausgew�hltes Muster
static UChar array_index_phase; // Aktuelle Phase

// Array von Blinkmustern mit Anzahl der Ticks. Jedes Muster besteht aus einer Sequenz von HIGH und LOW Phasen,
// wobei jede Phase eine bestimmte Anzahl von Ticks dauert. Ein Muster endet mit einer 0.
LOCAL const UInt blink_pattern[] =
{
HIGH | TICK(2000), LOW | TICK(500), 0,
HIGH | TICK(750), LOW | TICK(750), 0,
HIGH | TICK(250), LOW | TICK(250), 0,
LOW | TICK(500), HIGH | TICK(500), LOW | TICK(1500), 0,
LOW | TICK(500), HIGH | TICK(500), LOW | TICK(500), HIGH | TICK(500), LOW | TICK(1500), 0,
LOW | TICK(500), HIGH | TICK(500), LOW | TICK(500), HIGH | TICK(500), LOW | TICK(500), HIGH | TICK(500), LOW | TICK(1500), 0
};

// Zeiger auf den Anfang jedes Musters
const UInt *const pattern_pointers[AMOUNT_BLINK_PATTERNS] =
{
    &blink_pattern[0],    // Muster 1
    &blink_pattern[3],    // Muster 2
    &blink_pattern[6],    // Muster 3
    &blink_pattern[9],    // Muster 4
    &blink_pattern[13],   // Muster 5
    &blink_pattern[19]    // Muster 6
};

// Funktion zum Ausw�hlen des n�chsten Musters. Diese Funktion wird in der main Funktion aufgerufen und
// setzt die Variable pattern_index_new_selected auf den �bergebenen Wert.
GLOBAL Void set_blink_muster(UInt arg)
{
    pattern_index_new_selected = arg;
}

// Funktion zur Initialisierung von Timer A0. Diese Funktion konfiguriert den Timer A0 f�r den Betrieb,
// setzt das erste Blinkmuster und startet den Timer.
#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {
    CLRBIT(TA0CTL, MC0 | MC1   // Stop-Modus
                  | TAIE      // Unterbrechung deaktivieren
                  | TAIFG);   // Unterbrechungsflag l�schen
    CLRBIT(TA0CCTL0, CM1 | CM0 // Kein Erfassungsmodus
                  | CAP       // Vergleichsmodus
                  | CCIE      // Unterbrechung deaktivieren
                  | CCIFG);   // Unterbrechungsflag l�schen
    TA0CCR0  = 0;              // Vergleichsregister einrichten
    TA0EX0   = TAIDEX_7;       // Erweiterungsregister einrichten 614.4Khz * 0,25 = 153600
                               // 153600 /8 /8 = 2400

    pattern_index_new_selected = MUSTER1;
    pattern_index = MUSTER1;
    array_index_phase = 0;

    TA0CTL   = TASSEL__ACLK    // 614.4 kHz
             | MC__UP          // Aufw�rtsmodus
             | ID__8           // /8
             | TACLR           // Timer l�schen und starten
             | TAIE            // Unterbrechung aktivieren
             | TAIFG;          // Unterbrechungsflag setzen
                               // ---> 614.4kHz / 8(TA0CTL = ID__8) / 8(TA0EX0 = TAIDEX_7)
                               // = 9600Hz
                               // 9600Hz * 0,25s = 2400
}

// Unterbrechungsroutine f�r Timer A0. Diese Funktion wird aufgerufen, wenn der Timer A0 eine Unterbrechung ausl�st.
// Sie steuert das Blinken der LED gem�� dem ausgew�hlten Muster.
#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1_ISR(Void)
{
    // Wenn das Ende der aktuellen Phase erreicht ist, wird das n�chste Muster ausgew�hlt.
    if((*(pattern_pointers[pattern_index] + array_index_phase)) EQ 0)
    {
        array_index_phase = 0;
        pattern_index = pattern_index_new_selected;
    }
    // Je nach Wert des aktuellen Phasenbits wird die LED ein- oder ausgeschaltet.
    if(TSTBIT(*(pattern_pointers[pattern_index] + array_index_phase), HIGH))
    {
        SETBIT(P1OUT, BIT2); // LED einschalten
    } else {
        CLRBIT(P1OUT, BIT2); // LED ausschalten
    }
    // Das Vergleichsregister wird auf die Anzahl der Ticks der aktuellen Phase gesetzt.
    TA0CCR0 = ~HIGH BAND (*(pattern_pointers[pattern_index] + array_index_phase));
    // Der Index der aktuellen Phase wird inkrementiert, um zur n�chsten Phase zu wechseln.
    array_index_phase++;

    // Das Timer-Unterbrechungsflag wird gel�scht, um die n�chste Unterbrechung zu erm�glichen.
    CLRBIT(TA0CTL, TAIFG);
}
