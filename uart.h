#ifndef __UART_H__
#define __UART_H__

#include "config.h"

extern volatile unsigned char buf[BUFSIZE+1];
extern volatile uint8_t pos;
extern volatile uint8_t msg_available;
extern volatile uint32_t received;
extern volatile uint32_t sent;

extern void uart_init(void);
extern void uart_putc(unsigned char c);
extern void uart_puts (const unsigned char *s) ;
extern void uart_putps (const unsigned char *p);
extern void uart_ok (const unsigned char *s);
extern void uart_err (const unsigned char *s);
extern void uart_greeting (void);
extern void uart_putn(uint8_t val, uint8_t lf);
extern void uart_putl(uint32_t val, uint8_t lf);
extern void uart_prompt(void);
extern void uart_put_rbuf(void);

#endif // __UART_H__
