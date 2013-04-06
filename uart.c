#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "config.h"
#include "uart.h"

#define UBRR_VAL ((F_OSC+BAUD*8)/(BAUD*16)-1)   // calculate UBBR_VAL
#define BAUD_REAL (F_OSC/(16*(UBRR_VAL+1)))     // real baud rate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD)      // calculate relative error
 
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error baud rate error exceeds limit +-1%
  #error BAUD_REAL
#endif


volatile char buf[BUFSIZE+1];
volatile uint8_t pos = 0;
volatile uint8_t msg_available = 0;

volatile uint32_t received = 0;
volatile uint32_t sent = 0;

const char ok_msg[] PROGMEM = " ok\r\n";
const char err_msg[] PROGMEM = " error\r\n";
const char hello_msg[] PROGMEM = "Welcome to homeCtrl Version " VERSION " \r\n";
const char version_msg[] PROGMEM = "v=homeCtrl Version " VERSION " \r\nv=Built " __DATE__ " " __TIME__ "\r\n";

volatile char rbuf[BUFSIZE];
volatile uint8_t rpos = 0;

void uart_init(void)
{
    UCSR0B |= (1<<RXEN0);
    UCSR0B |= (1<<RXCIE0);      			// enable UART RXCIE
    UCSR0B |= (1<<TXEN0);                	// enable UART TX
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);    	// asynchronous 8N1 

    UBRR0H = UBRR_VAL >> 8;
    UBRR0L = UBRR_VAL & 0xFF;

	for (uint8_t i=0; i<BUFSIZE; i++) {
		rbuf[i] = 0;
	}
}

void uart_putc(char c)
{
    while (!(UCSR0A & (1<<UDRE0))) {  		// wait until buffer is available
    }                             
 
    UDR0 = c;                      			// send char
    sent++;
}

void uart_puts(const char *s) {
    while (*s) {
        uart_putc(*s++);
    }
}

void uart_putps(const char *p) {
    char c;
    while ((c = pgm_read_byte(p++)) != '\0') {
        uart_putc(c);
    }
}

void uart_ok(const char *s) {
    uart_puts(s);
    uart_putps(ok_msg);
} 

void uart_err(const char *s) {
    uart_puts(s);
    uart_putps(err_msg);
}

void uart_greeting(void) {
    uart_putps(hello_msg);
} 

void uart_version(void) {
    uart_putps(version_msg);
} 

void uart_put_digit(uint8_t n) {
	uart_putc('0' + n);
}

void uart_putn(uint8_t val, uint8_t lf) {
    uart_put_digit(val/10);
    uart_put_digit(val%10);
	if (lf) {
		uart_putc(13);
		uart_putc(10);
	}
}

void uart_put_time(uint32_t time) {
        uint32_t tmp_seconds = time + TZ_OFFSET;

        uint8_t sec = tmp_seconds % 60;
        tmp_seconds /= 60;

        uint8_t min = tmp_seconds % 60;
        tmp_seconds /= 60;

        uint8_t hour = tmp_seconds % 24;

        uart_putn(hour, 0);
        uart_putc(':');
        uart_putn(min, 0);
        uart_putc(':');
        uart_putn(sec, 0);
}

void uart_put_rbuf(void) {
	// output ring buffer
	uart_putc('b');
	uart_putc('=');
	uint8_t n = BUFSIZE;
	uint8_t p = rpos;
	while (n--) {
		if (p == BUFSIZE) {
			p=0;
		}
		char c = rbuf[p++];
		uart_putl(c, 0);
		if (c >= 0x20 && c < 0x80) {
			uart_putc('=');
			uart_putc(c);
		}
		if (n) {
			uart_putc(',');
		}
	}
	uart_putc(13);
	uart_putc(10);
}

#define LONGVALBUFSIZE 15

void uart_putl(uint32_t val, uint8_t lf) {
	char buf[LONGVALBUFSIZE];
	char *bufp = buf + LONGVALBUFSIZE;
	
	*(--bufp) = 0;  						// String terminating 0
	
	if (val == 0) {							// add at least one zero digit to buf
		*(--bufp) = '0';
	}
	
	while (val > 0) {						// add digits to buf from right to left
		*(--bufp) = '0' + (val % 10);
		val /= 10;
	}
	
	uart_puts(bufp);						// print buf

	if (lf) {								// optionally print CR/LF
		uart_putc(13);
		uart_putc(10);
	}
}

void uart_prompt(void) {
    uart_putc('-');
    uart_putc('>');
    uart_putc(' ');
}

ISR (USART_RX_vect) {
    char c = UDR0;
    received++;
    
    if (rpos == BUFSIZE) {
		rpos = 0;
	}
	rbuf[rpos++] = c;

    if (msg_available) {
        return;
    }

    if (c == 13 || c == 10) {
        msg_available = 1;
        uart_putc(13);
        uart_putc(10);
    } else if (c >= 0x20 && c < 0x80 && pos < BUFSIZE) {	// consider only printable ascii chars and CR/LF
        buf[pos++] = c;
        buf[pos] = 0;
        uart_putc(c);
    }
}
