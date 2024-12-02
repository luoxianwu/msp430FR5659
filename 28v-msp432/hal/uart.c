#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "ring_buffer.h"
#include "uart.h"
#include "eusci_a_uart.h"
RingBuffer txBuffer;
RingBuffer rxBuffer;

/*
 * UART Communication Initialization
 */
void Init_UART()
{
    RingBuffer_Init(&txBuffer);
    RingBuffer_Init(&rxBuffer);

    // Configure UART
    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    
    /* 9600 
    param.clockPrescalar = 52;
    param.firstModReg = 1;
    param.secondModReg = 0x49;
    */

    /* 115200   */
    param.clockPrescalar = 4;
    param.firstModReg = 5;
    param.secondModReg = 0x55;

    param.parity = EUSCI_A_UART_NO_PARITY;
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    param.uartMode = EUSCI_A_UART_MODE;
    param.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

    if(STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param))
        return;

    EUSCI_A_UART_enable(EUSCI_A0_BASE);

    EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
                                EUSCI_A_UART_RECEIVE_INTERRUPT);

    // Enable USCI_A0 RX interrupt
    EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
                                 EUSCI_A_UART_RECEIVE_INTERRUPT); // Enable interrupt

    // Enable globale interrupt
    __enable_interrupt();
}



bool uart_putc(uint8_t data) {
    uint16_t volatile tc_int_en = EUSCI_A_UART_getInterruptEnableStatus(EUSCI_A0_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);
    if (tc_int_en)  {
        //tx interrupt is enabled, meaning tx is in progress, write data to ringbuffer
        if (RingBuffer_Write(&txBuffer, data)) {
            return true;  // Success
        } else {
            return false;  // Failure: TX buffer is full
        }        
    }else {
        //tx interrupt is disabled, meaning this is the start of tx, write uart txBuf directly and 
        // enable tx interrupt. both action together will cause Tx ISR
        HWREG16(EUSCI_A0_BASE + OFS_UCAxTXBUF) = data; // write data first, it's important!
        EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_TRANSMIT_INTERRUPT);
        
        return true;  // Success
    }
}

uint8_t uart_puts(uint8_t *data, uint8_t length ) {
    int i;
    for (i = 0; i < length; i++) {
        if (uart_putc(data[i]) == false) {
            return i;
        }
    }
    return length;

}

/*
* if there is received data, return true.
*/
bool uart_getc(uint8_t *data) {
    // Try reading data from the RX ring buffer
    if (RingBuffer_Read(&rxBuffer, data)) {
        return true;  // Success
    } else {
        return false;  // Failure: RX buffer is empty
    }
}

/*
* get a ccsds packet, verify crc. return size of the packet
*/
uint16_t uart_get_pkt( ){
    return 128;
}




// UART printf implementation
void uart_printf(const char *format, ...) {
    char buffer[128]; // Buffer to hold the formatted string
    va_list args;     // Variable argument list

    // Initialize the argument list
    va_start(args, format);

    // Format the string into the buffer
    vsnprintf(buffer, sizeof(buffer), format, args);

    // Clean up the argument list
    va_end(args);

    // Send the formatted string over UART
    uart_puts((uint8_t*)buffer, strlen(buffer));
}
