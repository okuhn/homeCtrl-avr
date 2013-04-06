#include <avr/io.h>
#include "config.h"
#include "debounce.h"
#include "uart.h"
#include "history.h"

typedef struct hist_entry {
    uint32_t time;
    char cmd[8];
    char action;
    uint32_t info;
} hist_entry;

static hist_entry hist[HISTSIZE];

static int8_t next = 0;
static uint8_t count = 0;

void log_cmd(const char *cmd, char action) {
    hist[next].time = seconds;
    for (int i=0; i<7; i++) {
	hist[next].cmd[i] = cmd[i];
    }
    hist[next].cmd[7] = '\0';
    hist[next].action = action;
    next++;
    if (next == HISTSIZE) {
        next = 0;
    }
    if (count < HISTSIZE) {
        count++;
    }
}

void log_show() {
    int8_t pos = next;
    for (uint8_t i=0; i<count; i++) {
        pos--;
        if (pos < 0) {
            pos = HISTSIZE - 1;
        }
        uart_putc('h');
        uart_putc('=');
        // uart_put_time(hist[pos].time);
        uart_putl(hist[pos].time, 0);
        uart_putc(';');
        uart_puts(hist[pos].cmd);
        uart_putc(';');
        uart_putc(hist[pos].action);
        uart_putc('\r');
        uart_putc('\n');
    }
}


