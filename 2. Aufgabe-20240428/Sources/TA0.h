#include "..\base.h"

#ifndef TA0_H_
#define TA0_H_

#define MUSTER1 0
#define MUSTER2 1
#define MUSTER3 2
#define MUSTER4 3
#define MUSTER5 4
#define MUSTER6 5

#define HIGH 0x8000
#define LOW 0x00


#define ACKFRQ 614.4 // kHz
#define TIMEBASE 50  //ms
#define AMOUNT_BLINK_PATTERNS 6


#define SCALING ((UInt)(ACKFRQ * TIMEBASE))
#define TICK(t) ((SCALING / 64) * ((t) / TIMEBASE) - 1)


EXTERN Void TA0_init(Void);
EXTERN Void set_blink_muster(UInt);

#endif /* TA0_H_ */
