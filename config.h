#ifndef __CONFIG_H__
#define __CONFIG_H__

#ifdef F_OSC
#define F_CPU F_OSC
#endif
 
#ifndef F_CPU
#error F_CPU undefined
#endif
 
#define BAUD 57600UL          				

#define KEY_DDR         DDRD
#define KEY_PORT        PORTD
#define KEY_PIN         PIND
#define KEY0            5
#define KEY1            6
#define KEY2            7
#define ALL_KEYS        (1<<KEY0 | 1<<KEY1 | 1<<KEY2)

#define BUFSIZE 40

#define DEBOUNCE    256L        // debounce clock (256Hz = 4msec)

#define TZ_OFFSET 3600			// winter
// #define TZ_OFFSET 7200    	// summer


#endif // __CONFIG_H__
