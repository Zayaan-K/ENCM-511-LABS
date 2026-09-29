#include "xc.h"
#include <stdint.h>
#include "timerDelay.h"

#ifndef DELAY_FCY_HZ
#define DELAY_FCY_HZ 4000000UL
#endif

static uint8_t timer1Ready = 0;

static void initDelayTimer(void)
{
    if (timer1Ready)
        return;

    T1CONbits.TON = 0;
    IEC0bits.T1IE = 0;       
    T1CONbits.TCKPS = 0;    
    T1CONbits.TCS = 0;      
    T1CONbits.TGATE = 0;
    T1CONbits.TSIDL = 0;
    PR1 = 0xFFFFu;         
    TMR1 = 0;
    IFS0bits.T1IF = 0;
    T1CONbits.TON = 1;

    timer1Ready = 1;
}

void delay_us(uint16_t us)
{
    initDelayTimer();

    while (us != 0u)
    {

        uint16_t chunk = (us > 1000u) ? 1000u : us;
        uint16_t start = TMR1;
        uint16_t ticks = (uint16_t)(
            (DELAY_FCY_HZ / 1000000UL) * chunk +
            ((DELAY_FCY_HZ % 1000000UL) * chunk + 999999UL) / 1000000UL);


        while ((uint16_t)(TMR1 - start) < ticks){}

        us -= chunk;
    }
}

void delay_ms(uint16_t ms)
{
    while (ms != 0u)
    {
        delay_us(1000u);
        --ms;
    }
}
