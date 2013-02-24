#include <string.h> // memset()
#include "ringbuf.h"

void rbuf_clear(RingBuf *buf)
{
	buf->start = 0;
	buf->end = 0;
	memset(buf->elems, 'Q', RINGBUF_SIZE);  // XXX debugging
	buf->ready = false;
}

bool rbuf_compare(RingBuf *buf, const char *bytes, unsigned len)
{
	unsigned i, pos;

	pos = buf->start;
	for (i = 0; i < len; i++) {
		if (pos == buf->end)
			return false;	// reached end of buffer before comparison completed
		if (bytes[i] != buf->elems[pos])
			return false;	// found a mismatch

		// advance pos, checking for rollover
		if (pos >= RINGBUF_SIZE - 1)
			pos = 0;
		else
			pos++;
	}

	return true;
}

void rbuf_delete(RingBuf *buf, unsigned len)
{
	unsigned i;
	char c;

	for (i = 0; i < len; i++)
		rbuf_read(buf, &c);
}

void rbuf_delete_until(RingBuf *buf, char c)
{
	char tmp;

	while (rbuf_read(buf, &tmp)) {
		if (tmp == c)
			break;
	}
}

void rbuf_flush(RingBuf *buf)
{
#ifdef TESTING
	char c;
	while (rbuf_read(buf, &c))
		/* do nothing - pop off into air */ ;
#else
	if (!rbuf_is_empty(buf))
		IE2 |= UCA0TXIE;  // the interrupt USCIAB0TX_ISR() does the actual work in the background
#endif
}

bool rbuf_is_empty(const RingBuf *buf)
{
	if (buf->end == buf->start)
		return true;
	else
		return false;
}

bool rbuf_peek(RingBuf *buf, char *c)
{
	if (rbuf_is_empty(buf))
		return false;

	*c = buf->elems[buf->start];
	return true;
}

bool rbuf_read(RingBuf *buf, char *c)
{
	// bail out if there is nothing to read
	if (rbuf_is_empty(buf))
		return false;

	// pop byte off buffer
	*c = buf->elems[buf->start];
	buf->elems[buf->start] = 'q';  // XXX debugging

	// move position forward, roll over if at end
	if (buf->start >= RINGBUF_SIZE - 1)
		buf->start = 0;
	else
		buf->start++;

	return true;
}

unsigned rbuf_readstr(RingBuf *buf, char *str, unsigned len)
{
	unsigned newlen;
	bool rc;

	// move characters from `buf' to `str' until end of buf -or- str is full
	for (newlen = 0; newlen < (len - 1); newlen++) {
		rc = rbuf_read(buf, str + newlen);
		if (!rc)  // break out when nothing is left to read
			break;
	}
	str[newlen] = 0;

	return newlen;
}

unsigned rbuf_waiting(const RingBuf *buf)
{
	int len;

	len = buf->end - buf->start;
	if (len >= 0)	// start is before end
		return len;
	else			// end is before start
		return (buf->start - RINGBUF_SIZE) + (buf->end + 1);
}

bool rbuf_write(RingBuf *buf, char c)
{
	buf->elems[buf->end] = c;

	// advance write position, roll over if at end
	if (buf->end >= RINGBUF_SIZE - 1)
		buf->end = 0;
	else
		buf->end++;

	// if the buffer is full, the oldest byte is overwritten
	if (buf->end == buf->start) {
		// advance read position, roll over if at end
		if (buf->start >= RINGBUF_SIZE - 1)
			buf->start = 0;
		else
			buf->start++;
		return false;
	}

	return true;
}

void rbuf_writestr(RingBuf *buf, const char *str)
{
	unsigned len, i;
	bool rc;

	len = strlen(str);
	for (i = 0; i < len; i++) {
		rc = rbuf_write(buf, str[i]);
		// flush the buffer now if it is overflowing or about to overflow
#ifndef TESTING
		if (rc == false || rbuf_waiting(buf) <= RINGBUF_SIZE - 2)
			rbuf_flush(buf);
#endif
	}
#ifndef TESTING
	rbuf_flush(buf);
#endif
}

void rbuf_writestr_nf(RingBuf *buf, const char *str)
{
	unsigned len, i;

	len = strlen(str);
	for (i = 0; i < len; i++)
		rbuf_write(buf, str[i]);
}


/*******************************************************************************
 * Ring buffer unit test
 * Compile with -DTESTING=1 for byte op test, =2 for string test
 ******************************************************************************/
#ifdef TESTING
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void rbuf_print(RingBuf *buf)
{
	unsigned i;

	for (i = 0; i < buf->end; i++)
		putchar(' ');
	puts("v");

	for (i = 0; i < RINGBUF_SIZE; i++)
		putchar(buf->elems[i]);
	putchar('\n');

	for (i = 0; i < buf->start; i++)
		putchar(' ');
	puts("^\n");
}

char *randstr()
{
	int choice = (random() % RINGBUF_SIZE) + 1;
	int i;

	char *teststr = malloc(choice);
	for (i = 0; i < choice; i++)	// fill with different letters depending on length
		teststr[i] = choice + 64;	// 65 = 'A'
	return teststr;
}

void fakedelete(RingBuf *buf, unsigned size)
{
	rbuf_delete(buf, size);
	printf("DELETED %u\n", size);
}

void fakeread(RingBuf *buf)
{
	char c = ' ';
	bool rc;

	rc = rbuf_read(buf, &c);
	printf("READ BYTE %c,  ret = %s\n", c, rc ? "OK" : "UNDERFLOW");
}

void fakewrite(RingBuf *buf)
{
	char c = '+';
	bool rc;

	// if overwriting use hash instead of plus
	if (buf->elems[buf->end] == '+' || buf->elems[buf->end] == '#')
		c = '#';

	rc = rbuf_write(buf, c);
	printf("WROTE BYTE %c, ret = %s\n", c, rc ? "OK" : "OVERFLOW");
}

int main()
{
	RingBuf testbuf;
	int r, i, cnt;

	puts("Hello from " __DATE__ " " __TIME__);

	rbuf_clear(&testbuf);

	// throw random shit at it
#if TESTING == 1
	while (true) {
		srandom(time(NULL));
		r = (random() % 3) + 1;
		cnt = (random() % RINGBUF_SIZE) + 1;
		if (r <= 2) {
			/* byte driven operations */
			for (i = cnt; i >= 1; i--) {
				printf("%d/%d bytes remaining\n", i, cnt);
				switch (r) {
					case 1:
						fakeread(&testbuf);
						break;
					case 2:
						fakewrite(&testbuf);
						break;
					default:
						return 1;
				}
				rbuf_print(&testbuf);
				sleep(2);
			}
		} else {
			/* larger operations */
			switch (r) {
				case 3:
					fakedelete(&testbuf, cnt);
					break;
				default:
					return 1;
			}
			rbuf_print(&testbuf);
			sleep(2);
		}
	}
#else
	while (true) {
		srandom(time(NULL));
		r = (random() % 2) + 1;
		cnt = (random() % RINGBUF_SIZE) + 1;
		char tmpstr[RINGBUF_SIZE];
		char *teststr = randstr();
		unsigned bytes_read;
		switch (r) {
			case 1:
				bytes_read = rbuf_readstr(&testbuf, tmpstr, cnt);
				printf("READ STR of reqlen=%u, gotlen=%u - %s\n", cnt, bytes_read, tmpstr);
				break;
			case 2:
				rbuf_writestr(&testbuf, teststr);
				printf("WROTE STR of len=%u             - %s\n", (unsigned)strlen(teststr), teststr);
				free(teststr);
				break;
			default:
				return 1;
		}
		rbuf_print(&testbuf);
		sleep(2);
	}
#endif

	return 0;
}
#endif
