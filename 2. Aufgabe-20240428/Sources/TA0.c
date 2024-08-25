#include <msp430.h>
#include "..\base.h"
#include "TA0.h"
#include "event.h"
#include <stdio.h>

// Variablen für Blinkmuster
static UChar pattern_index_new_selected; // Neu ausgewähltes Muster
static UChar pattern_index; // Ausgewähltes Muster
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

// Funktion zum Auswählen des nächsten Musters. Diese Funktion wird in der main Funktion aufgerufen und
// setzt die Variable pattern_index_new_selected auf den übergebenen Wert.
GLOBAL Void set_blink_muster(UInt arg)
{
    pattern_index_new_selected = arg;
}

// Funktion zur Initialisierung von Timer A0. Diese Funktion konfiguriert den Timer A0 für den Betrieb,
// setzt das erste Blinkmuster und startet den Timer.
#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {
    CLRBIT(TA0CTL, MC0 | MC1   // Stop-Modus
                  | TAIE      // Unterbrechung deaktivieren
                  | TAIFG);   // Unterbrechungsflag löschen
    CLRBIT(TA0CCTL0, CM1 | CM0 // Kein Erfassungsmodus
                  | CAP       // Vergleichsmodus
                  | CCIE      // Unterbrechung deaktivieren
                  | CCIFG);   // Unterbrechungsflag löschen
    TA0CCR0  = 0;              // Vergleichsregister einrichten
    TA0EX0   = TAIDEX_7;       // Erweiterungsregister einrichten 614.4Khz * 0,25 = 153600
                               // 153600 /8 /8 = 2400

    pattern_index_new_selected = MUSTER1;
    pattern_index = MUSTER1;
    array_index_phase = 0;

    TA0CTL   = TASSEL__ACLK    // 614.4 kHz
             | MC__UP          // Aufwärtsmodus
             | ID__8           // /8
             | TACLR           // Timer löschen und starten
             | TAIE            // Unterbrechung aktivieren
             | TAIFG;          // Unterbrechungsflag setzen
                               // ---> 614.4kHz / 8(TA0CTL = ID__8) / 8(TA0EX0 = TAIDEX_7)
                               // = 9600Hz
                               // 9600Hz * 0,25s = 2400
}

// Unterbrechungsroutine für Timer A0. Diese Funktion wird aufgerufen, wenn der Timer A0 eine Unterbrechung auslöst.
// Sie steuert das Blinken der LED gemäß dem ausgewählten Muster.
#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1_ISR(Void)
{
    // Wenn das Ende der aktuellen Phase erreicht ist, wird das nächste Muster ausgewählt.
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
    // Der Index der aktuellen Phase wird inkrementiert, um zur nächsten Phase zu wechseln.
    array_index_phase++;

    // Das Timer-Unterbrechungsflag wird gelöscht, um die nächste Unterbrechung zu ermöglichen.
    CLRBIT(TA0CTL, TAIFG);
}
