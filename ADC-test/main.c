#include <msp430.h>

// Define calibration constants for the temperature sensor
#define CALADC_15V_30C *((unsigned int *)0x1A1A) // Calibration at 30°C
#define CALADC_15V_85C *((unsigned int *)0x1A1C) // Calibration at 85°C

void configureADCForTempSensor(void) {
    // Enable internal reference voltage (1.5V)
    REFCTL0 = REFON | REFVSEL_0;             // Turn on 1.5V reference
    __delay_cycles(400);                     // Wait for reference to stabilize

    // Configure ADC12_B
    ADC12CTL0 = ADC12SHT0_3 | ADC12ON;       // 32 ADC clock cycles sample time, ADC ON
    ADC12CTL1 = ADC12SHP;                    // Use sampling timer
    ADC12CTL2 = ADC12RES_2;                  // 12-bit resolution
    ADC12CTL3 = ADC12TCMAP;                  // Enable temperature sensor mapping
    ADC12MCTL0 = ADC12INCH_30 | ADC12VRSEL_1; // Input channel for temp sensor, VREF=1.5V
    ADC12CTL0 |= ADC12ENC;                   // Enable ADC conversions
}

unsigned int readTemperatureSensor(void) {
    ADC12CTL0 |= ADC12SC;                    // Start conversion
    while (!(ADC12IFGR0 & BIT0));            // Wait for conversion to complete
    return ADC12MEM0;                        // Return ADC value
}

float convertADCToCelsius(unsigned int adcValue) {
    // Use calibration constants to convert ADC value to temperature in Celsius
    int diff = 0;
    diff = (int)(adcValue - CALADC_15V_30C);
    return ((float)diff * (85.0 - 30.0) /
            (CALADC_15V_85C - CALADC_15V_30C)) + 30.0;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;                // Stop watchdog timer

    configureADCForTempSensor();             // Configure ADC for temp sensor

    volatile unsigned int cal30 = CALADC_15V_30C;
    volatile unsigned int cal85 = CALADC_15V_85C;

    while (1) {
        volatile unsigned int adcValue = readTemperatureSensor(); // Read ADC value
        volatile float temperature = convertADCToCelsius(adcValue); // Convert to Celsius

        // Debugging: Place a breakpoint here to inspect `temperature` and `adcValue`
        __no_operation();
    }
}
