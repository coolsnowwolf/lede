/*
 * rt-loader header
 * (c) 2025 Markus Stockhausen
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#define UIMAGE_HDR_SIZE		64
#define UIMAGE_OS_LINUX		5
#define UIMAGE_COMP_NONE	0
#define UIMAGE_COMP_LZMA	3

#define KSEG0			0x80000000
#define STACK_SIZE		0x10000
#define HEAP_SIZE		0x40000
#define MEMORY_ALIGNMENT	32

#define CP0_COUNT		$9
#define CP0_COMPARE		$11
#define CP0_STATUS		$12
#define CP0_CAUSE		$13
#define CP0_WATCHLO		$18
#define CP0_WATCHHI		$19

#define printf(fmt, ...)	npf_pprintf(board_putchar, NULL, fmt, ##__VA_ARGS__)
#define snprintf		npf_snprintf

#endif  // _GLOBALS_H_
