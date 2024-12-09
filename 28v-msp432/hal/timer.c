
#include <msp430.h>
#include <stdint.h>

void init_timer(void)
{
    // Configure Timer A0 for 5ms period using SMCLK (8MHz)
    TA0CCR0 = 40000;            // 8MHz/TA0CCR0 = 200HZ, T = 5ms
    TA0CCTL0 = CCIE;            // Enable CCR0 interrupt
    TA0CTL = TASSEL_2 |         // SMCLK source
             MC_1 |             // Up mode
             TACLR;             // Clear timer
}


// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0_ISR(void)
{
    static uint16_t count5ms = 0;
    if ((count5ms % 400) == 0) ADC12CTL0 |= ADC12SC;        // Start ADC conversion
    P2OUT ^= BIT2;               // Toggle P2.2
    count5ms++;
}
