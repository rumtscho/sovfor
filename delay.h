#ifndef DELAY_H_
#define DELAY_H_

#define CYCLES_PER_US 8L // depends on the CPU speed
#define CYCLES_PER_MS (CYCLES_PER_US * 1000L)

void delay_us(int us);

void delay_ms(int ms);


#endif /* DELAY_H_ */
