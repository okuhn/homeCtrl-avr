#include <avr/io.h>

#include "config.h"
#include "light.h"
#include "uart.h"
#include "history.h"

uint8_t handle_light(const unsigned char *cmd, unsigned char action) {
    log_cmd(cmd, action);

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
		
		if (cmd[2] < '0' || cmd[2] > '1') {
			return 0;
		}
		
		uint8_t val = cmd[2] - '0';

#if INVERSE
		val = !val;
#endif
		
        if (val) {
            LIGHT_PORT |= mask;
        } else {
            LIGHT_PORT &= ~mask;
        }
    }
    
    return mask;
}

void handle_light_query(void) {
    uart_putc('l');
    uart_putc('=');
    uart_putl(LIGHT_PORT & 0x07, 1);
}
