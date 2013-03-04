#include "global.h"
#include "delay.h"

void delay_us(int us)
{
	while (us--)
		__delay_cycles(CYCLES_PER_US);
}

void delay_ms(int ms)
{
	while (ms--)
		__delay_cycles(CYCLES_PER_MS);
}
