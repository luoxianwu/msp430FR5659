#include <stdint.h>
#include <stdbool.h>
#include "ring_buffer.h"

void RingBuffer_Init(RingBuffer *ringBuffer) {
    ringBuffer->head = 0;
    ringBuffer->tail = 0;
}

bool RingBuffer_IsFull(const RingBuffer *ringBuffer) {
    return ((ringBuffer->head + 1) % BUFFER_SIZE) == ringBuffer->tail;
}

bool RingBuffer_IsEmpty(const RingBuffer *ringBuffer) {
    return ringBuffer->head == ringBuffer->tail;
}

bool RingBuffer_Write(RingBuffer *ringBuffer, uint8_t data) {
    if (RingBuffer_IsFull(ringBuffer)) {
        return false;  // Buffer is full, cannot write
    }
    ringBuffer->buffer[ringBuffer->head] = data;
    ringBuffer->head = (ringBuffer->head + 1) % BUFFER_SIZE;
    return true;
}

bool RingBuffer_Read(RingBuffer *ringBuffer, uint8_t *data) {
    if (RingBuffer_IsEmpty(ringBuffer)) {
        return false;  // Buffer is empty, cannot read
    }
    *data = ringBuffer->buffer[ringBuffer->tail];
    ringBuffer->tail = (ringBuffer->tail + 1) % BUFFER_SIZE;
    return true;
}
