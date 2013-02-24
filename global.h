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

/*
 * Red LED indicates errors
 * Green LED is lit when the MCU is an interrupt
 */
#define LED_RED BIT0
#define LED_GRN BIT6

#define SERIAL_BUFFER_SIZE 80

#endif // GLOBAL_H
