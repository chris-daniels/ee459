/* ATtiny4313 - Send over bluetooth the button signals every 0.1 seconds
*
* Program loops reading PB0-PB4
* and sending the data over bluetooth
*
* 9600 BAUD rate and sending every 0.1 seconds
* 
* 05/11/15 Morelle Arian, Andrew Leung, Chris Daniels
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include "sci.h"

#define BUTTONMASK 0b00011111 // 0x1f on pb0-pb4
/* PB4 = LEFT
 * PB3 = RIGHT
 * PB2 = BRAKE 1
 * PB1 = BRAKE 2
 * PB0 = REAR VIEW CAMERA
 */

void port_init();

/* BUAD equation:
 * UBRR = fosc / (16 * BUAD) - 1
 * UBRR = 9830400 / (16 * 9600) - 1 = 63
 * This is the first parameter sent to sci_init()
 */

/* Looking on scope at the digital output, the output is of this form:
 * Low bit before char, high bit after char, the character in the middle is transmitted lowest bit first, highest bit last
 */


int main(void)
{
  sci_init(63, 1, 1); // buad rate, turn on transmitter, turn off receiver
  port_init(); // set buttons to inputs
  char buttons = BUTTONMASK;// initialize to buttons are all off 
                            // (remember that if = 0 that means on, if = 1 that means off) 
  while (1) {
    // send buttons byte every 100ms 
    buttons = PINB;  
    sci_out(~buttons & BUTTONMASK);
    _delay_ms(100);
      
    // check if button values have changed; if they have, send info via bluetooth
    /*if(buttons != (PINB & BUTTONMASK)) {
        buttons = PINB & BUTTONMASK;
        //buttons_out = ~buttons & BUTTONMASK;
        sci_out(~buttons & BUTTONMASK); // invert to output intuitive output...
    }*/
      
  }
  return 0; /* never reached */
}

void port_init() {
  // set buttons to inputs
  DDRB &= ~(BUTTONMASK);
  // turn on pull up resistors
  PORTB |= BUTTONMASK;
}

