#ifndef __DEBOUNCE_H__
#define __DEBOUNCE_H__

extern void debounce_init(void);
extern uint8_t get_key_press(uint8_t key_mask);

extern volatile uint32_t millis;
extern volatile uint32_t seconds;

#endif // __DEBOUNCE_H__
