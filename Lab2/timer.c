#include <xc.h>
#include <stdint.h>
#include "io.h"
#include "timer.h"

// Timer2 delay variables
static volatile uint16_t delay_remaining = 0;
static volatile uint8_t delay_done = 0;

// stores which LED Timer3 controls
static volatile uint8_t blink_led = 0;


// Timer2 used for delay_ms
void Timer2_init(void)
{
    // 16 bit timer mode
    T2CONbits.T32 = 0;

    // timer prescaler
    T2CONbits.TCKPS = 1;

    // internal clock
    T2CONbits.TCS = 0;

    // keep timer running during Idle
    T2CONbits.TSIDL = 0;

    // timer period
    PR2 = 499;
    TMR2 = 0;

    // configure Timer2 interrupt
    IFS0bits.T2IF = 0;
    IPC1bits.T2IP = 2;
    IEC0bits.T2IE = 1;

    // timer starts off
    T2CONbits.TON = 0;
}


// Timer3 used for LED blinking
void Timer3_init(void)
{
    // 1 to 256 prescaler
    T3CONbits.TCKPS = 3;

    // internal clock
    T3CONbits.TCS = 0;

    // keep timer running during Idle
    T3CONbits.TSIDL = 0;

    TMR3 = 0;

    // configure Timer3 interrupt
    IFS0bits.T3IF = 0;
    IPC2bits.T3IP = 3;
    IEC0bits.T3IE = 1;

    // timer starts off
    T3CONbits.TON = 0;
}


// hardware timer delay
void delay_ms(uint16_t ms)
{
    if(ms == 0)
        return;

    // number of milliseconds remaining
    delay_remaining = ms;
    delay_done = 0;

    // start Timer2
    TMR2 = 0;
    IFS0bits.T2IF = 0;
    T2CONbits.TON = 1;

    // CPU sleeps between Timer2 interrupts
    while(!delay_done)
    {
        Idle();
    }
}


// starts LED blinking with Timer3
void start_blink(uint16_t ms, uint8_t led)
{
    uint32_t counts;

    // stop timer while changing settings
    T3CONbits.TON = 0;

    LED0 = 0;
    LED1 = 0;

    // remember which LED to blink
    blink_led = led;

    // convert milliseconds to Timer3 counts
    counts = ((uint32_t)ms * 15625UL) / 1000UL;

    // keep count in valid range
    if(counts < 1)
        counts = 1;

    if(counts > 65535)
        counts = 65535;

    // set new Timer3 period
    PR3 = (uint16_t)(counts - 1);

    TMR3 = 0;
    IFS0bits.T3IF = 0;

    // start selected LED on
    if(blink_led == BLINK_LED0)
        LED0 = 1;
    else if(blink_led == BLINK_LED1)
        LED1 = 1;

    // start Timer3
    T3CONbits.TON = 1;
}


// stops blinking and turns LEDs off
void stop_blink(void)
{
    T3CONbits.TON = 0;

    TMR3 = 0;
    blink_led = 0;

    LED0 = 0;
    LED1 = 0;
    LED2 = 0;

    IFS0bits.T3IF = 0;
}


// Timer2 ISR runs once per delay tick
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
    // clear interrupt flag
    IFS0bits.T2IF = 0;

    if(delay_remaining > 0)
        delay_remaining--;

    // delay finished
    if(delay_remaining == 0)
    {
        T2CONbits.TON = 0;
        delay_done = 1;
    }
}


// Timer3 ISR toggles the selected LED
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    // clear interrupt flag
    IFS0bits.T3IF = 0;

    if(blink_led == BLINK_LED0)
        LED0 ^= 1;

    else if(blink_led == BLINK_LED1)
        LED1 ^= 1;
}