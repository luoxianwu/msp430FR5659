#ifndef __UART_H__
#define __UART_H__
#include <stdint.h>
#include <stdbool.h>
#include "ring_buffer.h"

extern RingBuffer txBuffer;
extern RingBuffer rxBuffer;

void Init_UART();
bool uart_putc(uint8_t data);
uint8_t uart_puts(uint8_t *data, uint8_t length );
 
bool uart_getc(uint8_t *data);

#endif //__UART_H__

