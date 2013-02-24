#ifndef SERIALCON_H
#define SERIALCON_H
#include "global.h"
#include "ringbuf.h"

/*******************************************************************************
 * Serial debugging console on USCI_A
 ******************************************************************************/
extern RingBuf serialcon_rxbuf;	// data queued to read
extern RingBuf serialcon_txbuf;	// data queued to write
extern bool serialcon_has_input;	// a line of input is ready for parsing

// sets up the console
void serialcon_setup();

// reads a line of input from the console
// returns length of line
unsigned serialcon_readln(char *line, unsigned len);

// immediately transmits a line of data to the console
void serialcon_writeln(const char *line);

#endif // SERIALCON_H
