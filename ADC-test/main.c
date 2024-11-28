#include <msp430.h>

void configureADC(void);
unsigned int readADC(unsigned int channel);

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;       // Disable GPIO high-impedance mode

    // Configure ADC
    configureADC();

    unsigned int p1_2_raw;
    volatile float p1_2_voltage;

    while (1) {
        // Read voltage on P1.2
        p1_2_raw = readADC(ADC12INCH_2); // P1.2 (Channel 2)
        p1_2_voltage = (p1_2_raw * 3.3) / 4096; // Assuming Vcc = 3.3V

        __no_operation(); // Breakpoint here to monitor `p1_2_voltage` in the debugger
    }
}

void configureADC(void) {
    // Configure P1.2 as ADC input
    P1SEL1 |= BIT2;    // Enable analog function on P1.2
    P1SEL0 |= BIT2;

    // Configure ADC12 module
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;    // Sampling time, ADC on
    ADC12CTL1 = ADC12SHP;                 // Use sampling timer
    ADC12CTL2 = ADC12RES_2;               // 12-bit resolution
    ADC12MCTL0 = ADC12VRSEL_0 | ADC12INCH_2; // Use Vcc as reference, Channel 2
    ADC12CTL0 |= ADC12ENC;                // Enable ADC conversions
}

unsigned int readADC(unsigned int channel) {
    ADC12MCTL0 = ADC12VRSEL_0 | channel;  // Use Vcc as reference, select channel
    ADC12CTL0 |= ADC12SC;                 // Start conversion
    while (!(ADC12IFGR0 & ADC12IFG0));    // Wait for conversion to complete
    return ADC12MEM0;                     // Return ADC value
}

