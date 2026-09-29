#include "io.h"

// stored button states
volatile uint8_t pb0_pressed = 0;
volatile uint8_t pb1_pressed = 0;
volatile uint8_t pb2_pressed = 0;

// events shared between ISR and main
volatile uint8_t button_event = 0;
volatile uint8_t press_event = 0;
volatile uint8_t pb2_clicked = 0;


// initialize LEDs buttons and IOC
void IOinit(void)
{
    // make analog capable pins digital
    ANSELA = 0x0000;
    ANSELB = 0x0000;

    // LEDs are outputs
    TRISBbits.TRISB5 = 0;
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB7 = 0;

    // buttons are inputs
    TRISAbits.TRISA4 = 1;
    TRISBbits.TRISB8 = 1;
    TRISBbits.TRISB9 = 1;

    // LEDs start off
    LED0 = 0;
    LED1 = 0;
    LED2 = 0;

    // enable button pullups
    IOCPUA |= (1 << 4);
    IOCPUB |= (1 << 8) | (1 << 9);

    // enable IOC module
    PADCONbits.IOCON = 1;

    // PB0 interrupt on press and release
    IOCPA |= (1 << 4);
    IOCNA |= (1 << 4);

    // PB1 and PB2 interrupt on press and release
    IOCPB |= (1 << 8) | (1 << 9);
    IOCNB |= (1 << 8) | (1 << 9);

    // clear IOC pin flags
    IOCFA = 0;
    IOCFB = 0;

    // read initial button states
    pb0_pressed = (PB0 == 0);
    pb1_pressed = (PB1 == 0);
    pb2_pressed = (PB2 == 0);

    // make main process initial state
    button_event = 1;

    // enable IOC interrupt
    IFS1bits.IOCIF = 0;
    IPC4bits.IOCIP = 4;
    IEC1bits.IOCIE = 1;
}


// runs when a button changes state
void __attribute__((interrupt, no_auto_psv)) _IOCInterrupt(void)
{
    // save previous states
    uint8_t old_pb0 = pb0_pressed;
    uint8_t old_pb1 = pb1_pressed;
    uint8_t old_pb2 = pb2_pressed;

    // read new states
    uint8_t new_pb0 = (PB0 == 0);
    uint8_t new_pb1 = (PB1 == 0);
    uint8_t new_pb2 = (PB2 == 0);

    // detect any new button press
    if((!old_pb0 && new_pb0) ||
       (!old_pb1 && new_pb1) ||
       (!old_pb2 && new_pb2))
    {
        press_event = 1;
    }

    // PB2 click completes on release
    if(old_pb2 && !new_pb2)
    {
        pb2_clicked = 1;
    }

    // store new button states
    pb0_pressed = new_pb0;
    pb1_pressed = new_pb1;
    pb2_pressed = new_pb2;

    // tell main something changed
    button_event = 1;

    // clear IOC flags
    IOCFA = 0;
    IOCFB = 0;
    IFS1bits.IOCIF = 0;
}