#include <avr/io.h>
#include <avr/interrupt.h>
#include <ctype.h>

#include "config.h"
#include "uart.h"
#include "debounce.h"
#include "history.h"
#include "light.h"
#include "watchdog.h"

volatile char *cmd;

uint8_t ldr_enabled = 1;

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
    uart_putc('e');
    uart_putc('=');
    uart_putl(ldr_enabled,1);
    uart_putc('S');
    uart_putc('=');
    uart_putl((seconds + TZ_OFFSET) % (60*60*24L),1);
    uart_putc('L');
    uart_putc('=');
    uart_putl(LIGHT_PORT & (1 << 2), 1);
    uart_putc('T');
    uart_putc('=');
    uart_putl(60L*60*23+41*60, 1);
    uart_putc('X');
    uart_putc('=');
    uart_putl(60L*60*23+41*60, 1);
    uart_put_rbuf();
    cmd++;
}

void show_version(void) {
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

#if LDR
uint16_t adc_read(void) {
    ADCSRA |= (1<<ADSC);
    while (ADCSRA & (1<<ADSC)) {
    }
    return ADCW;
}

void adc_init(void) {
    ADMUX = (1<<REFS0) |(1<<REFS0) | LDR_ADC;   // internal reference ADCC, input = LDR_ADC
    ADCSRA |= (1<<ADEN);						// enable ADC
    ADCSRA |= (1<<ADPS2);           			// ADC clock frequency divison by 8
    adc_read();
}

void show_light(void) {
    uart_putc('d');
    uart_putc('=');
    uart_putl(adc_read(),1);
    cmd++;   
}

void handle_adc(void) {
	if (adc_read() <= MIN_BRIGHTNESS) {
		STATUS_PORT |=  (1 << 5);
		if (ldr_enabled) {
			handle_light("l21", 'a');
			ldr_enabled = 0;
		}
	} else {
		STATUS_PORT &= ~(1 << 5);
	}
}
#endif

void handle_serial(void) {
    if (msg_available) {
        uint8_t error = 0;

        cmd = buf;

        while (!error && *cmd != '\0') {
            if (*cmd == 'l') {
                error = handle_light(cmd, 'w') == 0;
                cmd += 3;
            } else if (*cmd == 'q') {
                handle_light_query();
                cmd++;
            } else if (*cmd == 't') {
                handle_time();
            } else if (*cmd == 's') {
                read_statistics();
            } else if (*cmd == 'v') {
                show_version();
            } else if (*cmd == 'h') {
                log_show();
                cmd++;
#if LDR
            } else if (*cmd == 'd') {
                show_light();
#endif
            } else if (*cmd == 'r') {
				cmd++;
                reset();
            } else if (*cmd == 'e') {
				ldr_enabled = !ldr_enabled;
				cmd++;
                // reset();                
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

void toggle(uint8_t pin, uint8_t keys) {
    uint8_t flag = keys & (1<<pin);
    if (flag) {
        char cmd[4] = "l00";
        cmd[1] += pin;
        cmd[2] += !(PORTB & flag);
        handle_light(cmd, 'b');    
    }
}

void handle_keys(void) {
    uint8_t key_press = get_key_press(ALL_KEYS) >> KEY0;    // toggle pins according to key presses
    toggle(0, key_press);
    toggle(1, key_press);
    toggle(2, key_press);
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
    STATUS_PORT = 0x01;
    _delay_ms(200);
    STATUS_PORT = 0x02;
    _delay_ms(200);
    STATUS_PORT = 0x01;
    _delay_ms(200);
    STATUS_PORT = 0x00;
}

uint8_t off_enable = 1;

int main(void){
	// wdt_disable();
   
#if INVERSE
    LIGHT_PORT = 0xff;
#else
    LIGHT_PORT = 0x00;
#endif
    LIGHT_DDR = 0xff;

    STATUS_PORT = 0;
    STATUS_DDR = 0xff;
    
    KEY_PORT = 0xff;
    KEY_DDR = 0x00;
  
    blink();
    uart_init( );
    debounce_init();

#if LDR
    adc_init();
#endif
    
    sei();

    uart_greeting();
    uart_prompt();
    
    while (1) {
        handle_keys();
        handle_serial();
      
#if LDR
		handle_adc();
#endif
      

#if 0
        uint8_t sec = seconds % 60;

        if (sec == 0) {
            PORTC |= (1<<5);
        } else if (sec == 1) {
            PORTC &= ~(1<<5);
        }
#endif

        uint32_t day_seconds = (seconds + TZ_OFFSET) % (60*60*24L);
        if ((day_seconds == (60L*60*12))) {  // enable ldr at 12:00
			ldr_enabled = 1;
			off_enable = 1;
		}

        if ((day_seconds == (60L*60*23+8*60)) && (LIGHT_PORT & (1 << 2))) {  // put off light at 23:08
			handle_light("l20", 't');
		}

#if 0
        if (day_seconds == (60L*60*23+41*60)) {
			if (off_enable) {
				handle_light("l20", 't');
				off_enable = 0;
			}
		}
#endif
		
    } 

    return 0;
}

