#include <msp430.h> 

#include <msp430.h>

#define CALADC12_12V_30C *((unsigned int *)0x1A1A)  // Temperature Sensor Calibration-30 C
#define CALADC12_12V_85C *((unsigned int *)0x1A1C)  // Temperature Sensor Calibration-85 C

// measure VCC on board, it's 3.6v, rather than 3.3v
#define AVCC (3.6f)

unsigned int temp;
unsigned int batt;
volatile float temperatureDegC;
volatile float batteryVoltage;

int main(void) {
    WDTCTL = WDTPW + WDTHOLD;    // Stop WDT

    // Initialize reference
    while(REFCTL0 & REFGENBUSY);  // Wait if ref generator busy
    REFCTL0 |= REFVSEL_0 + REFON; // Enable internal 1.2V reference

    // Initialize ADC12
    ADC12CTL0 &= ~ADC12ENC;      // Disable ADC12
    ADC12CTL0 = ADC12SHT0_8 + ADC12ON + ADC12MSC;  // Set sample time
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;          // Sample timer, sequence mode
    ADC12CTL3 = ADC12TCMAP | ADC12BATMAP;      // Enable temperature sensor

    // Configure channels
    ADC12MCTL0 = ADC12VRSEL_1 + ADC12INCH_30;      //  VR+ = VREF(1.2V) buffered, VR- = AVSS, Temperature sensor
    ADC12MCTL1 = ADC12VRSEL_0 + ADC12INCH_31 + ADC12EOS;  //  VR+ = AVCC, VR- = AVSS. Internal battery monitor, end of sequence

    ADC12IER0 = 0x003;           // Enable ADC12IFG0 and ADC12IFG1

    while(!(REFCTL0 & REFGENRDY));  // Wait for reference generator
    ADC12CTL0 |= ADC12ENC;       // Enable ADC

    while(1) {
        ADC12CTL0 |= ADC12SC;    // Start sampling/conversion
        __bis_SR_register(LPM0_bits + GIE);  // Enter LPM0 with interrupts
        __no_operation();

        // Calculate temperature
        temperatureDegC = (float)(((long)temp - CALADC12_12V_30C) * (85 - 30)) /
                         (CALADC12_12V_85C - CALADC12_12V_30C) + 30.0f;

        // Calculate battery voltage (adjust scale factor as needed)
        batteryVoltage = (float)batt * (AVCC / 4096.0f);

        __no_operation();         // Set breakpoint here
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(ADC12_VECTOR))) ADC12ISR(void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(ADC12IV, ADC12IV_ADC12RDYIFG)) {
        case ADC12IV_ADC12IFG0:        // ADC12MEM0 - Temperature
            temp = ADC12MEM0;
            break;
        case ADC12IV_ADC12IFG1:        // ADC12MEM1 - Battery
            batt = ADC12MEM1;
            __bic_SR_register_on_exit(LPM0_bits);  // Exit LPM0
            break;
        default: break;
    }
}
