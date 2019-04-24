/******************************************************************
 * $File:   flash.h
 *
 * $Author: Hua Shao
 * $Date:   Oct, 2014
 *
 ******************************************************************/

#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

uint32_t mtd_size(const char *name);
int32_t flash_read(char *path, char *buf, off_t from, size_t len);
int32_t flash_write(char *path, char *buf, off_t to, size_t len);

#endif /* FLASH_H */

