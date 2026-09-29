#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

// identifies which LED Timer3 controls
#define BLINK_LED0 1
#define BLINK_LED1 2

// timer setup
void Timer2_init(void);
void Timer3_init(void);

// Timer2 based delay
void delay_ms(uint16_t ms);

// Timer3 blink control
void start_blink(uint16_t ms, uint8_t led);
void stop_blink(void);

#endif