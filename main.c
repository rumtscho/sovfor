/* Blink LED example */
 
#include <msp430g2553.h>

volatile int mode;
 
/** Delay function. **/
void delay(unsigned int d) {
  int i;
  for (i = 0; i<d; i++) {
    nop();
  }
}

void dot(int bitnr) {
    P1OUT |= bitnr;
    delay(0x8fff);
    P1OUT &= ~bitnr;
    delay(0x8fff);
}

void dash(int bitnr) {
    P1OUT |= bitnr;
    delay(0xffff);
    delay(0xffff);
    P1OUT &= ~bitnr;
    delay(0x8fff);
}

void empty() {
    delay(0xffff);
    delay(0xffff);
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

  for (;;) {
if(mode==1)
 morseRumi();

else{
morseAxel();
  }
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

