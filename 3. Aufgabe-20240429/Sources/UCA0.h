/*
 * UCA0.h
 *
 *  Created on: Jun 25, 2024
 *      Author: Robin
 */

#ifndef SOURCES_UCA0_H_
#define SOURCES_UCA0_H_

#include "..\base.h"
#include "Handler.h"


extern Char rx_buf[DIGISIZE + 1];

EXTERN Void UCA0_init(Void);
EXTERN Int UCA0_printf(const Char * str);



#endif /* SOURCES_UCA0_H_ */
