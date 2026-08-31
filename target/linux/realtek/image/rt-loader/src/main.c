/*
 * rt-loader main program
 * (c) 2025 Markus Stockhausen
 *
 * This code was inspired by the OpenWrt lzma loader. Thanks to
 *
 * Copyright (C) 2004 Manuel Novoa III (mjn3@codepoet.org)
 * Copyright (C) 2005 Mineharu Takahara <mtakahar@yahoo.com>
 * Copyright (C) 2005 by Oleg I. Vdovikin <oleg@cs.msu.su>
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 */

#include <stdbool.h>
#include "board.h"
#include "globals.h"
#include "memory.h"

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS	1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS		0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS		0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS		0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS	0
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS	0
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS		0
#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"

extern void *_kernel_load_addr;
extern void *_kernel_data_addr;
extern int _kernel_data_size;
extern int _kernel_comp_type;
extern void *_my_load_addr;
extern int _my_load_size;
extern int _my_run_count;

extern int unlzma(unsigned char *buf, long in_len, long (*fill)(void*, unsigned long),
	   long (*flush)(void*, unsigned long), unsigned char *output,
	   long *outlen, long *posp, void(*error)(char *x));

typedef void (*entry_func_t)(unsigned long reg_a0, unsigned long reg_a1,
			     unsigned long reg_a2, unsigned long reg_a3);


static bool is_uimage(unsigned char *m)
{
	unsigned int data[UIMAGE_HDR_SIZE / sizeof(int)];
	unsigned int image_crc;

	/*
	 * The most basic way to find a uImage is to lookup the operating system
	 * opcode (for Linux it is 5). Then verify the header checksum. This is
	 * reasonably fast and all other magic value or constants can be avoided.
	 */

	if (m[28] != UIMAGE_OS_LINUX)
		return false;

	memcpy(data, m, UIMAGE_HDR_SIZE);
	image_crc = data[1];
	data[1] = 0;

	return image_crc == crc32(data, UIMAGE_HDR_SIZE);
}

void *relocate(void *src, int len)
{
	void *addr;
	unsigned int offs;

	/*
	 * Relocate to highest possible memory address. This is usually the RAM size
	 * minus some space for the heap and the stack pointer. As we do not have any
	 * highmem features limit this to 256MB.
	 */
	offs = (board_get_memory() - STACK_SIZE - HEAP_SIZE - len - 1024) & 0xfff0000;
	addr = (void *)KSEG0 + offs;

	printf("Relocate %d bytes from 0x%08x to 0x%08x\n", len, src, addr);

	memcpy(addr, src, len);
	flush_cache(addr, len);

	return addr;
}

void welcome(void)
{
	char system[80];

	board_get_system(system, sizeof(system));

	printf("\nrt-loader\n");
	printf("Running on %s SoC with %d MB\n", system, board_get_memory() >> 20);
}

void decompress_error(char *x)
{
	printf("%s\n", x);
}

void *decompress(void *out, void *in, int len)
{
	long outlen;
	int ret = 1;

	printf("Extract image with %d bytes from 0x%08x to 0x%08x ...\n", len, in, out);

	switch (_kernel_comp_type) {
	case UIMAGE_COMP_LZMA:
		ret = unlzma(in, len, 0, 0, out, &outlen, 0, decompress_error);
		break;
	case UIMAGE_COMP_NONE:
		memcpy(out, in, len);
		outlen = len;
		ret = 0;
		break;
	default:
		printf("Unknown uImage compression type %d\n", _kernel_comp_type);
		break;
	}

	if (ret)
		board_panic();

	printf("Final kernel size is %d bytes\n", outlen);
	flush_cache(out, outlen);

	return out;
}

void search_image(void **image_addr, int *image_size, void **load_addr)
{
	unsigned char *addr = *image_addr;

	*image_addr = NULL;
	for (int i = 0; i < 256 * 1024; i += 1, addr += 1) {
		if (is_uimage(addr)) {
			*image_addr = addr;
			*image_size = *(int *)(addr + 12);
			*load_addr = *(void **)(addr + 16);
			_kernel_comp_type = addr[31];
			break;
		}
	}
}

void load_uimage_from_flash(void *flash_start)
{
	void *flash_addr = flash_start;

	/*
	 * Loader has been started standalone. So no piggy backed kernel. Search
	 * flash for kernel uImage and copy it to memory before the loader.
	 */
	printf("Searching for uImage starting at 0x%08x ...\n", flash_addr);

	search_image(&flash_addr, &_kernel_data_size, &_kernel_load_addr);
	_kernel_data_addr = _my_load_addr - _kernel_data_size - 1024;

	if (!flash_addr) {
		printf("Kernel uImage not found\n");
		board_panic();
	}

	printf("uImage '%s' found at 0x%08x with load address 0x%08x\n",
	       (char *)(flash_addr + 32), flash_addr, _kernel_load_addr);
	printf("Copy %d bytes of image data to 0x%08x ...\n",
	       _kernel_data_size, _kernel_data_addr);

	memcpy(_kernel_data_addr, flash_addr + UIMAGE_HDR_SIZE, _kernel_data_size);
}

bool search_piggy_backed_uimage(void)
{
	void *addr = _kernel_data_addr;

	/*
	 * Piggy-backed data might be an uImage or not. Run a lazy uImage check.
	 * In case it fails it should be safe to assume an lzma data stream.
	 */
	search_image(&addr, &_kernel_data_size, &_kernel_load_addr);

	if (!addr)
		return false;

	printf("piggy-backed uImage '%s' found at 0x%08x with load address 0x%08x\n",
	       (char *)(addr + 32), addr, _kernel_load_addr);

	_kernel_data_addr = addr + UIMAGE_HDR_SIZE;

	return true;
}

void main(unsigned long reg_a0, unsigned long reg_a1,
	  unsigned long reg_a2, unsigned long reg_a3)
{
	void *flash_start = (void *)FLASH_ADDR; /* from makefile */
	void *kernel_addr = (void *)KERNEL_ADDR; /* from makefile */
	entry_func_t fn;

	/*
	 * During first run relocate the whole package to the end of memory. Use
	 * _my_load_size as relocation length. That includes the bss section, aka
	 * uninitialized globals. So it is possible to initialize globals during
	 * first run and have them at hand after relocation.
	 */
	if (_my_run_count == 1) {
		welcome();
		fn = relocate(_my_load_addr, _my_load_size);
		fn(reg_a0, reg_a1, reg_a2, reg_a3);
	}

	/*
	 * Usually the loader expects a piggy-backed lzma compressed kernel stream.
	 * Evaluate alternative configurations for kernel loading and decompression.
	 */
	if (flash_start)
		load_uimage_from_flash(flash_start);
	else if (!search_piggy_backed_uimage() && kernel_addr)
		_kernel_load_addr = kernel_addr;

	/*
	 * Finally extract the attached kernel image to the load address. This is
	 * either the first load address or what was found in uImage on flash
	 */
	fn = decompress(_kernel_load_addr, _kernel_data_addr, _kernel_data_size);

	printf("Booting kernel from 0x%08x ...\n\n", fn);
	fn(reg_a0, reg_a1, reg_a2, reg_a3);
}
