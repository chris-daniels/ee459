/*************************************************************
*       at328-0.c - Demonstrate simple I/O functions of ATmega328
*
*       Program loops turning PC0 on and off as fast as possible.
*
* The program should generate code in the loop consisting of
*   LOOP:   SBI  PORTC,0        (2 cycles)
*           CBI  PORTC,0        (2 cycles)
*           RJMP LOOP           (2 cycles)
*
* PC0 will be low for 4 / XTAL freq
* PC0 will be high for 2 / XTAL freq
* A 9.8304MHz clock gives a loop period of about 600 nanoseconds.
*
* Revision History
* Date     Author      Description
* 09/14/12 A. Weber    Initial Release
* 11/18/13 A. Weber    Renamed for ATmega328P
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>

void sci_init(void);
void usart_out ( char ch);
char usart_in ();


int main(void)
{

	sci_init();
	char read;

    while (1) {

    	read = usart_in();
    	usart_out(read);
    	usart_out("\r");
    	usart_out("\n");

	}

    return 0;   /* never reached */
}

/* sci_init - Initialize the SCI (serial communication interface) port */

void sci_init(void) {
	unsigned short ubrr = 20e6/16/9600 - 1; // set buad rate to 9600
	UBRR0 = ubrr; 

	UCSR0B |= (1 << TXEN0); // turn on transmitter
	UCSR0B |= (1 << RXEN0); // turn on receiver
	UCSR0C = (3 << UCSZ00); // async, no parity, one stop, 8 data bits

}

/* usart_out - Output a byte to the USART0 port */
void usart_out ( char ch) {
	while (( UCSR0A & (1<< UDRE0 )) == 0);
	UDR0 = ch;
}

/* usart_in - Read a byte from the USART0 and return it */
char usart_in () {
	while ( !( UCSR0A & (1 << RXC0 )) );
	return UDR0 ;
}

