/*
 * rt-loader board functions
 * (c) 2025 Markus Stockhausen
 */

#include "globals.h"
#include "memory.h"
#include "nanoprintf.h"

#define UART_BUFFER_REG			0xb8002000
#define UART_LINE_STATUS_REG		0xb8002014
#define UART_TX_READY			(1 << 29)

#define RTL83XX_DRAM_CONFIG_REG		0xb8001004
#define RTL838X_ENABLE_RW_MASK		0x3
#define RTL838X_INT_RW_CTRL_REG		0xbb000058
#define RTL838X_MODEL_NAME_INFO_REG	0xbb0000d4
#define RTL839X_MODEL_NAME_INFO_REG	0xbb000ff0
#define RTL83XX_CHIP_INFO_EN		0xa0000000
#define RTL931X_DRAM_CONFIG_REG		0xb814304c
#define RTL93XX_MODEL_NAME_INFO_REG	0xbb000004
#define RTL93XX_CHIP_INFO_EN		0xa0000


/*
 * board_putchar() is the central function to write to serial console of the device. This is
 * linked to printf() and the provider library via globals.h.
 */

void board_putchar(int ch, void *ctx)
{
	while (!(ioread32(UART_LINE_STATUS_REG) & UART_TX_READY));
	iowrite32(((int)ch) << 24, UART_BUFFER_REG);

	if (ch == '\n')
		board_putchar('\r', ctx);
}

/*
 * board_get_memory() does what it is named after. On Realtek switches the DRAM config register
 * has information about bank count, bus width, ... From that the memory size can be derived.
 */

unsigned int board_get_memory(void)
{
	unsigned int dcr, bits;

	if ((ioread32(RTL93XX_MODEL_NAME_INFO_REG) & 0xfffc0000) == 0x93100000) {
		dcr = ioread32(RTL931X_DRAM_CONFIG_REG);
		bits = (dcr >> 12) + ((dcr >> 6) & 0x3f) + (dcr & 0x3f);
	} else {
		dcr = ioread32(RTL83XX_DRAM_CONFIG_REG);
		bits = ((dcr >> 28) & 0x3) + ((dcr >> 24) & 0x3) +
		       ((dcr >> 20) & 0xf) + ((dcr >> 16) & 0xf) + 20;
	}

	return 1 << bits;
}

/*
 * board_get_system() generates a readable system name that will be printed during startup.
 * Formatting can be whatever is helpful.
 */

void board_get_system(char *buffer, int len)
{
	unsigned int chip_id, model_id, model_version, chip_version;
	unsigned int reg, act, minfo, cinfo;

	act = RTL93XX_CHIP_INFO_EN;
	reg = RTL93XX_MODEL_NAME_INFO_REG;
	minfo = ioread32(reg);

	if ((minfo & 0xffec0000) == 0x93000000)
		goto found;

	act = RTL83XX_CHIP_INFO_EN;
	reg = RTL839X_MODEL_NAME_INFO_REG;
	minfo = ioread32(reg);
	if ((minfo & 0xfff80000) == 0x83900000)
		goto found;

	iowrite32(0x3, RTL838X_INT_RW_CTRL_REG);
	reg = RTL838X_MODEL_NAME_INFO_REG;
	minfo = ioread32(reg);
found:
	iowrite32(act, reg + 4);
	cinfo = ioread32(reg + 4);

	model_id = minfo >> 16;
	model_version = (minfo >> 11) & 0x1f;
	chip_id = cinfo & 0xffff;

	if (model_id >= 0x9300)
		chip_version = minfo & 0xf;
	else if (model_id >= 0x8390)
		chip_version = (minfo >> 1) & 0x1f;
	else
		chip_version = ((cinfo >> 16) & 0x1f) - 1;

	snprintf(buffer, len, "RTL%04X%c rev %c (%04x)", model_id,
		 model_version ? model_version + 64 : 0, chip_version + 65, chip_id);
}

/*
 * board_panic() is called in critical cases. Whatever is needed can be done here. Maybe
 * an automatic reboot can be issued some day. For now just halt processing.
 */

void board_panic(void)
{
	printf("halt system\n");
	while (1) {
	}
}
