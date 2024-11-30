#include <msp430.h>

void configureADCForPin(void) {
    // Configure P1.2 as ADC input
    P1SEL1 |= BIT2;  // Set P1.2 as analog input
    P1SEL0 |= BIT2;

    // Configure ADC12_B
    ADC12CTL0 = ADC12SHT0_3 | ADC12ON;       // 32 ADC clock cycles sample time, ADC ON
    ADC12CTL1 = ADC12SHP;                    // Use sampling timer
    ADC12CTL2 = ADC12RES_2;                  // 12-bit resolution
    ADC12MCTL0 = ADC12INCH_2;                // Input channel A2 (P1.2), VREF = Vcc
    ADC12CTL0 |= ADC12ENC;                   // Enable ADC conversions
}

unsigned int readADCValue(void) {
    ADC12CTL0 |= ADC12SC;                // Start conversion
    while (!(ADC12IFGR0 & BIT0));        // Wait for conversion to complete
    return ADC12MEM0;                    // Return ADC value
}

float convertADCToVoltage(unsigned int adcValue) {
    return (adcValue * 3.3) / 4095.0;    // Convert ADC value to voltage using Vcc = 3.3V
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;            // Stop watchdog timer

    configureADCForPin();                // Configure ADC for P1.2 (A2)

    while (1) {
        volatile unsigned int adcValue = readADCValue(); // Read ADC value directly
        volatile float voltage = convertADCToVoltage(adcValue); // Convert to voltage

        // Debugging: Place a breakpoint here to inspect `adcValue` and `voltage`
        __no_operation();
    }
}
