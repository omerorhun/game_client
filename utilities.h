#ifndef _UTILITIES_H_
#define _UTILITIES_H_

#include <stdint.h>

#define B0(x)               (uint8_t)((x) & 0xFF)
#define B1(x)               (uint8_t)(((x)>>8) & 0xFF)
#define B2(x)               (uint8_t)(((x)>>16) & 0xFF)
#define B3(x)               (uint8_t)(((x)>>24) & 0xFF)

#define GET_32(x)           (uint32_t)((*(x+ 0) << 0) & 0xFF) | \
                            ((*(x + 1) << 8) & 0xFF00) | \
                            ((*(x + 2) << 16) & 0xFF0000) | \
                            ((*(x + 3) << 24) & 0xFF000000)

#define GET_64(x)           (uint64_t)(((*(uint64_t *)(x + 0) << 0) & 0xFF) | \
                            ((*(uint64_t *)(x + 1) << 8) & 0xFF00) | \
                            ((*(uint64_t *)(x + 2) << 16) & 0xFF0000) | \
                            ((*(uint64_t *)(x + 3) << 24) & 0xFF000000) | \
                            ((*(uint64_t *)(x + 4) << 32) & 0xFF00000000) | \
                            ((*(uint64_t *)(x + 5) << 40) & 0xFF0000000000) | \
                            ((*(uint64_t *)(x + 6) << 48) & 0xFF000000000000) | \
                            ((*(uint64_t *)(x + 7) << 56) & 0xFF00000000000000))
                            

void print_hex(const char *header, char *buffer, uint16_t len);
uint16_t gen_crc16(const uint8_t *data, uint16_t size);

#endif // _UTILITIES_H_