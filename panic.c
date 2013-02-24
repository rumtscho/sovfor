#include "panic.h"

void pause(unsigned multiplier)
{
	unsigned cnt;
	for (cnt = 0; cnt < multiplier; cnt++)
		__delay_cycles(65535);
}

void panic(PanicCode code)
{
	int blinkcnt;

	_DINT();  // we can't escape from a panic with an interrupt

	while (true) {
		for (blinkcnt = 0; blinkcnt < code; blinkcnt++) {
			P1OUT |= LED_RED;
			pause(5);
			P1OUT &= ~LED_RED;
			pause(5);
		}
		pause(10);
	}
}
