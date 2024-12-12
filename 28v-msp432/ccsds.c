#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "util.h"
#include "ccsds.h"

#define ASSERT(x) if(!(x)) printf("Assert failed: %s, File: %s, Line: %d\n", #x, __FILE__, __LINE__)


extern bool uart_getc( uint8_t * c);

typedef enum {
    STATE_IDLE,
    STATE_SYNC,
    STATE_PRIMARY_HEADER,
    STATE_SECONDARY_HEADER,
    STATE_DATA,
    STATE_CRC
} CCSDS_State;


CCSDS_Packet ccsds_pkt_response = {
    .pkt_sync = {0x55, 0xAA},  // Packet sync bytes
    .primary = {
        .ver_type_sec_apid = 0x09, //TM type, with second header
        .apid_lo = 0x23,
        .segence_flag_number_h = 0xc0,      // flag : 2 bits signal packet, number_h 6 bits
        .sequence_number_l = 1,             // number_l
        .data_length_h = 0,                 // Will be set later based on actual data
        .data_length_l = 0
    },
    .secondary = {
        .timing_info = 0,
        .segment_number = 0,
        .function_code = 0,
        .address_code_h = 0,
        .address_code_l = 0
    },
    .data = {0},  // Initialize all data to zero
    .crc = 0  // Will be calculated later
};

CCSDS_Packet ccsds_pkt;
static uint8_t * const pkt_byte = (uint8_t *)&ccsds_pkt;
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

    pkt_byte[bytes_received++] = byte;
    switch (state) {

        case STATE_IDLE:
            state = STATE_SYNC;
            break;
        case STATE_SYNC:
            if( bytes_received == 2 ){
                if( pkt_byte[0] == 0x55 && pkt_byte[1] == 0xAA ){
                    state = STATE_PRIMARY_HEADER;
                    break;
                }
            }else{
                bytes_received = 1;
                pkt_byte[0] = pkt_byte[1]; //discard first byte
                break;
            }

        case STATE_PRIMARY_HEADER:
            if (bytes_received == SYNC_BYTES + PRIMARY_HEADER_SIZE) {
                // Convert data length from big-endian, data length is 1 less than actually data count
                data_length = (pkt_byte[bytes_received-2]<<8) + pkt_byte[bytes_received-1] + 1;
                state = STATE_SECONDARY_HEADER;
            }
            break;

        case STATE_SECONDARY_HEADER:
            if (bytes_received == SYNC_BYTES + PRIMARY_HEADER_SIZE + SECONDARY_HEADER_SIZE) {
                state = STATE_DATA;
            }
            break;

        case STATE_DATA:
            if (bytes_received == SYNC_BYTES + PRIMARY_HEADER_SIZE + data_length ) {
                state = STATE_IDLE;
                bytes_received = 0;
                data_length = 0;
                return true;
            }
            break;

    }

    return false;  // Packet not complete yet
}

/*
 * pack data to ccsds packet
 */

unsigned int ccdss_pack_data( void *data, size_t data_len ){
    uint8_t * pkt_bytes = (uint8_t*)&ccsds_pkt_response.primary;
    uint16_t length;
    uint32_t crc_cal_len;
    uint32_t crc32;
    if (data_len <= MAX_DATA_SIZE - 4) {
        memcpy( ccsds_pkt_response.data, x, data_len );
        // 1 less than actual length
        length = SECONDARY_HEADER_SIZE + data_len + 4 - 1;
        ccsds_pkt_response.primary.data_length_h = length >> 8;
        ccsds_pkt_response.primary.data_length_l = length & 0xff;
        //calculate crc, append to the end
        crc_cal_len = sizeof(ccsds_pkt_response.primary) + sizeof(ccsds_pkt_response.secondary) + data_len;
        crc32 = calculate_crc32_bitwise( (uint8_t*)&ccsds_pkt_response.primary, crc_cal_len );
        //append CRC to the ccsds packet
        pkt_bytes[crc_cal_len] =   (uint8_t)(crc32 >> 24);
        pkt_bytes[crc_cal_len+1] = (uint8_t)(crc32 >> 16);
        pkt_bytes[crc_cal_len+2] = (uint8_t)(crc32 >> 8);
        pkt_bytes[crc_cal_len+3] = (uint8_t)(crc32);

        //total bytes of the packet
        return sizeof(ccsds_pkt_response.pkt_sync) + sizeof(ccsds_pkt_response.primary) + length + 1;
    }
    return 0;
}

/*
 * add data to ccsds packet
 */
int ccsds_pkt_add( void* p_data, uint8_t size ){
    return 0;
}

#if 0
// Example usage
void main(void) {
    ASSERT( sizeof(CCSDS_PrimaryHeader) == PRIMARY_HEADER_SIZE );
    ASSERT( sizeof(CCSDS_SecondaryHeader) == STATE_SECONDARY_HEADER );
    while (1) {
        if (uart_get_ccsds_pkt()) {
            //int function = ccsds_pkt.secondary.function_code;
            //int address = ccsds_pkt.secondary.address_code;
/*
            ret = exe_cmd( function, address, ret_data, ret_len );
            pack_pkt( ret_data, ret_len );
            uart_puts( ccsds_pkt, pkt_size );
*/            
        }
    }
}
#endif

