#include <avr/wdt.h>
#include "watchdog.h"
#include "uart.h"

void reset() {
	wdt_enable(WDTO_15MS);
	uart_puts("resetting ...\r\n");
	while(1) {
	}
}

void wdt_init(void) __attribute__((naked)) __attribute__((section(".init1")));

void wdt_init(void)
{
	MCUSR = 0;
    wdt_disable();
    return;
}
