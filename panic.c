#include "panic.h"

void panic(PanicCode code)
{
	int blinkcnt;

	_DINT();  // we can't escape from a panic with an interrupt

	while (true) {
		for (blinkcnt = 0; blinkcnt < code; blinkcnt++) {
			P1OUT |= LED_RED;
			delay_ms(200);
			P1OUT &= ~LED_RED;
			delay_ms(200);
		}
		delay_ms(400);
	}
}
