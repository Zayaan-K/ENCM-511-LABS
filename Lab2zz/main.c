/*
 * File:   main.c
 * Author: ENTER GROUP MEMBER NAME(S) HERE
 *
 * Created FOR ENCM 511
 * PLEASE ADD DATE CREATED HERE: 2025-XX-XX
 * 
 * FAILURE TO UPDATE THIS HEADER WITH YOUR GROUP MEMBER NAMES
 * MAY RESULT IN PENALTIES
 */

// FSEC
#pragma config BWRP = OFF    //Boot Segment Write-Protect bit->Boot Segment may be written
#pragma config BSS = DISABLED    //Boot Segment Code-Protect Level bits->No Protection (other than BWRP)
#pragma config BSEN = OFF    //Boot Segment Control bit->No Boot Segment
#pragma config GWRP = OFF    //General Segment Write-Protect bit->General Segment may be written
#pragma config GSS = DISABLED    //General Segment Code-Protect Level bits->No Protection (other than GWRP)
#pragma config CWRP = OFF    //Configuration Segment Write-Protect bit->Configuration Segment may be written
#pragma config CSS = DISABLED    //Configuration Segment Code-Protect Level bits->No Protection (other than CWRP)
#pragma config AIVTDIS = OFF    //Alternate Interrupt Vector Table bit->Disabled AIVT

// FBSLIM
#pragma config BSLIM = 8191    //Boot Segment Flash Page Address Limit bits->8191

// FOSCSEL
#pragma config FNOSC = FRC    //Oscillator Source Selection->Internal Fast RC (FRC)
#pragma config PLLMODE = PLL96DIV2    //PLL Mode Selection->96 MHz PLL. Oscillator input is divided by 2 (8 MHz input)
#pragma config IESO = OFF    //Two-speed Oscillator Start-up Enable bit->Start up with user-selected oscillator source

// FOSC
#pragma config POSCMD = NONE    //Primary Oscillator Mode Select bits->Primary Oscillator disabled
#pragma config OSCIOFCN = ON    //OSC2 Pin Function bit->OSC2 is general purpose digital I/O pin
#pragma config SOSCSEL = OFF    //SOSC Power Selection Configuration bits->Digital (SCLKI) mode
#pragma config PLLSS = PLL_FRC    //PLL Secondary Selection Configuration bit->PLL is fed by the on-chip Fast RC (FRC) oscillator
#pragma config IOL1WAY = ON    //Peripheral pin select configuration bit->Allow only one reconfiguration
#pragma config FCKSM = CSECMD    //Clock Switching Mode bits->Clock switching is enabled,Fail-safe Clock Monitor is disabled

// FWDT
#pragma config WDTPS = PS32768    //Watchdog Timer Postscaler bits->1:32768
#pragma config FWPSA = PR128    //Watchdog Timer Prescaler bit->1:128
#pragma config FWDTEN = ON_SWDTEN    //Watchdog Timer Enable bits->WDT Enabled/Disabled (controlled using SWDTEN bit)
#pragma config WINDIS = OFF    //Watchdog Timer Window Enable bit->Watchdog Timer in Non-Window mode
#pragma config WDTWIN = WIN25    //Watchdog Timer Window Select bits->WDT Window is 25% of WDT period
#pragma config WDTCMX = WDTCLK    //WDT MUX Source Select bits->WDT clock source is determined by the WDTCLK Configuration bits
#pragma config WDTCLK = LPRC    //WDT Clock Source Select bits->WDT uses LPRC

// FPOR
#pragma config BOREN = ON    //Brown Out Enable bit->Brown Out Enable Bit
#pragma config LPCFG = OFF    //Low power regulator control->No Retention Sleep
#pragma config DNVPEN = ENABLE    //Downside Voltage Protection Enable bit->Downside protection enabled using ZPBOR when BOR is inactive

// FICD
#pragma config ICS = PGD1    //ICD Communication Channel Select bits->Communicate on PGEC1 and PGED1
#pragma config JTAGEN = OFF    //JTAG Enable bit->JTAG is disabled

// FDEVOPT1
#pragma config ALTCMPI = DISABLE    //Alternate Comparator Input Enable bit->C1INC, C2INC, and C3INC are on their standard pin locations
#pragma config TMPRPIN = OFF    //Tamper Pin Enable bit->TMPRN pin function is disabled
#pragma config SOSCHP = ON    //SOSC High Power Enable bit (valid only when SOSCSEL = 1->Enable SOSC high power mode (default)
#pragma config ALTI2C1 = ALTI2CEN    //Alternate I2C pin Location->SDA1 and SCL1 on RB9 and RB8


#include "xc.h"
#include "IOinit.h"
#include "timerDelay.h"



#define TICKS_PB0_ONLY  2u   // LED0: 0.25 s on, 0.25 s off 
#define TICKS_PB0_PB1   4u   // LED0: 0.5 s on, 0.5 s off 
#define INITIAL_PB2_RATE_TICKS 32u // LED1: 4 s on, 4 s off 

enum BlinkMode {
    MODE_OFF,
    MODE_PB0,
    MODE_PB1,
    MODE_PB0_PB1
};


static uint16_t takeTimer2Ticks(void)
{
    uint16_t ticks;

    IEC0bits.T2IE = 0;
    ticks = timer2Ticks;
    timer2Ticks = 0;
    IEC0bits.T2IE = 1;

    return ticks;
}


static uint8_t takePb2Click(void)
{
    uint8_t clicked;

    IEC1bits.IOCIE = 0;
    clicked = pb2Click;
    pb2Click = 0;
    IEC1bits.IOCIE = 1;

    return clicked;
}

int main(void) {
    enum BlinkMode mode = MODE_OFF;
    uint8_t pb2RateTicks = INITIAL_PB2_RATE_TICKS;
    uint8_t elapsedTicks = 0;

    ANSELA = 0x0000;
    ANSELB = 0x0000;

    initLeds();
    initButtons();
    initTimer2(3, 0, 1, 2, 1, 1952);
    initISR();

    while (1) {
        enum BlinkMode newMode;
        uint8_t buttons;
        uint8_t targetTicks;
        uint16_t pendingTicks;


        buttons = (pb0Down ? 1u : 0u) | (pb1Down ? 2u : 0u);

        if (takePb2Click() && buttons == 0u) {

            delay_ms(25);
            if (!pb0Down && !pb1Down && !pb2Down &&
                PORTBbits.RB9 == 0) {
                pb2RateTicks = (pb2RateTicks == 1u)
                                   ? INITIAL_PB2_RATE_TICKS
                                   : (uint8_t)(pb2RateTicks >> 1);
            }
        }


        if (pb2Down) {
            newMode = MODE_OFF;
        } else {
            switch (buttons) {
                case 0:                 
                    newMode = MODE_OFF;
                    break;
                case 1:                
                    newMode = MODE_PB0;
                    break;
                case 2:                
                    newMode = MODE_PB1;
                    break;
                case 3:                
                    newMode = MODE_PB0_PB1;
                    break;
                default:
                    newMode = MODE_OFF;
                    break;
            }
        }

        pendingTicks = takeTimer2Ticks();

        if (newMode != mode) {
            mode = newMode;
            elapsedTicks = 0;
            LATBbits.LATB5 = (mode == MODE_PB0 || mode == MODE_PB0_PB1);
            LATBbits.LATB6 = (mode == MODE_PB1);
            pendingTicks = 0;  
        }

        if (mode == MODE_OFF) {
            LATBbits.LATB5 = 0;
            LATBbits.LATB6 = 0;
        } else {
            targetTicks = (mode == MODE_PB0) ? TICKS_PB0_ONLY : (mode == MODE_PB0_PB1) ? TICKS_PB0_PB1 : pb2RateTicks;

            while (pendingTicks != 0u) {
                --pendingTicks;
                if (++elapsedTicks >= targetTicks) {
                    elapsedTicks = 0;
                    if (mode == MODE_PB1)
                        LATBbits.LATB6 ^= 1; 
                    else
                        LATBbits.LATB5 ^= 1; 
                }
            }
        }

        Idle();
    }
}
