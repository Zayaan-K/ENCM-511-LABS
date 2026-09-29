#ifndef IO_H
#define IO_H

#include <xc.h>
#include <stdint.h>

// LED pin names
#define LED0 LATBbits.LATB5
#define LED1 LATBbits.LATB6
#define LED2 LATBbits.LATB7

// button pin names
#define PB0 PORTAbits.RA4
#define PB1 PORTBbits.RB8
#define PB2 PORTBbits.RB9

// stored button states
extern volatile uint8_t pb0_pressed;
extern volatile uint8_t pb1_pressed;
extern volatile uint8_t pb2_pressed;

// button events
extern volatile uint8_t button_event;
extern volatile uint8_t press_event;
extern volatile uint8_t pb2_clicked;

// IO setup
void IOinit(void);

#endif