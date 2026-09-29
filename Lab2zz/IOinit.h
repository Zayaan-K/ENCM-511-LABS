#ifndef IOINIT_H
#define IOINIT_H

#include <stdint.h>


void initTimer2(uint8_t prescaler, uint8_t clk, uint8_t runInIdle, uint8_t priority, uint8_t interruptEnable, uint16_t period);
void initTimer3(uint8_t prescaler, uint8_t clk, uint8_t runInIdle, uint8_t priority, uint8_t interruptEnable, uint16_t period);

void initButtons(void);
void initLeds(void);
void initISR(void);


extern volatile uint16_t timer2Ticks;
extern volatile uint16_t timer3Ticks;

extern volatile uint8_t pb0Down;
extern volatile uint8_t pb1Down;
extern volatile uint8_t pb2Down;
extern volatile uint8_t pb2Click;

#endif