#include <avr/io.h>
#include <avr/interrupt.h>
#include <ctype.h>

#include "config.h"
#include "uart.h"
#include "debounce.h"
#include "history.h"

volatile unsigned char *cmd;

uint8_t handle_light(void) {
	int mask = 0;
	char c = cmd[1];
	
	// set bits in mask according to pins that should be changed
	if (c == '*') {
		mask = 0x07;
	} else if (c >= '0' && c <= '2') {
		uint8_t n = c - '0';
		mask = 1 << n;
	}
	
	// set or clear pins
	if (mask != 0) {
		if (cmd[2] == '1') {
			PORTB |= mask;
		} else if (cmd[2] == '0') {
			PORTB &= ~mask;
		} else {
			mask = 0;
		}
	}
	
	cmd += 3;
	
	return mask;
}

void handle_query(void) {
	uart_putc('l');
	uart_putc('=');
	uart_putl(PORTB & 0x07, 1);
	cmd++;
}

void set_time(void) {
	uint32_t tmp_seconds = 0;
	
	while (isdigit(*(++cmd))) {
		tmp_seconds *= 10;
		tmp_seconds += *cmd - '0';
	}
	
	cli();
	seconds = tmp_seconds;
	sei();
}

void read_time(void) {
	uart_putc('t');
	uart_putc('=');
	uart_putl(seconds,1);
	cmd++;
}

void read_statistics(void) {
	uart_putc('s');
	uart_putc('=');
	uart_putl(sent,1);
	uart_putc('r');
	uart_putc('=');
	uart_putl(received,1);
	uart_put_rbuf();
	cmd++;
}

void show_version() {
	uart_version();
	cmd++;
}
 
void read_time_format(void) {
        uart_put_time(seconds);
        uart_putc('\r');
        uart_putc('\n');

	cmd++;
}
 
void handle_time(void) {
	cmd++;
	if (*cmd == 's') {
		set_time();
	} else if (*cmd == 'r') {
		read_time();
	} else if (*cmd == 'f') {
		read_time_format();
	} 
}
 
void handle_serial(void) {
    if (msg_available) {
		uint8_t error = 0;

		cmd = buf;

		while (!error && *cmd != '\0') {
			if (*cmd == 'l') {
                                log_cmd(cmd);
				error = handle_light() == 0;
			} else if (*cmd == 'q') {
				handle_query();
			} else if (*cmd == 't') {
				handle_time();
			} else if (*cmd == 's') {
				read_statistics();
			} else if (*cmd == 'v') {
				show_version();
			} else if (*cmd == 'h') {
				log_show();
                                cmd++;
			} else {
				error = 1;
			}
		}
		
		if (error) {
		    uart_err(buf);
		} else {
			uart_ok(buf);
		}
		
		/*
		if (strcmp("on", buf) == 0) {
			PORTB |= 0x07;
		    uart_ok(buf);
		} else if (strcmp("off", buf) == 0) {
            PORTB &= 0x00;
		    uart_ok(buf);
		} else {
	    }
	    */
	    
	    uart_prompt();
	    pos = 0;
        buf[pos] = 0;
        msg_available = 0;
    }	 
}

extern uint8_t key_state;
extern uint8_t key_press;

void handle_keys(void) {
	PINB = get_key_press(ALL_KEYS) >> KEY0; 	// toggle pins according to key presses
}

/*
 * s1u shutter 1 up
 * s1d shutter 1 down
 * s1s shutter 1 stop
 * s1q shutter 1 query
 * 
 * s*u all shutters up
 * s*q query all shutters
 * 
 * ts time set
 * tr time read
 * tf time rad formatted
 */

#ifndef F_CPU
#define F_CPU F_OSC
#endif

#include <util/delay.h>

void blink(void) {
	PORTB = 0x01;
	_delay_ms(200);
	PORTB = 0x00;
	_delay_ms(700);
	PORTB = 0x02;
	_delay_ms(200);
	PORTB = 0x00;
	_delay_ms(700);
}

int main(void){
    DDRB = 0xff;
    PORTB = 0x0;

    DDRC= 0xff;
    PORTC = 0x0;
    
    DDRD = 0x00;
    PORTD = 0xff;
  
	blink();
    uart_init( );
    debounce_init();
    
    sei();

    uart_greeting();
	uart_prompt();
	
    while (1) {
		handle_keys();
		handle_serial();
		uint8_t sec = seconds % 60;
		if (sec == 0) {
			PORTC |= (1<<5);
		} else if (sec == 1) {
			PORTC &= ~(1<<5);
		}
    } 

    return 0;
}

