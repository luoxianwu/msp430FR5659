#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "util.h"

#define ASSERT(x) if(!(x)) printf("Assert failed: %s, File: %s, Line: %d\n", #x, __FILE__, __LINE__)

#define PRIMARY_HEADER_SIZE 6
#define SECONDARY_HEADER_SIZE 10
#define MAX_DATA_SIZE 256

extern bool uart_getc( uint8_t * c);

typedef enum {
    STATE_IDLE,
    STATE_PRIMARY_HEADER,
    STATE_SECONDARY_HEADER,
    STATE_DATA,
    STATE_CRC
} CCSDS_State;

typedef struct {
    uint16_t version_type_flag_apid;
    uint16_t sequence_control;
    uint16_t data_length;
} __attribute__((packed)) CCSDS_PrimaryHeader;

typedef struct {
    uint8_t timing_info[6];
    uint8_t segment_number;
    uint8_t function_code;
    uint16_t address_code;
} __attribute__((packed)) CCSDS_SecondaryHeader;


typedef struct {
    CCSDS_PrimaryHeader primary;
    CCSDS_SecondaryHeader secondary;
    uint8_t data[MAX_DATA_SIZE];
    uint32_t crc;
} CCSDS_Packet;


CCSDS_Packet ccsds_pkt;
CCSDS_State state;
uint16_t bytes_received;
uint16_t data_length;

void ccsds_init(CCSDS_Packet *packet) {
    state = STATE_IDLE;
    bytes_received = 0;
    data_length = 0;
}


bool uart_get_ccsds_pkt() {
    uint8_t byte;
    
    if (!uart_getc(&byte)) {
        return false;  // No byte available
    }
    uint8_t *pkt_byte = (uint8_t *)&ccsds_pkt;
    switch (state) {
        case STATE_IDLE:
            pkt_byte[0] = byte;
            bytes_received = 1;
            state = STATE_PRIMARY_HEADER;
            break;

        case STATE_PRIMARY_HEADER:
            pkt_byte[bytes_received++] = byte;
            if (bytes_received == PRIMARY_HEADER_SIZE) {
                // Convert data length from big-endian
                data_length = pkt_byte[4] << 8 + pkt_byte[5] + 1; 
                state = STATE_SECONDARY_HEADER;
            }
            break;

        case STATE_SECONDARY_HEADER:
            pkt_byte[bytes_received++] = byte;
            if (bytes_received == PRIMARY_HEADER_SIZE + SECONDARY_HEADER_SIZE) {
                state = STATE_DATA;
            }
            break;

        case STATE_DATA:
            pkt_byte[bytes_received++] = byte;
            if (bytes_received == data_length - PRIMARY_HEADER_SIZE + SECONDARY_HEADER_SIZE) {
                return true;
            }
            break;

    }

    return false;  // Packet not complete yet
}

// Example usage
void main(void) {
    ASSERT( sizeof(CCSDS_PrimaryHeader) == PRIMARY_HEADER_SIZE );
    ASSERT( sizeof(CCSDS_SecondaryHeader) == STATE_SECONDARY_HEADER );
    while (1) {
        if (uart_get_ccsds_pkt()) {
            int function = ccsds_pkt.secondary.function_code;
            int address = ccsds_pkt.secondary.address_code;
/*
            ret = exe_cmd( function, address, ret_data, ret_len );
            pack_pkt( ret_data, ret_len );
            uart_puts( ccsds_pkt, pkt_size );
*/            
        }
    }
}