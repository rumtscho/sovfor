/* Blink LED example */

#include "serialcon.h"
#include "panic.h"

volatile int mode;

const int dot_delay = 100;

void dot(int bitnr) {
	P1OUT |= bitnr;
	delay_ms(dot_delay);
	P1OUT &= ~bitnr;
	delay_ms(dot_delay);
}

void dash(int bitnr) {
	P1OUT |= bitnr;
	delay_ms(2*dot_delay);
	P1OUT &= ~bitnr;
	delay_ms(dot_delay);
}

void empty() {
	delay_ms(3*dot_delay);
}

void morseRumi() {
	// .-. ..- -- ..  heisst rumi
	dot(BIT0);
	dash(BIT0);
	dot(BIT0);
	empty();
	dot(BIT0);
	dot(BIT0);
	dash(BIT0);
	empty();
	dash(BIT0);
	dash(BIT0);
	empty();
	dot(BIT0);
	dot(BIT0);
	empty();
}

void morseAxel() {
	//.- -..- . .-.. heisst Axel
	dot(BIT6);
	dash(BIT6);
	empty();
	dash(BIT6);
	dot(BIT6);
	dot(BIT6);
	dash(BIT6);
	empty();
	dot(BIT6);
	empty();
	dot(BIT6);
	dash(BIT6);
	dot(BIT6);
	dot(BIT6);
	empty();
}

static void check_for_input();

void check_for_input()
{
	char line[SERIAL_BUFFER_SIZE];
	unsigned len;
	memset(line, 'Z', SERIAL_BUFFER_SIZE);  // XXX debugging

	if (serialcon_has_input) {
		len = serialcon_readln(line, SERIAL_BUFFER_SIZE);
		if (len > 0) {  // == 0 would be when it rcvd unprintable control characters
			if (strncmp(line, "help", 4) == 0) {
				serialcon_writeln("Valid commands: help, panic, hello, green");
			} else if (strncmp(line, "panic", 5) == 0) {
				panic(PANIC_USER_ERROR);
			} else if (strncmp(line, "hello", 5) == 0) {
				serialcon_writeln("Hey, what's up?");
			} else if (strncmp(line, "green", 5) == 0) {
				P1OUT |= LED_GRN;
				delay_ms(25);
				P1OUT &= ~LED_GRN;
			} else if (strcmp(line, "axel")==0){
				morseAxel();
			} else if (strcmp(line, "rumi")==0){
				morseRumi();
			} else {
				serialcon_writeln("Huh?  Try 'help'");
			}

			serialcon_writeln("Enter next command:");
		}
	}
}


int main(void) {
	WDTCTL = WDTPW | WDTHOLD;
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;

	/* The basic clock system control register 3 controls the oscillator
	 * the auxilliary clock uses.  Change ACLK from external timer crystal
	 * oscillator to internal very low-power oscillator (VLO) clock,
	 * which runs at appoximately 12kHz.
	 */
	BCSCTL3 |= LFXT1S_2;

	P1DIR |= BIT0 + BIT6;
	P1DIR &= ~BIT3;
	P1REN |= BIT3;
	P1OUT |= BIT3;
	P1OUT &= ~BIT6;

	P1IE |= BIT3;
	P1IES |= BIT3;
	P1IFG &= ~BIT3;

	mode = 0;

	_EINT();

	/* USCI setup */
	P1SEL = BIT1 + BIT2;			// Set pin modes to USCI_A0
	P1SEL2 = BIT1 + BIT2;			// Set pin modes to USCI_A0
	P1DIR |= BIT2;					// Set 1.2 to output
	UCA0CTL1 |= UCSSEL_2;			// USCI_A use SMCLK
#if 0 // 1 MHz
	UCA0BR0 = 104;					// 1 MHz -> 9600
	UCA0BR1 = 0;					// 1 MHz -> 9600
	UCA0MCTL = UCBRS1;				// Modulation UCBRSx = 5
#else // 8 MHz
	UCA0BR0 = 0x41;	                // 8 MHz -> 9600
	UCA0BR1 = 0x03;                 // 8 MHz -> 9600
	UCA0MCTL = UCBRS0;				// Modulation UCBRSx = 1
#endif
	UCA0CTL1 &= ~UCSWRST;			// **Initialize USCI state machine**
	IE2 |= UCA0RXIE;				// Enable USCI_A0 RX interrupt
	serialcon_setup();

	serialcon_writeln("Enter a command:");

	/* Main loop */
	while (true) {
		delay_ms(1);
		check_for_input();
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1() {

	if(mode==0)
		mode=1;
	else
		mode=0;

	P1IFG &= ~BIT3;
	// disable button interrupt
	P1IE &= ~BIT3;

	/* set watchdog timer to trigger every 681 milliseconds -- normally
	 * this would be 250 ms, but the VLO is slower
	 */
	WDTCTL = WDT_ADLY_250;
	/* clear watchdog timer interrupt flag */
	IFG1 &= ~WDTIFG;
	/* enable watchdog timer interrupts; in 681 ms the button
	 * will be re-enabled by WDT_ISR() -- program will continue in
	 * the meantime.
	 */
	IE1 |= WDTIE;
}

/* This function catches watchdog timer interrupts, which are
 * set to happen 681ms after the user presses the button.  The
 * button has had time to "bounce" and we can turn the button
 * interrupts back on.
 */
void WDT_ISR(void) __attribute__((interrupt(WDT_VECTOR)));
void WDT_ISR(void)
{
	/* Disable interrupts on the watchdog timer */
	IE1 &= ~WDTIE;
	/* clear the interrupt flag for watchdog timer */
	IFG1 &= ~WDTIFG;
	/* resume holding the watchdog timer so it doesn't reset the chip */
	WDTCTL = WDTPW + WDTHOLD;
	/* and re-enable interrupts for the button */
	P1IE |= BIT3;
}

