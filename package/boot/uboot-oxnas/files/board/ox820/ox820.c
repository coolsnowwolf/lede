#include <common.h>
#include <spl.h>
#include <phy.h>
#include <netdev.h>
#include <ide.h>
#include <nand.h>
#include <asm/arch/spl.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/clock.h>
#include <asm/arch/sysctl.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPL_BUILD

#ifdef DEBUG
#define DILIGENCE (1048576/4)
static int test_memory(u32 memory)
{
	volatile u32 *read;
	volatile u32 *write;
	const u32 INIT_PATTERN = 0xAA55AA55;
	const u32 INC_PATTERN = 0x01030507;
	u32 pattern;
	int check;
	int i;

	check = 0;
	read = write = (volatile u32 *) memory;
	pattern = INIT_PATTERN;
	for (i = 0; i < DILIGENCE; i++) {
		*write++ = pattern;
		pattern += INC_PATTERN;
	}
	puts("testing\n");
	pattern = INIT_PATTERN;
	for (i = 0; i < DILIGENCE; i++) {
		check += (pattern == *read++) ? 1 : 0;
		pattern += INC_PATTERN;
	}
	return (check == DILIGENCE) ? 0 : -1;
}
#endif

void uart_init(void)
{
	/* Reset UART1 */
	reset_block(SYS_CTRL_RST_UART1, 1);
	udelay(100);
	reset_block(SYS_CTRL_RST_UART1, 0);
	udelay(100);

	/* Setup pin mux'ing for UART1 */
	pinmux_set(PINMUX_BANK_MFA, 30, PINMUX_UARTA_SIN);
	pinmux_set(PINMUX_BANK_MFA, 31, PINMUX_UARTA_SOUT);
}

extern void init_ddr(int mhz);

void board_inithw(void)
{
	int plla_freq;
#ifdef DEBUG
	int i;
#endif	/* DEBUG */

	timer_init();
	uart_init();
	preloader_console_init();

	plla_freq = plla_set_config(CONFIG_PLLA_FREQ_MHZ);
	init_ddr(plla_freq);

#ifdef DEBUG
	if(test_memory(CONFIG_SYS_SDRAM_BASE)) {
		puts("memory test failed\n");
	} else {
		puts("memory test done\n");
	}
#endif /* DEBUG */
#ifdef CONFIG_SPL_BSS_DRAM_START
	extern char __bss_dram_start[];
	extern char __bss_dram_end[];
	memset(&__bss_dram_start, 0, __bss_dram_end - __bss_dram_start);
#endif
}

void board_init_f(ulong dummy)
{
	/* Set the stack pointer. */
	asm volatile("mov sp, %0\n" : : "r"(CONFIG_SPL_STACK));

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* Set global data pointer. */
	gd = &gdata;

	board_inithw();

	board_init_r(NULL, 0);
}

u32 spl_boot_device(void)
{
	return CONFIG_SPL_BOOT_DEVICE;
}

#ifdef CONFIG_SPL_BLOCK_SUPPORT
void spl_block_device_init(void)
{
	ide_init();
}
#endif

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
        /* break into full u-boot on 'c' */
        return (serial_tstc() && serial_getc() == 'c');
}
#endif

void spl_display_print(void)
{
	/* print a hint, so that we will not use the wrong SPL by mistake */
	puts("  Boot device: " BOOT_DEVICE_TYPE "\n" );
}

void lowlevel_init(void)
{
}

#ifdef USE_DL_PREFIX
/* quick and dirty memory allocation */
static ulong next_mem = CONFIG_SPL_MALLOC_START;

void *memalign(size_t alignment, size_t bytes)
{
	ulong mem = ALIGN(next_mem, alignment);

	next_mem = mem + bytes;

	if (next_mem > CONFIG_SYS_SDRAM_BASE + CONFIG_MIN_SDRAM_SIZE) {
		printf("spl: out of memory\n");
		hang();
	}

	return (void *)mem;
}

void free(void* mem)
{
}
#endif

#endif /* CONFIG_SPL_BUILD */

int board_early_init_f(void)
{
	return 0;
}

#define STATIC_CTL_BANK0		(STATIC_CONTROL_BASE + 4)
#define STATIC_READ_CYCLE_SHIFT		0
#define STATIC_DELAYED_OE		(1 << 7)
#define STATIC_WRITE_CYCLE_SHIFT	8
#define STATIC_WRITE_PULSE_SHIFT	16
#define STATIC_WRITE_BURST_EN		(1 << 23)
#define STATIC_TURN_AROUND_SHIFT	24
#define STATIC_BUFFER_PRESENT		(1 << 28)
#define STATIC_READ_BURST_EN		(1 << 29)
#define STATIC_BUS_WIDTH8		(0 << 30)
#define STATIC_BUS_WIDTH16		(1 << 30)
#define STATIC_BUS_WIDTH32		(2 << 30)

void nand_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;
	unsigned long nandaddr = (unsigned long) this->IO_ADDR_W;

	if (ctrl & NAND_CTRL_CHANGE) {
		nandaddr &= ~(BIT(NAND_ALE_ADDR_PIN) | BIT(NAND_CLE_ADDR_PIN));
		if (ctrl & NAND_CLE)
			nandaddr |= BIT(NAND_CLE_ADDR_PIN);
		else if (ctrl & NAND_ALE)
			nandaddr |= BIT(NAND_ALE_ADDR_PIN);
		this->IO_ADDR_W = (void __iomem *) nandaddr;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, (void __iomem *) nandaddr);
}

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_BOOT_FROM_NAND)

int nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	udelay(chip->chip_delay);

	return 1;
}

void nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;
	struct nand_chip *chip = mtd->priv;

	for (i = 0; i < len; i++)
		buf[i] = readb(chip->IO_ADDR_R);
}

void nand_dev_reset(struct nand_chip *chip)
{
	writeb(NAND_CMD_RESET, chip->IO_ADDR_W + BIT(NAND_CLE_ADDR_PIN));
	udelay(chip->chip_delay);
	writeb(NAND_CMD_STATUS, chip->IO_ADDR_W + BIT(NAND_CLE_ADDR_PIN));
	while (!(readb(chip->IO_ADDR_R) & NAND_STATUS_READY)) {
		;
	}
}

#else

#define nand_dev_reset(chip)	/* framework will reset the chip anyway */
#define nand_read_buf		NULL /* framework will provide a default one */
#define nand_dev_ready		NULL /* dev_ready is optional */

#endif

int board_nand_init(struct nand_chip *chip)
{
	/* Block reset Static core */
	reset_block(SYS_CTRL_RST_STATIC, 1);
	reset_block(SYS_CTRL_RST_STATIC, 0);

	/* Enable clock to Static core */
	enable_clock(SYS_CTRL_CLK_STATIC);

	/* enable flash support on static bus.
	 * Enable static bus onto GPIOs, only CS0 */
	pinmux_set(PINMUX_BANK_MFA, 12, PINMUX_STATIC_DATA0);
	pinmux_set(PINMUX_BANK_MFA, 13, PINMUX_STATIC_DATA1);
	pinmux_set(PINMUX_BANK_MFA, 14, PINMUX_STATIC_DATA2);
	pinmux_set(PINMUX_BANK_MFA, 15, PINMUX_STATIC_DATA3);
	pinmux_set(PINMUX_BANK_MFA, 16, PINMUX_STATIC_DATA4);
	pinmux_set(PINMUX_BANK_MFA, 17, PINMUX_STATIC_DATA5);
	pinmux_set(PINMUX_BANK_MFA, 18, PINMUX_STATIC_DATA6);
	pinmux_set(PINMUX_BANK_MFA, 19, PINMUX_STATIC_DATA7);

	pinmux_set(PINMUX_BANK_MFA, 20, PINMUX_STATIC_NWE);
	pinmux_set(PINMUX_BANK_MFA, 21, PINMUX_STATIC_NOE);
	pinmux_set(PINMUX_BANK_MFA, 22, PINMUX_STATIC_NCS);
	pinmux_set(PINMUX_BANK_MFA, 23, PINMUX_STATIC_ADDR18);
	pinmux_set(PINMUX_BANK_MFA, 24, PINMUX_STATIC_ADDR19);

	/* Setup the static bus CS0 to access FLASH */

	writel((0x3f << STATIC_READ_CYCLE_SHIFT)
			| (0x3f << STATIC_WRITE_CYCLE_SHIFT)
			| (0x1f << STATIC_WRITE_PULSE_SHIFT)
			| (0x03 << STATIC_TURN_AROUND_SHIFT) |
			STATIC_BUS_WIDTH16,
		STATIC_CTL_BANK0);

	chip->cmd_ctrl = nand_hwcontrol;
	chip->ecc.mode = NAND_ECC_SOFT;
	chip->chip_delay = 30;
	chip->dev_ready = nand_dev_ready;
	chip->read_buf = nand_read_buf;

	nand_dev_reset(chip);

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;
	gd->bd->bi_arch_number = MACH_TYPE_OXNAS;

	/* assume uart is already initialized by SPL */

#if defined(CONFIG_START_IDE)
	puts("IDE:   ");
	ide_init();
#endif

	return 0;
}

/* copied from board/evb64260/sdram_init.c */
/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */
static long int dram_size (long int *base, long int maxsize)
{
	volatile long int *addr, *b = base;
	long int cnt, val, save1, save2;

#define STARTVAL (CONFIG_MIN_SDRAM_SIZE / 2)	/* start test at half size */
	for (cnt = STARTVAL / sizeof (long); cnt < maxsize / sizeof (long);
	     cnt <<= 1) {
		addr = base + cnt;	/* pointer arith! */

		save1 = *addr;	/* save contents of addr */
		save2 = *b;	/* save contents of base */

		*addr = cnt;	/* write cnt to addr */
		*b = 0;		/* put null at base */

		/* check at base address */
		if ((*b) != 0) {
			*addr = save1;	/* restore *addr */
			*b = save2;	/* restore *b */
			return (0);
		}
		val = *addr;	/* read *addr */

		*addr = save1;
		*b = save2;

		if (val != cnt) {
			/* fix boundary condition.. STARTVAL means zero */
			if (cnt == STARTVAL / sizeof (long))
				cnt = 0;
			return (cnt * sizeof (long));
		}
	}
	return maxsize;
}

int dram_init(void)
{
	gd->ram_size = dram_size((long int *)CONFIG_SYS_SDRAM_BASE,
					CONFIG_MAX_SDRAM_SIZE);
	return 0;
}

int board_eth_init(bd_t *bis)
{
	u32 value;

	/* set the pin multiplexers to enable talking to Ethernent Phys */
	pinmux_set(PINMUX_BANK_MFA, 3, PINMUX_MACA_MDC);
	pinmux_set(PINMUX_BANK_MFA, 4, PINMUX_MACA_MDIO);

	// Ensure the MAC block is properly reset
	reset_block(SYS_CTRL_RST_MAC, 1);
	udelay(10);
	reset_block(SYS_CTRL_RST_MAC, 0);

	// Enable the clock to the MAC block
	enable_clock(SYS_CTRL_CLK_MAC);

	value = readl(SYS_CTRL_GMAC_CTRL);
	/* Use simple mux for 25/125 Mhz clock switching */
	value |= BIT(SYS_CTRL_GMAC_SIMPLE_MUX);
	/* Enable GMII_GTXCLK to follow GMII_REFCLK - required for gigabit PHY */
	value |= BIT(SYS_CTRL_GMAC_CKEN_GTX);
	/* set auto tx speed */
	value |= BIT(SYS_CTRL_GMAC_AUTOSPEED);

	writel(value, SYS_CTRL_GMAC_CTRL);

	return designware_initialize(MAC_BASE, PHY_INTERFACE_MODE_RGMII);
}

