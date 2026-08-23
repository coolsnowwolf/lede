#ifndef _DRV_TYPES_H_
#define _DRV_TYPES_H_

#include <linux/types.h>

#ifdef UINT32
#undef UINT32
#endif
#ifdef UINT16
#undef UINT16
#endif
#ifdef UINT8
#undef UINT8
#endif

typedef unsigned int		UINT32 ;
typedef unsigned short		UINT16 ;
typedef unsigned char   	UINT8 ;
#define PACKING
typedef unsigned int FIELD;

#ifndef VPint
#define VPint			*(volatile unsigned int *)
#endif /* VPint */

#ifndef     TRUE
#define     TRUE    1
#endif
#ifndef     FALSE
#define     FALSE   0
#endif

#ifndef NACK
#define NACK 	0
#endif
#ifndef ACK
#define ACK 1
#endif

#endif /* _DRV_TYPES_H_ */

