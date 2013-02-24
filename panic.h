#ifndef PANIC_H
#define PANIC_H
#include "global.h"

/*******************************************************************************
 * An unrecoverable error has occurred
 ******************************************************************************/
typedef enum {
	PANIC_LOGIC_ERROR = 1,
	PANIC_RUNTIME_ERROR = 2,
	PANIC_OUT_OF_MEMORY = 3,
	PANIC_USER_ERROR = 4
} PanicCode;

// delay
void pause(unsigned multiplier);

// freezes MCU and blinks red LED x times corresponding to error code listed above
void panic(PanicCode code);

#endif // PANIC_H
