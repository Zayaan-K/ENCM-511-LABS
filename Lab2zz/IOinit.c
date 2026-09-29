#include "xc.h"
#include "IOinit.h"
#include <stdint.h>

void initTimer2(uint8_t prescaler, uint8_t clk, uint8_t runInIdle, uint8_t priority, uint8_t interruptEnable, uint16_t period)
{
    T2CONbits.TON = 0;                 
    IEC0bits.T2IE = 0;                
    T2CONbits.T32 = 0;                
    T2CONbits.TCKPS = prescaler; 
    T2CONbits.TCS = clk;      
    T2CONbits.TSIDL = !runInIdle;     
    T2CONbits.TGATE = 0;
    IPC1bits.T2IP = priority;
    PR2 = period;
    TMR2 = 0;
    IFS0bits.T2IF = 0;                
    IEC0bits.T2IE = interruptEnable;
    T2CONbits.TON = 1;                
}

void initTimer3(uint8_t prescaler, uint8_t clk, uint8_t runInIdle, uint8_t priority, uint8_t interruptEnable, uint16_t period)
{
    T3CONbits.TON = 0;
    IEC0bits.T3IE = 0;
    T3CONbits.TCKPS = prescaler;
    T3CONbits.TCS = clk;
    T3CONbits.TSIDL = !runInIdle;
    T3CONbits.TGATE = 0;
    IPC2bits.T3IP = priority;
    PR3 = period;
    TMR3 = 0;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = interruptEnable;
    T3CONbits.TON = 1;
}

void initButtons(void)
{
    ANSELA = 0x0000;
    ANSELB = 0x0000;

    TRISAbits.TRISA4 = 1;     
    TRISBbits.TRISB8 = 1;     
    TRISBbits.TRISB9 = 1;    

    PADCONbits.IOCON = 0;    

    IOCPA |= (1u << 4);   
    IOCNA |= (1u << 4);    

    IOCPB |= (1u << 8) | (1u << 9);  
    IOCNB |= (1u << 8) | (1u << 9);  

    pb0Down = (PORTAbits.RA4 == 1);
    pb1Down = (PORTBbits.RB8 == 1);
    pb2Down = (PORTBbits.RB9 == 1);
}

void initLeds(void)
{
    LATBbits.LATB5 = 0;
    LATBbits.LATB6 = 0;

    TRISBbits.TRISB5 = 0;    
    TRISBbits.TRISB6 = 0;  
}

void initISR(void)
{
    IEC1bits.IOCIE = 0;

    IOCFA &= ~(1u << 4);                
    IOCFB &= ~((1u << 8) | (1u << 9));   
    IFS1bits.IOCIF = 0;

    IPC4bits.IOCIP = 3;
    PADCONbits.IOCON = 1;
    IEC1bits.IOCIE = 1;
}



volatile uint16_t timer2Ticks = 0;
volatile uint16_t timer3Ticks = 0;

volatile uint8_t pb0Down = 0;
volatile uint8_t pb1Down = 0;
volatile uint8_t pb2Down = 0;
volatile uint8_t pb2Click = 0;
volatile uint8_t pb2ClickArmed = 0;

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    timer2Ticks++;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;
    timer3Ticks++;
}

void __attribute__((interrupt, no_auto_psv)) _IOCInterrupt(void)
{
    if (IOCFA & (1u << 4))            
    {
        pb0Down = (PORTAbits.RA4 == 1);
        if (pb0Down && pb2Down)
            pb2ClickArmed = 0;
        IOCFA &= ~(1u << 4);
    }

    if (IOCFB & (1u << 8))             
    {
        pb1Down = (PORTBbits.RB8 == 1);
        if (pb1Down && pb2Down)
            pb2ClickArmed = 0;
        IOCFB &= ~(1u << 8);
    }

    if (IOCFB & (1u << 9))            
    {
        uint8_t nowDown = (PORTBbits.RB9 == 1);

        if (nowDown && !pb2Down) {
            pb2ClickArmed = !pb0Down && !pb1Down;
        } else if (!nowDown && pb2Down) {
            if (pb2ClickArmed && !pb0Down && !pb1Down)
                pb2Click = 1;         

            pb2ClickArmed = 0;
        }

        pb2Down = nowDown;
        IOCFB &= ~(1u << 9);
    }

    IFS1bits.IOCIF = 0;
}
