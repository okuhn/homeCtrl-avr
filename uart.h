#ifndef __UART_H__
#define __UART_H__

#include "config.h"

extern volatile char buf[BUFSIZE+1];
extern volatile uint8_t pos;
extern volatile uint8_t msg_available;
extern volatile uint32_t received;
extern volatile uint32_t sent;

extern void uart_init(void);
extern void uart_putc(char c);
extern void uart_puts (const char *s) ;
extern void uart_putps (const char *p);
extern void uart_ok (const char *s);
extern void uart_err (const char *s);
extern void uart_greeting (void);
extern void uart_version (void);
extern void uart_putn(uint8_t val, uint8_t lf);
extern void uart_putl(uint32_t val, uint8_t lf);
extern void uart_prompt(void);
extern void uart_put_rbuf(void);
extern void uart_put_time(uint32_t seconds);

#endif // __UART_H__
