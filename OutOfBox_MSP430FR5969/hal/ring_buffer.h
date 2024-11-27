#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <stdint.h>
#include <stdbool.h>

#define BUFFER_SIZE 32  // Size of the ring buffer (must be a power of 2 for simplicity)

typedef struct {
    volatile uint8_t buffer[BUFFER_SIZE];  // Storage array
    volatile uint8_t head;                // Index for the next write
    volatile uint8_t tail;                // Index for the next read
} RingBuffer;


void RingBuffer_Init(RingBuffer *ringBuffer);

bool RingBuffer_IsFull(const RingBuffer *ringBuffer);

bool RingBuffer_IsEmpty(const RingBuffer *ringBuffer);

bool RingBuffer_Write(RingBuffer *ringBuffer, uint8_t data);

bool RingBuffer_Read(RingBuffer *ringBuffer, uint8_t *data);

#endif //__RING_BUFFER_H__
