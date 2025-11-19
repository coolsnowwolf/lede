#ifndef __ASM_BYTEORDER_H
#define __ASM_BYTEORDER_H

#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN
#include <linux/byteorder/little_endian.h>
#else
#include <linux/byteorder/big_endian.h>
#endif

#endif
