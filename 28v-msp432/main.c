/******************************************************************************
 *
 * 11/28/2024
 * Luo, xianwu
 *
 ******************************************************************************/

#include "main.h"
#include "LiveTempMode.h"
#include "FRAMLogMode.h"
#include "driverlib.h"
#include "uart.h"
#include "ccsds.h"
#include "adc.h"

uint8_t RXData = 0;                               // UART Receive byte
int mode = 0;                                     // mode selection variable
int pingHost = 0;                                 // ping request from PC GUI
Calendar calendar;                                // Calendar used for RTC

#if defined(__IAR_SYSTEMS_ICC__)
#pragma location = 0x9000
__no_init uint16_t dataArray[12289];
#endif

//-----------------------------------------------------------------------------
int _system_pre_init(void)
{
    // Stop Watchdog timer
    WDT_A_hold(__MSP430_BASEADDRESS_WDT_A__);     // Stop WDT

    /*==================================*/
    /* Choose if segment initialization */
    /* should be done or not. */
    /* Return: 0 to omit initialization */
    /* 1 to run initialization */
    /*==================================*/
    return 1;
}

/*
 * main.c
 */
int main(void) {

    // Check if a wakeup from LPMx.5
    if (SYSRSTIV == SYSRSTIV_LPM5WU) {
    	// Button S2 pressed
        if (P1IFG & BIT1)
        {
        	// Clear P1.1 interrupt flag
        	GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN0);

        	// Exit FRAM Log Mode
        	mode = '0';

        	// Light up LED1 to indicate Exit from FRAM mode
        	Init_GPIO();
        	GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
        	__delay_cycles(600000);
        }
        else
        {
        	// Continue FRAM data logging
            mode = FRAM_LOG_MODE;
            Init_RTC();
            Init_GPIO();
            framLog();
        }
    }
    else
    {
    	Init_GPIO();
    	GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0); //green LED on

    	// Toggle LED1 and LED2 to indicate OutOfBox Demo start
    	int i;
    	for (i=0;i<10;i++)
    	{
            GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
            GPIO_toggleOutputOnPin(GPIO_PORT_P4, GPIO_PIN6); //red led toggle
            __delay_cycles(200000);
    	}
    }

    // Board initializations
    Init_GPIO();
    Init_Clock();
    Init_UART();
    init_ADC();


    // Select UART TXD on P2.0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0, GPIO_SECONDARY_MODULE_FUNCTION);
    // Select UART RXD on P2.1
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN1, GPIO_SECONDARY_MODULE_FUNCTION);




    uart_printf("---------%s%d-----------\n\r", "MSP", 430);
    // simple test uart
    uint8_t s[] = "1234567\n\r";
    while(1){
        uint16_t adc_ch[1];
        uart_puts(s, sizeof(s)-1);

        //delay and check input
        long long volatile cnt = 90000;
        while(cnt--) {
            if (uart_get_ccsds_pkt()) {
                uint8_t function = ccsds_pkt.secondary.function_code;
                int address = ccsds_pkt.secondary.address_code;
                if( function == 0 && address == 0 ) {
                    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

                }else
                if( function == 0 && address == 1 ) {
                    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
                    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
                }else
                if( function == 1 && address == 0 ) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

                }else
                if( function == 1 && address == 1 ) {
                    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
                    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
                }

    /*
                ret = exe_cmd( function, address, ret_data, ret_len );
                pack_pkt( ret_data, ret_len );
                uart_puts( ccsds_pkt, pkt_size );
    */
            }



        }

        adc_start();
        if( adc_read_all( adc_ch, 1) ) {
            uint32_t x = (uint32_t)adc_ch[0] * 36;
            x = x/4095;
            uart_printf("%d\n\r", x);
        }


    }


    // Main Loop
    while (1)
    {
    	// Acknowledge PC GUI's ping request
        if (pingHost)
            sendAckToPC();

        switch (mode)
        {
            case LIVE_TEMP_MODE:                  // Measures and transmits internal temperature data every 0.125 second
            	sendCalibrationConstants();
                liveTemp();
                break;
            case FRAM_LOG_MODE:                   // Logs internal temperature and battery voltage data to FRAM every 5 seconds
            	Init_RTC();
            	storeTimeStamp();
                dataArray[12288] = 0;

            	int i;
            	for (i=0;i<3;i++)
            	{
                    GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
                    __delay_cycles(300000);
                    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);
                    __delay_cycles(2000000);
            	}

                enterLPM35();
                framLog();
                break;
            case TRANSMIT_DATA_MODE:              // Transmits Logged FRAM data through UART to PC
            	sendCalibrationConstants();
            	sendTimeStamp();
                transmitFRAMData();
                break;
        }

        __bis_SR_register(LPM3_bits | GIE);       // Enter LPM3 and wait for PC commands
        __no_operation();
    }
}

/*
 * GPIO Initialization
 */
void Init_GPIO()
{
    // Set all GPIO pins to output low for low power
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_PJ, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7|GPIO_PIN8|GPIO_PIN9|GPIO_PIN10|GPIO_PIN11|GPIO_PIN12|GPIO_PIN13|GPIO_PIN14|GPIO_PIN15);

    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_PJ, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2|GPIO_PIN3|GPIO_PIN4|GPIO_PIN5|GPIO_PIN6|GPIO_PIN7|GPIO_PIN8|GPIO_PIN9|GPIO_PIN10|GPIO_PIN11|GPIO_PIN12|GPIO_PIN13|GPIO_PIN14|GPIO_PIN15);

	// Configure P2.0 - UCA0TXD and P2.1 - UCA0RXD
	GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
	GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN1, GPIO_SECONDARY_MODULE_FUNCTION);

    // Set PJ.4 and PJ.5 as Primary Module Function Input, LFXT.
    GPIO_setAsPeripheralModuleFunctionInputPin(
           GPIO_PORT_PJ,
           GPIO_PIN4 + GPIO_PIN5,
           GPIO_PRIMARY_MODULE_FUNCTION
           );

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();
}

/*
 * Clock System Initialization
 */
void Init_Clock()
{
    // Set DCO frequency to 8 MHz
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);
    //Set external clock frequency to 32.768 KHz
    CS_setExternalClockSource(32768, 0);
    //Set ACLK=LFXT
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
    // Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    // Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //Start XT1 with no time out
    CS_turnOnLFXT(CS_LFXT_DRIVE_0);
}


/*
 * Real Time Clock Initialization
 */
void Init_RTC()
{
    //Setup Current Time for Calendar
    calendar.Seconds    = 0x55;
    calendar.Minutes    = 0x30;
    calendar.Hours      = 0x04;
    calendar.DayOfWeek  = 0x01;
    calendar.DayOfMonth = 0x30;
    calendar.Month      = 0x04;
    calendar.Year       = 0x2014;

    // Initialize RTC with the specified Calendar above
    RTC_B_initCalendar(RTC_B_BASE,
                       &calendar,
                       RTC_B_FORMAT_BCD);

    RTC_B_setCalendarEvent(RTC_B_BASE,
    		               RTC_B_CALENDAREVENT_MINUTECHANGE
    		               );

    RTC_B_clearInterrupt(RTC_B_BASE,
                         RTC_B_TIME_EVENT_INTERRUPT
                         );

    RTC_B_enableInterrupt(RTC_B_BASE,
                          RTC_B_TIME_EVENT_INTERRUPT
                          );

    //Start RTC Clock
    RTC_B_startClock(RTC_B_BASE);
}

/*
 * Transmit Internal Temperature Sensor's Calibration constants through UART
 */
void sendCalibrationConstants()
{
	// Select UART TXD on P2.0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0, GPIO_SECONDARY_MODULE_FUNCTION);

    // Send Temp Sensor Calibration Data
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, CAL_ADC_12T30_H);
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, CAL_ADC_12T30_L);

    __delay_cycles(900000);

    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, CAL_ADC_12T85_H);
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, CAL_ADC_12T85_L);

    while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY));

    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
	GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
}

/*
 * Transmit FRAMLogMode starting TimeStamp through UART
 */
void sendTimeStamp()
{
	// Select UART TXD on P2.0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0, GPIO_SECONDARY_MODULE_FUNCTION);
    __delay_cycles(900000);
    int i;
    for (i=0;i<6;i++) {
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, timeStamp[i]);
        __delay_cycles(900000);
    }

    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
	GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
}

/*
 * Transmit 0xFF to acknowledge PC GUI's ping request
 */
void sendAckToPC()
{
	// Select UART TXD on P2.0
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0, GPIO_SECONDARY_MODULE_FUNCTION);

    // Send Ackknowledgement to Host PC
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 0xFF);
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 0xFF);

    while(EUSCI_A_UART_queryStatusFlags(EUSCI_A0_BASE, EUSCI_A_UART_BUSY));
    pingHost = 0;

    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
}

/*
 * Enter Low Power Mode 3.5
 */
void enterLPM35()
{
	// Configure button S2 (P1.1) interrupt
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN1, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN1);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1);

    // Request the disabling of the core voltage regulator when device enters
    // LPM3 (or LPM4) so that we can effectively enter LPM3.5 (or LPM4.5).
    PMM_turnOffRegulator();

    //Enter LPM3 mode with interrupts enabled
    __bis_SR_register(LPM4_bits + GIE);
    __no_operation();
}

bool rxOverflow = false;
/*
 * USCI_A0 Interrupt Service Routine that receives PC GUI's commands
 */

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    static int cnt = 0;
    cnt++;
    uint8_t c;
	int i;
    switch (__even_in_range(UCA0IV, USCI_UART_UCTXCPTIFG)) {
    	case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            i = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
            if (RingBuffer_Write( &rxBuffer, i ) == true) {

            }
            else {
                rxOverflow = true;
            }
            break;

        case USCI_UART_UCTXIFG:  // TX buffer ready
            if (RingBuffer_Read(&txBuffer, &c)) {
                UCA0TXBUF = c;  // Send the next character
            } else {
                // No more data to send, disable TX interrupt
                EUSCI_A_UART_disableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);
            }
            break;
        default:
            break;

        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
    }
}


/*
 * Timer0_A3 Interrupt Vector (TAIV) handler
 * Used to trigger ADC conversion every 0.125 seconds
 *
 */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    __bic_SR_register_on_exit(LPM3_bits); // Exit active CPU
}

/*
 * Port 1 interrupt service routine to handle S2 button press
 *
 */
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  P1IFG &= ~BIT1;                           // Clear P1.1 IFG
  mode = FRAM_LOG_MODE;
  __bic_SR_register_on_exit(LPM3_bits);     // Exit LPM3
}

void configureADC(void) {
    // Configure P1.2 as ADC input
    P1SEL1 |= BIT2;    // Enable analog function on P1.2
    P1SEL0 |= BIT2;

    // Configure ADC12 module
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;    // Sampling time, ADC on
    ADC12CTL1 = ADC12SHP;                 // Use sampling timer
    ADC12CTL2 = ADC12RES_2;               // 12-bit resolution
    ADC12MCTL0 = ADC12VRSEL_0 | ADC12INCH_2;      // P1.2 (Channel 2)
    ADC12MCTL1 = ADC12VRSEL_1 | ADC12INCH_10 | ADC12EOS; // Temperature sensor, EOS

    ADC12CTL0 |= ADC12ENC;                // Enable ADC conversions
}

unsigned int readADC(unsigned int channel) {
    ADC12MCTL0 = ADC12VRSEL_0 | channel;  // Use Vcc as reference, select channel
    ADC12CTL0 |= ADC12SC;                 // Start conversion
    while (!(ADC12IFGR0 & ADC12IFG0));    // Wait for conversion to complete
    return ADC12MEM0;                     // Return ADC value
}
