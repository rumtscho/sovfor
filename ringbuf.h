#ifndef RINGBUF_H
#define RINGBUF_H

#ifndef TESTING
#include "global.h"
#else
#include <stdbool.h>
#endif

/*******************************************************************************
 * Circular/ring buffer for FIFO queues
 ******************************************************************************/
#ifdef TESTING
#define RINGBUF_SIZE 10
#else
#define RINGBUF_SIZE 80
#endif
typedef struct {
	unsigned	start;		// reading position in buffer
	unsigned	end;		// writing position in buffer
	char		elems[RINGBUF_SIZE];
	bool		ready;		// A ready flag is set by an interrupt when some
							// expected text has arrived in the buffer (e.g. a
							// carriage return).  It is not used by any of the
							// buffer functions below and is expected to be
							// controlled externally.
} RingBuf;

// resets the buffer to be empty
void rbuf_clear(RingBuf *buf);

// compares the elements in the buffer to `bytes', which is `len' bytes long
// returns true for match
bool rbuf_compare(RingBuf *buf, const char *bytes, unsigned len);

// removes the next `size' bytes from the buffer
void rbuf_delete(RingBuf *buf, unsigned len);

// deletes all the characters up to and including `c' from the buffer
// will erase everything if `c' is not found!
void rbuf_delete_until(RingBuf *buf, char c);

// sends the data in buffer to the destination
void rbuf_flush(RingBuf *buf);

// returns true if the buffer is empty
bool rbuf_is_empty(const RingBuf *buf);

// the same as rbuf_read() but doesn't remove the byte from the buffer
bool rbuf_peek(RingBuf *buf, char *c);

// reads a byte from the buffer to `c' and removes it from the buffer
// returns false and changes nothing if the buffer was empty
bool rbuf_read(RingBuf *buf, char *c);

// reads up to `len' into 'str' and pops the bytes out of the buffer. result is always zero terminated
// returns length of new string
unsigned rbuf_readstr(RingBuf *buf, char *str, unsigned len);

// returns the number of bytes currently waiting in the buffer
unsigned rbuf_waiting(const RingBuf *buf);

// pushes a byte into the buffer - must be manually flush()ed
// returns false and overwrites the oldest data if the buffer is overflowing
bool rbuf_write(RingBuf *buf, char c);

// writes a string to the buffer and flush()es it (requires but skips zero termination character)
void rbuf_writestr(RingBuf *buf, const char *str);

// the same as above, except rbuf_flush() must be called manually.  nf = no flush
void rbuf_writestr_nf(RingBuf *buf, const char *str);

#ifdef TESTING
void rbuf_print(RingBuf *buf);
#endif

#endif // RINGBUF_H
