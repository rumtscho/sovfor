#include "global.h"
#include "serialcon.h"

/*
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR()
{

}

#pragma vector=COMPARATORA_VECTOR
__interrupt void COMPARATORA_ISR()
{

}
*/

// nonmaskable system error
#pragma vector=NMI_VECTOR
__interrupt void NMI_ISR()
{
	serialcon_writeln("Error!  Unhandled interrupt NMI_ISR!");
}

/*
#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR()
{

}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR()
{

}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR()
{

}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR()
{

}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR()
{

}

#pragma vector=TIMER1_A1_VECTOR
__interrupt void TIMER1_A1_ISR()
{

}
*/

/*
 * USCI_A has received data
 */
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR()
{
	P1OUT |= LED_GRN;

	/*
	 * Some USCI_A data is waiting to be read - move it to the memory buffer.
	 * If the buffer is full, the oldest data will be overwritten.
	 */
	char c;
	while (IFG2 & UCA0RXIFG) {			// while receiver has data
		c = UCA0RXBUF;					//  get it out of the register
		rbuf_write(&serialcon_rxbuf, c);//  move it to the circular buffer
		if (c == '\r' || c =='\n')		//  if the user pressed enter
			serialcon_has_input = true;	//   set a flag to alert the console
	}

	P1OUT &= ~LED_GRN;
}

/*
 * USCI_A is ready to send data
 */
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR()
{
	P1OUT |= LED_GRN;

	/*
	 * Flush the data out, and once it is all out, we shut the interrupt off
	 * until the next round.
	 */
	char c;
	while (IFG2 & UCA0TXIFG) {				// while transmitter is ready for data
		if (rbuf_read(&serialcon_txbuf, &c))//  if the circular buffer has data
			UCA0TXBUF = c;					//   then send it
		else {								//  else the circular buffer is empty
			IE2 &= ~UCA0TXIE;				//   so disable this interrupt
			break;
		}
	}

	P1OUT &= ~LED_GRN;
}

/*
#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR()
{

}
*/
