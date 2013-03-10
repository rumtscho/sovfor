#include "serialcon.h"
#include "panic.h"

int putchar(int c)
{
	if (c == '\n') putchar('\r');
	while (!(IFG2&UCA0TXIFG));
	UCA0TXBUF = c;
	return 0;
}

// flushes any data in the buffer to the console
static void serialcon_flush();

RingBuf serialcon_rxbuf;
RingBuf serialcon_txbuf;
bool serialcon_has_input;

void serialcon_setup()
{
	rbuf_clear(&serialcon_rxbuf);
	rbuf_clear(&serialcon_txbuf);
	serialcon_has_input = false;
}

unsigned serialcon_readln(char *line, unsigned len)
{
	char c;
	unsigned pos;

	/*
	 * Due to how the circular buffer is implemented, its contents has a maximum
	 * of SERIAL_BUFFER_SIZE - 1.  This gives us room to put a terminating NUL
	 * at the end of `line' so that it can be processed as a string.
	 */
	if (len < SERIAL_BUFFER_SIZE)
		panic(PANIC_RUNTIME_ERROR);

	if (rbuf_is_empty(&serialcon_rxbuf))
		panic(PANIC_LOGIC_ERROR);  // shouldn't have `serialcon_has_input' set with no data to read!

	pos = 0;
	while (!rbuf_is_empty(&serialcon_rxbuf)) {
		rbuf_read(&serialcon_rxbuf, &c);
		// don't copy control characters - http://www.ascii-code.com/
		if (isprint(c)) {
			line[pos] = c;
			pos++;
		}
	}
	line[pos] = '\0';
	serialcon_has_input = false;
	return pos;  // doesn't include \0 to be like strlen() return value
}

void serialcon_writeln(const char *line)
{
	unsigned i;
	unsigned len = strlen(line);

	for (i = 0; i < len; i++)
		rbuf_write(&serialcon_txbuf, line[i]);
	rbuf_write(&serialcon_txbuf, '\r');
	rbuf_write(&serialcon_txbuf, '\n');

	serialcon_flush();
}

void serialcon_flush()
{
	if (!rbuf_is_empty(&serialcon_txbuf))
		IE2 |= UCA0TXIE;  // the interrupt USCIAB0TX_ISR() does the actual work in the background
}
