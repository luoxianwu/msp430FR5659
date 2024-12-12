#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <stdint.h>

#define ASSERT(x) if(!(x)) printf("Assert failed: %s, File: %s, Line: %d\n", #x, __FILE__, __LINE__)
#define LOG(str) printf("%s\n", str)
#define LOG_H(v) printf("%s : 0x%x\n", #v, v)
#define LOG_D(v) printf("%s : %d\n",   #v, v)

uint16_t swap_bytes16( uint16_t x );
uint32_t swap_bytes32(uint32_t x);

uint32_t calculate_crc32_bitwise(const unsigned char *data, size_t length);

#endif //__UTIL_H__

