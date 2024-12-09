/*
 *
 *
*/
//******************************************************************************
//  MSP430FR59xx Demo - ADC12B, Sample A1, AVcc Ref, Set P1.0 if A1 > 0.5*AVcc
//
//   Description: A single sample is made on A1 with reference to AVcc.
//   Software sets ADC12BSC to start sample and conversion - ADC12BSC
//   automatically cleared at EOC. ADC12B internal oscillator times sample (16x)
//   and conversion. In Mainloop MSP430 waits in LPM0 to save power until ADC12B
//   conversion complete, ADC12_B_ISR will force exit from LPM0 in Mainloop on
//   reti. If A0 > 0.5*AVcc, P1.0 set, else reset. The full, correct handling of
//   and ADC12B interrupt is shown as well.
//
//
//                MSP430FR5969
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//        >---|P1.1/A1      P1.0|-->LED
//
//******************************************************************************
#include "driverlib.h"
#include "uart.h"
#include "adc.h"
#include "util.h"

#define CALADC12_12V_30C *((unsigned int *)0x1A1A)  // Temperature Sensor Calibration-30 C
#define CALADC12_12V_85C *((unsigned int *)0x1A1C)  // Temperature Sensor Calibration-85 C

// measure VCC on board, it's 3.6v, rather than 3.3v
#define AVCC (3600)   //mv


uint16_t adc_channels[ADC_TOTAL_CH];
bool     adc_refreshed = false;


unsigned int temp;
unsigned int batt;
volatile float temperatureDegC;
volatile float batteryVoltage;



void init_ADC2(void)
{
    // Configure P1.2 to P1.7 for analog inputs
    P1SEL0 |= BIT2 | BIT3 | BIT4 | BIT5 |BIT6 | BIT7 ;
    P1SEL1 |= BIT2 | BIT3 | BIT4 | BIT5 |BIT6 | BIT7 ;

    // Ensure P1.2 to P1.7 are set as inputs
    P1DIR &= ~(BIT2 | BIT3 | BIT4 | BIT5 |BIT6 | BIT7);

    // Initialize reference
    while(REFCTL0 & REFGENBUSY);  // Wait if ref generator busy
    REFCTL0 |= REFVSEL_0 + REFON; // Enable internal 1.2V reference

    // Initialize ADC12
    ADC12CTL0 &= ~ADC12ENC;      // Disable ADC12
    ADC12CTL0 = ADC12SHT0_8 + ADC12ON + ADC12MSC;  // Set sample time
    ADC12CTL1 = ADC12SHP + ADC12CONSEQ_1;          // Sample timer, sequence mode
    ADC12CTL3 = ADC12TCMAP | ADC12BATMAP;      // Enable temperature sensor

    // Configure channels
    ADC12MCTL0 = ADC12VRSEL_0 + ADC12INCH_2;              //  VR+ = AVCC, VR- = AVSS. A2 input,
    ADC12MCTL1 = ADC12VRSEL_0 + ADC12INCH_3;              //  VR+ = AVCC, VR- = AVSS. A3 input,
    ADC12MCTL2 = ADC12VRSEL_0 + ADC12INCH_4;              //  VR+ = AVCC, VR- = AVSS. A4 input,
    ADC12MCTL3 = ADC12VRSEL_0 + ADC12INCH_5;              //  VR+ = AVCC, VR- = AVSS. A5 input,
    ADC12MCTL4 = ADC12VRSEL_0 + ADC12INCH_6;              //  VR+ = AVCC, VR- = AVSS. A6 input,
    ADC12MCTL5 = ADC12VRSEL_0 + ADC12INCH_7;              //  VR+ = AVCC, VR- = AVSS. A7 input,

    ADC12MCTL6 = ADC12VRSEL_1 + ADC12INCH_30;             //  VR+ = VREF(1.2V) buffered, VR- = AVSS, Temperature sensor
    ADC12MCTL7 = ADC12VRSEL_0 + ADC12INCH_31 + ADC12EOS;  //  VR+ = AVCC, VR- = AVSS. Internal battery monitor, end of sequence

    //clear interrupt flags ( also used as convertion finish flag )
    ADC12IFGR0 = 0;

    ADC12IER0 = ADC12IE7;           // Enable interrupt for MEM7

    while(!(REFCTL0 & REFGENRDY));  // Wait for reference generator
    ADC12CTL0 |= ADC12ENC;       // Enable ADC
}

void init_ADC(void)
{

    //Set P1.1 as Ternary Module Function Output.
    /*

    * Select Port 1
    * Set Pin 1 to output Ternary Module Function, (A1, C1, VREF+, VeREF+).
    */
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN1,
        GPIO_TERNARY_MODULE_FUNCTION
    );

    /* Select Port 1
    * Set Pin 2 to output Ternary Module Function, (A1, C1, VREF+, VeREF+).
    */
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN2,
        GPIO_TERNARY_MODULE_FUNCTION
    );

    /*
     * Disable the GPIO power-on default high-impedance mode to activate
     * previously configured port settings
     */
    PMM_unlockLPM5();

    //Initialize the ADC12B Module
    /*
    * Base address of ADC12B Module
    * Use internal ADC12B bit as sample/hold signal to start conversion
    * USE MODOSC 5MHZ Digital Oscillator as clock source
    * Use default clock divider/pre-divider of 1
    * Not use internal channel
    */
    ADC12_B_initParam initParam = {0};
    initParam.sampleHoldSignalSourceSelect = ADC12_B_SAMPLEHOLDSOURCE_SC;
    initParam.clockSourceSelect = ADC12_B_CLOCKSOURCE_ADC12OSC;
    initParam.clockSourceDivider = ADC12_B_CLOCKDIVIDER_1;
    initParam.clockSourcePredivider = ADC12_B_CLOCKPREDIVIDER__1;
    initParam.internalChannelMap = ADC12_B_NOINTCH;
    ADC12_B_init(ADC12_B_BASE, &initParam);

    //Enable the ADC12B module
    ADC12_B_enable(ADC12_B_BASE);

    /*
    * Base address of ADC12B Module
    * For memory buffers 0-7 sample/hold for 64 clock cycles
    * For memory buffers 8-15 sample/hold for 4 clock cycles (default)
    * Disable Multiple Sampling
    */
    ADC12_B_setupSamplingTimer(ADC12_B_BASE,
      ADC12_B_CYCLEHOLD_16_CYCLES,
      ADC12_B_CYCLEHOLD_4_CYCLES,
      ADC12_B_MULTIPLESAMPLESDISABLE);

    //Configure Memory Buffer
    /*
    * Base address of the ADC12B Module
    * Configure memory buffer 0
    * Map input A1 to memory buffer 0
    * Vref+ = AVcc
    * Vref- = AVss
    * Memory buffer 0 is not the end of a sequence
    */
    ADC12_B_configureMemoryParam configureMemoryParam = {0};
    configureMemoryParam.memoryBufferControlIndex = ADC12_B_MEMORY_0;
    configureMemoryParam.inputSourceSelect = ADC12_B_INPUT_A1;
    configureMemoryParam.refVoltageSourceSelect = ADC12_B_VREFPOS_AVCC_VREFNEG_VSS;
    configureMemoryParam.endOfSequence = ADC12_B_NOTENDOFSEQUENCE;
    configureMemoryParam.windowComparatorSelect = ADC12_B_WINDOW_COMPARATOR_DISABLE;
    configureMemoryParam.differentialModeSelect = ADC12_B_DIFFERENTIAL_MODE_DISABLE;
    ADC12_B_configureMemory(ADC12_B_BASE, &configureMemoryParam);

    // Map input A2 to memory buffer 1
    configureMemoryParam.memoryBufferControlIndex = ADC12_B_MEMORY_1;
    configureMemoryParam.inputSourceSelect = ADC12_B_INPUT_A2;
    configureMemoryParam.endOfSequence = ADC12_B_ENDOFSEQUENCE;
    ADC12_B_configureMemory(ADC12_B_BASE, &configureMemoryParam);


    ADC12_B_clearInterrupt(ADC12_B_BASE,
        0,
        ADC12_B_IFG0
        );
/*
    //Enable memory buffer 0 interrupt
    ADC12_B_enableInterrupt(ADC12_B_BASE,
      ADC12_B_IE0,
      0,
      0);
*/


}

void adc_start(void){
    ADC12CTL0 |= ADC12SC;    // Start sampling/conversion
}


/*
 * when convertion finished, read all value from ADC memory to local variables
 */
bool adc_read_all_channels()
{
    int i = 0;
    uint16_t last_channel_flag = 1 << ADC_TOTAL_CH;

    // if all convert finish
    if( ADC12IFGR0 & last_channel_flag == 0 ){
        return false;
    }
    volatile uint16_t *p_adc_result = &ADC12MEM0;
    for( i = 0; i < ADC_TOTAL_CH; i++ ){
        adc_channels[i] = *p_adc_result++;
    }

    return true;

}

/*
 * when return true, read success
 */
bool adc_get_channels( uint16_t array[], uint8_t cnt )
{
    ASSERT( cnt >= ADC_TOTAL_CH );
    //disable ADC interrupt
    memcpy( array, adc_channels, ADC_TOTAL_CH );
    // enable ADC interrupt

    return true;

}



/*
 * convert adc_to_temperature
 */
uint16_t adc_board_temperature( uint16_t adc_value ){

    int32_t x = ((uint32_t)adc_value - CALADC12_12V_30C) * (85 - 30);
    return x/(CALADC12_12V_85C - CALADC12_12V_30C) + 30;

}


/*
 * calculate in 32 bits, avoid overflow
 */
uint16_t adc_to_voltage( uint16_t adc_value ){
    uint32_t x = (uint32_t)adc_value * AVCC;
    return x / 4095;

}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(ADC12_VECTOR)))
#endif
void ADC12_ISR(void)
{
  switch(__even_in_range(ADC12IV,12))
  {
    case  0: break;                         // Vector  0:  No interrupt
    case  2: break;                         // Vector  2:  ADC12BMEMx Overflow
    case  4: break;                         // Vector  4:  Conversion time overflow
    case  6: break;                         // Vector  6:  ADC12BHI
    case  8: break;                         // Vector  8:  ADC12BLO
    case 10: break;                         // Vector 10:  ADC12BIN
    case 12: break;                         // Vector 12:  ADC12BMEM0 Interrupt
    case 14: break;                         // Vector 14:  ADC12BMEM1
    case 16: break;                         // Vector 16:  ADC12BMEM2
    case 18: break;                         // Vector 18:  ADC12BMEM3
    case 20: break;                         // Vector 20:  ADC12BMEM4
    case 22: break;                         // Vector 22:  ADC12BMEM5
    case 24: break;                         // Vector 24:  ADC12BMEM6
    case 26:                                // Vector 26:  ADC12BMEM7
    {
        unsigned int i = 0;
        volatile uint16_t *p_adc_reg = &ADC12MEM0;
        for( i = 0; i < ADC_TOTAL_CH; i++ ) {
            adc_channels[i] = p_adc_reg[i];
        }
        P2OUT ^= BIT4;               // Toggle P2.4
        adc_refreshed = true;
    }
    case 28: break;                         // Vector 28:  ADC12BMEM8
    case 30: break;                         // Vector 30:  ADC12BMEM9
    case 32: break;                         // Vector 32:  ADC12BMEM10
    case 34: break;                         // Vector 34:  ADC12BMEM11
    case 36: break;                         // Vector 36:  ADC12BMEM12
    case 38: break;                         // Vector 38:  ADC12BMEM13
    case 40: break;                         // Vector 40:  ADC12BMEM14
    case 42: break;                         // Vector 42:  ADC12BMEM15
    case 44: break;                         // Vector 44:  ADC12BMEM16
    case 46: break;                         // Vector 46:  ADC12BMEM17
    case 48: break;                         // Vector 48:  ADC12BMEM18
    case 50: break;                         // Vector 50:  ADC12BMEM19
    case 52: break;                         // Vector 52:  ADC12BMEM20
    case 54: break;                         // Vector 54:  ADC12BMEM21
    case 56: break;                         // Vector 56:  ADC12BMEM22
    case 58: break;                         // Vector 58:  ADC12BMEM23
    case 60: break;                         // Vector 60:  ADC12BMEM24
    case 62: break;                         // Vector 62:  ADC12BMEM25
    case 64: break;                         // Vector 64:  ADC12BMEM26
    case 66: break;                         // Vector 66:  ADC12BMEM27
    case 68: break;                         // Vector 68:  ADC12BMEM28
    case 70: break;                         // Vector 70:  ADC12BMEM29
    case 72: break;                         // Vector 72:  ADC12BMEM30
    case 74: break;                         // Vector 74:  ADC12BMEM31
    case 76: break;                         // Vector 76:  ADC12BRDY
    default: break;
  }
}
