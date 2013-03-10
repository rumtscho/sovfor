/* Blink LED example */

#include <stdio.h>
#include "serialcon.h"
#include "onewire.h"
#include "panic.h"


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

int getTempInHalfDegrees() {
	onewire_t ow;
	int i, ret;
	uint8_t scratchpad[9];

	ow.port_out = &P2OUT;
	ow.port_in = &P2IN;
	ow.port_ren = &P2REN;
	ow.port_dir = &P2DIR;
	ow.pin = BIT3;

	ret = onewire_reset(&ow);
	if( ret != 0 )
		printf("error reset 1: %d\n", ret);
	onewire_write_byte(&ow, 0xcc); // skip ROM command
	onewire_write_byte(&ow, 0x44); // convert T command
	onewire_line_high(&ow);
	delay_ms(800); // at least 750 ms for the default 12-bit resolution
	ret = onewire_reset(&ow);
	if( ret != 0 )
		printf("error reset 2: %d\n", ret);
	onewire_write_byte(&ow, 0xcc); // skip ROM command
	onewire_write_byte(&ow, 0xbe); // read scratchpad command
	for (i = 0; i < 9; i++)
		scratchpad[i] = onewire_read_byte(&ow);

    printf("scratchpad: ");
    for (i = 0; i < 9; i++)
    	printf("%02x ", scratchpad[i]);
    printf("\n");

    return scratchpad[0];
}

void flipExtLed(int port, int pin){
	if(port==1)
	{
		P1DIR |= pin;
		P1OUT ^= pin;
	}
	else if (port==2)
	{
		P2DIR |= pin;
		P2OUT ^= pin;
	}

}

void reportTemp(){
	int doubledTemperature = getTempInHalfDegrees();
	printf("temperature: %d.%d\n", doubledTemperature/2, (doubledTemperature&1)*5 );
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
			} else if (strcmp(line, "t")==0) {
				reportTemp();
			} else if (strcmp(line, "extled")==0){
				flipExtLed(1, BIT7);
			} else {
				serialcon_writeln("Huh?  Try 'help'");
			}

			serialcon_writeln("Enter next command:");
		}
	}
}

void setupComm()
{
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
}

int main(void) {

	//disable watchdog and setup clock speeds
	WDTCTL = WDTPW | WDTHOLD;
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;

	/* The basic clock system control register 3 controls the oscillator
	 * the auxilliary clock uses.  Change ACLK from external timer crystal
	 * oscillator to internal very low-power oscillator (VLO) clock,
	 * which runs at appoximately 12kHz.
	 */
	BCSCTL3 |= LFXT1S_2;

	//
	P1DIR |= LED_GRN + LED_RED;// set red and green LEDs as output
	P1DIR &= ~BIT3; //set taster (at 1.3) as input
	P1IE |= BIT3; //Interrupt for 1.3
	P1IES |= BIT3; //?
	P1IFG &= ~BIT3; //set interrupt flag to zero
	P1REN |= BIT3; //pullup/pulldown resistor enable
	P1OUT |= BIT3; //decide if resistor is pullup or pulldown
	P1OUT &= ~LED_GRN; //turn off LED


	setupComm(); //setup the speeds needed for UART

	_EINT(); //enable all interrupts

	serialcon_writeln("Enter a command:");

	/* Main loop */
	while (true) {
		delay_ms(1);

			check_for_input();

	}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1() {



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

