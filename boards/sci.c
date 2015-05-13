/*************************************************************
*       sci - I/O routines for the ATtiny4313 USART (RS-232) port
*
* Revision History
* Date     Author      Description
* 09/30/11 A. Weber    Adapted for ATtiny4313 from ATmega168 routines
*************************************************************/

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "sci.h"

/*
  sci_init - Initialize the SCI port
*/
void sci_init(unsigned short ubrr, unsigned char tx, unsigned char rx) {
    UBRRH = ubrr >> 8;     // Set baud rate
    UBRRL = ubrr & 0xff;
    if (tx)
	UCSRB |= (1 << TXEN);  // Turn on transmitter
    if (rx)
	UCSRB |= (1 << RXEN);  // Turn on receiver
    UCSRC = (3 << UCSZ0);  // Set for asynchronous operation, no parity, 
                             // one stop bit, 8 data bits
}

/*
  sci_out - Output a byte to SCI port
*/
void sci_out(char ch)
{
    while ((UCSRA & (1<<UDRE)) == 0);
    UDR = ch;
}

/*
  sci_outs - Print the contents of the character string "s" out the SCI
  port. The string must be terminated by a zero byte.
*/
void sci_outs(char *s)
{
    char ch;

    while ((ch = *s++) != (char) '\0')
        sci_out(ch);
 }

void sci_outs_P(const char *s)
{
    char ch;

    while (1) {
	ch = pgm_read_byte(s++);
	if (ch == (char) '\0')
	    break;
	else
	    sci_out(ch);
    }
 }

/*
  sci_in - Read a character from the USART and return it
*/
char sci_in()
{
    while ( !(UCSRA & (1 << RXC)) );
    return UDR;
}

/*
  sci_got - Check to see if a character has been received.
  Returns 1 if so, zero otherwise.
*/
char sci_got()
{
    if (UCSRA & (1 << RXC))
	return(1);
    else
	return(0);
}
