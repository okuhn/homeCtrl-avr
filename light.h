#ifndef __LIGHT_H__
#define __LIGHT_H__

extern uint8_t handle_light(const unsigned char *cmd, unsigned char action);
extern void handle_light_query(void);

#endif // __LIGHT_H__
