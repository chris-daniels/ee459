/* ATtiny4313 - Receive the button signals over bluetooth 
*
* Program updates LEDS on PD2-6 and PB0-1 based on button data received over bluetooth
* BAUD rate of 9600 for bluetooth
* Looks for a "#" ascii character as the sign that the next byte is button data
*
* 05/11/15 Morelle Arian, Andrew Leung, Chris Daniels
* 
*************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "sci.h"


#define BUTTONMASK 0b00011111 // 0x1f on pb0-pb4
/* PB4 = LEFT
 * PB3 = RIGHT
 * PB2 = BRAKE 1
 * PB1 = BRAKE 2
 * PB0 = REAR VIEW CAMERA
 */

/*0 = left
  2, 4 = brake ! 
  8 = right
  */

void port_init();
volatile char left = 1; // bool for flashing
volatile char right = 1; // bool for flashing 
volatile char buttons = 0; // initialize to buttons are all off 
                                // (remember that if = 0 that means on, if = 1 that means off) 
void blink_right();
void blink_left();
void off_right();
void off_left(); 

    
int main(void)
{   
    sci_init(63, 0, 1); // buad rate, turn on receiver, turn off transmitter
    port_init();
    char received = 0;
    char flag = 0; 
    /*
       The demo board has a 9.8304 MHz clock.  We want the timer to
       interrupt every half second (2 Hz) so we need to count clocks to
       9.8304MHz/2Hz = 4,915,200.  This is too big for the 16 bit counter
       register so use the prescaler to divide the clock by 256 and then
       count that clock to 19,200.
    */
    // Reset clears register bits to zero so only set the 1's
    TCCR1B |= (1 << WGM12);     // Set for CTC mode.  OCR1A = modulus
    TIMSK |= (1 << OCIE1A);    // Enable CTC interrupt
    sei();                      // Enable global interrupts
    OCR1A = 19200;              // Set the counter modulus
    TCCR1B |= (1 << CS12);      // Set prescaler for divide by 256,
                                // also starts timer


    while(1) {                  // Loop forever while interrupts occur
        if(sci_got()) {
            received = sci_in();
            //off_left();
            //off_right();
            if(flag) {
                buttons = received;
                flag = 0;
            }
            else if (received == 0x23) // this is "#" (pound) in ascii
                flag = 1;
        
            if (buttons & ((1<<1) | (1<<2))) // turn on brake light
                PORTB |= (1<<PB1);
            else // turn off brake light
                PORTB &= ~(1<<PB1);
            
            if (buttons & (1<<0)) // turn on camera
                PORTB |= (1<<PB2);
            else // turn off camera
                PORTB &= ~(1<<PB2);
                
            if (buttons & 0x10)
                left = 1;
            else {
                left = 0;
                off_left();
            }
                
            if (buttons & 0x08)
                right = 1;
            else {
                right = 0;
                off_right();
            }
        }        
    }

    return 0; /* never reached */
}

ISR(TIMER1_COMPA_vect)
{
    if (left)
        blink_left();
    if (right)
        blink_right();
}

void blink_left() {
    if(PORTD & (1<<PD5)) {  // first light
        PORTD &= ~(1<<PD5); // turn off and
        PORTD |= (1<<PD6);  // blink next light
    }
    else if(PORTD & (1<< PD6)) {
        PORTD &= ~(1<< PD6);
        PORTB |= (1<<PB0);
    }
    else if(PORTB & (1<<PB0)) {
        PORTB &= ~(1<<PB0);
        PORTD |= (1<<PD5);
    }
    else {
        PORTD |= (1<<PD5); // start the cycle
    }
}

void off_left() {
    PORTD &= ~(1<<PD5);
    PORTD &= ~(1<<PD6);
    PORTB &= ~(1<<PB0);  
}

void blink_right() {
    if(PORTD & (1<<PD2)) {
        PORTD &= ~(1<<PD2);
        PORTD |= (1<<PD3);
    }
    else if(PORTD & (1<<PD3)) {
        PORTD &= ~(1<<PD3);
        PORTD |= (1<<PD4);
    }
    else if(PORTD & (1<<PD4)) {
        PORTD &= ~(1<<PD4);
        PORTD |= (1<<PD2);
    }
    else {
        PORTD |= (1<<PD2);
    }
}

void off_right() {
    PORTD &= ~(1<<PD2);
    PORTD &= ~(1<<PD3);
    PORTD &= ~(1<<PD4);
}


void port_init() {
  // set leds to outputs
    // PD2-6 (5), PB0-1 (2)
  DDRD |= 0b01111100; // signal lights
  DDRB |= ((1<<PB0) | (1<<PB1)); // break light
  DDRB |= (1<<PB2); // for the camera. hopefully this is okay. 
}

