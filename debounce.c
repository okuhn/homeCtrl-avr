/************************************************************************/
/*                                                                      */
/*                      Debouncing 8 Keys                               */
/*                      Sampling 4 Times                                */
/*                      With Repeat Function                            */
/*                                                                      */
/*              Author: Peter Dannegger                                 */
/*                      danni@specs.de                                  */
/*                                                                      */
/************************************************************************/
 
#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "config.h"
#include "debounce.h"

 
volatile uint8_t key_state;                       	// debounced and inverted key state:
													// bit = 1: key pressed
volatile uint8_t key_press;                       	// key press detect
 
volatile uint32_t millis = 0;

volatile uint32_t seconds = 0;
 
uint8_t prescaler;
 
static inline void debounce(void) {                       		// every 10ms
	static uint8_t ct0, ct1;
	uint8_t i;

	TCNT0 = (uint8_t)(int16_t)-(F_CPU / 1024 * 10e-3 + 0.5);  // preload for 10ms

	i = key_state ^ ~KEY_PIN;                       // key changed ?
	ct0 = ~( ct0 & i );                             // reset or count ct0
	ct1 = ct0 ^ (ct1 & i);                          // reset or count ct1
	i &= ct0 & ct1;                                 // count until roll over ?
	key_state ^= i;                                 // then toggle debounced state
	key_press |= key_state & i;                     // 0->1: key press detect

	millis += 4;
}
 
ISR(TIMER1_COMPA_vect)  {

	debounce();
	
#if F_CPU % DEBOUNCE                     			// bei rest
	OCR1A = F_CPU / DEBOUNCE - 1;      				// compare DEBOUNCE - 1 times
#endif

	if( --prescaler == 0 ){
		prescaler = (uint8_t) DEBOUNCE;
		seconds++;            
		   						// exact one second over
#if F_CPU % DEBOUNCE         						// handle remainder
		OCR1A = F_CPU / DEBOUNCE + F_CPU % DEBOUNCE - 1; 	// compare once per second
#endif
	}	
} 
 
 
///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed. Each pressed key is reported
// only once
//
uint8_t get_key_press( uint8_t key_mask ) {
	cli();                                          // read and clear atomic !
	key_mask &= key_press;                          // read key(s)
	key_press ^= key_mask;                          // clear key(s)
	sei();
	return key_mask;
}

void debounce_init(void) {
	// Configure debouncing routines and clock
	KEY_DDR &= ~ALL_KEYS;                			// configure key port for input
	KEY_PORT |= ALL_KEYS;                			// and turn on pull up resistors

    TCCR1B = (1<<WGM12) | (1<<CS10);      			// divide by 1
													// clear on compare
    OCR1A = F_CPU / DEBOUNCE - 1;          			// Output Compare Register
    TCNT1 = 0;                            			// Timmer startet mit 0
    seconds = 0;
    prescaler = (uint8_t)DEBOUNCE;          			// software teiler
 
    TIMSK1 = 1<<OCIE1A;                   			// beim Vergleichswertes Compare Match                    
													// Interrupt (SIG_OUTPUT_COMPARE1A)


/*
    TCCR0A = 0;
	TCCR0B = (1<<CS02)|(1<<CS00);         // divide by 1024
	TCNT0 = (uint8_t)(int16_t)-(F_CPU / 1024 * 10e-3 + 0.5);  // preload for 10ms
	TIMSK0 |= 1<<TOIE0;                   // enable timer interrupt
*/
}
