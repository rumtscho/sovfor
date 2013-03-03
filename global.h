#ifndef GLOBAL_H
#define GLOBAL_H

/*
 * System headers
 */
#include <msp430g2553.h>
#include <ctype.h>		// isprint()
#include <stdbool.h>	// bool
#include <string.h>		// C string functions

#define OFF false
#define ON true

#define LED_RED BIT0
#define LED_GRN BIT6

#define SERIAL_BUFFER_SIZE 80

#define CYCLES_PER_US 8L // depends on the CPU speed
#define CYCLES_PER_MS (CYCLES_PER_US * 1000L)

inline void delay_us(int us)
{
	while (us--)
#if CYCLES_PER_US > 4
		__delay_cycles(CYCLES_PER_US-4); // the -4 is to correct for delays from the while loop
#else
	__delay_cycles(CYCLES_PER_US);
#endif
}

inline void delay_ms(int ms)
{
	while (ms--)
		__delay_cycles(CYCLES_PER_MS);
}

#endif // GLOBAL_H
