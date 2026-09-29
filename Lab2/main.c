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


#include <xc.h>
#include <stdint.h>
#include "io.h"
#include "timer.h"

// possible controller states
typedef enum
{
    STATE_OFF,
    STATE_PB0,
    STATE_PB0_PB1,
    STATE_PB1
} State;


// sets LED behaviour for each state
static void set_state(State state, uint16_t pb2_rate)
{
    switch(state)
    {
        // PB0 only
        case STATE_PB0:
            start_blink(250, BLINK_LED0);
            break;

        // PB0 and PB1 together
        case STATE_PB0_PB1:
            start_blink(500, BLINK_LED0);
            break;

        // PB1 only
        case STATE_PB1:
            start_blink(pb2_rate, BLINK_LED1);
            break;

        // no buttons
        default:
            stop_blink();
            break;
    }
}


int main(void)
{
    // current and next FSM states
    State state = STATE_OFF;
    State next_state;

    // remembered PB2 blink rate
    uint16_t pb2_rate = 4000;

    // tracks PB2 rate changes
    uint8_t rate_changed;

    // initialize IO and timers
    IOinit();
    Timer2_init();
    Timer3_init();

    stop_blink();

    while(1)
    {
        // sleep until an interrupt occurs
        if(!button_event)
            Idle();

        // process button changes
        if(button_event)
        {
            button_event = 0;

            // short settling time after a press
            if(press_event)
            {
                press_event = 0;
                delay_ms(20);
            }

            rate_changed = 0;

            // PB2 was pressed and released
            if(pb2_clicked)
            {
                pb2_clicked = 0;

                delay_ms(20);

                if(!pb2_pressed)
                {
                    // cycle from 125 ms back to 4 seconds
                    if(pb2_rate == 125)
                        pb2_rate = 4000;
                    else
                        pb2_rate /= 2;

                    rate_changed = 1;
                }
            }

            button_event = 0;
            press_event = 0;

            // determine next FSM state
            if(pb0_pressed && pb1_pressed)
                next_state = STATE_PB0_PB1;

            else if(pb0_pressed)
                next_state = STATE_PB0;

            else if(pb1_pressed)
                next_state = STATE_PB1;

            else
                next_state = STATE_OFF;

            // update outputs when state changes
            if(next_state != state ||
               (next_state == STATE_PB1 && rate_changed))
            {
                state = next_state;
                set_state(state, pb2_rate);
            }
        }
    }

    return 0;
}