#include "util.h"

uint16_t swap_bytes16(uint16_t x) {
    return ((x & 0xFF) << 8) | ((x >> 8) & 0xFF);
}

uint32_t swap_bytes32(uint32_t x) {
    return ((x & 0xFF000000) >> 24) |
           ((x & 0x00FF0000) >> 8)  |
           ((x & 0x0000FF00) << 8)  |
           ((x & 0x000000FF) << 24);
}


uint32_t calculate_crc32_bitwise(const unsigned char *data, size_t length) {
    uint32_t crc = 0xFFFFFFFF; // Initial value
    uint32_t polynomial = 0xEDB88320; // Reversed polynomial used in IEEE 802.3
    size_t i;
    for (i = 0; i < length; i++) {
        int j;
        uint8_t byte = data[i];
        crc ^= byte;
        for (j = 0; j < 8; j++) { // Process each bit
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
    }
    return ~crc; // Final XOR
}
