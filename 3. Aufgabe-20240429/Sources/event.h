#include "..\base.h"

#ifndef EVENT_H_
#define EVENT_H_

typedef unsigned int TEvent;

#define NO_EVENTS   0x0000
#define EVENT_1     0x0001
#define EVENT_2     0x0002
#define EVENT_3     0x0004
#define EVENT_4     0x0008
#define EVENT_5     0x0010
#define EVENT_6     0x0020 // Bis hier Button Handler
#define EVENT_7     0x0040 // UART
#define EVENT_8     0x0080 // UART
#define EVENT_9     0x0100
#define EVENT_10    0x0200 //AS1108 Handler
#define EVENT_11    0x0400 //AS1108 Handler
#define EVENT_12    0x0800 //Der Hier  Number handler
#define EVENT_13    0x1000
#define EVENT_14    0x2000
#define EVENT_15    0x4000
#define EVENT_16    0x8000 //ERROR Handler
#define ALL_EVENTS  0xFFFF

#define EVENT_BTN1        EVENT_1   // click on button 1
#define EVENT_BTN2        EVENT_2   // click on button 2
#define EVENT_BTN3        EVENT_3   // click on button 3 (external button 0)
#define EVENT_BTN4        EVENT_4   // click on button 4 (external button 1)
#define EVENT_BTN5        EVENT_5   // click on button 5 (external button 2)
#define EVENT_BTN6        EVENT_6   // click on button 6 (external button 3)

#define EVENT_RXD         EVENT_8   // Receiver event for uart
#define EVENT_TXD         EVENT_9   // Transmitter event for uart

#define EVENT_UPDATE_BCD  EVENT_10  // Update BCD Display
#define EVENT_DONE_BCD    EVENT_15  // Update BCD Display done
#define EVENT_UPDATE_CNT  EVENT_12  // Update count
#define EVENT_PRINT       EVENT_13

#define EVENT_ERR         EVENT_16  // Error event

#define NO_ERROR          6        // no error
#define BYTE_RECEIVED     5        // byte received
#define BUFFER_ERROR      4        // buffer error (e.g. to many bytes received)
#define CHARACTOR_ERROR   3        // charactor error (e.g. wrong charactor received)
#define FROVPAR_ERROR     2        // frame overrun or parity error
#define BREAK_ERROR       1        // break error (lost communication)

EXTERN Void Event_init(Void);
EXTERN Void Event_wait(Void);
EXTERN Void Event_set(TEvent);
EXTERN Void Event_clr(TEvent);
EXTERN Bool Event_tst(TEvent);
EXTERN Bool Event_err(Void);

#endif /* EVENT_H_ */
