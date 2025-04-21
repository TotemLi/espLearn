#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

void blink_led(uint8_t state);
void configure_led(void);

#ifdef __cplusplus
}
#endif

#endif
