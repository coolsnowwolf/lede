/******************************************************************
 * $File:   crc32.h
 *
 * $Author: Hua Shao
 * $Date:   Oct, 2014
 *
 ******************************************************************/

#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>

uint32_t crc32(uint32_t crc, const uint8_t *buf, uint32_t len);

#endif /* CRC32_H */
