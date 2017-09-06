#include <common.h>
#include <asm/arch/sysctl.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>

typedef struct {
	unsigned short mhz;
	unsigned char refdiv;
	unsigned char outdiv;
	unsigned int fbdiv;
	unsigned short bwadj;
	unsigned short sfreq;
	unsigned int sslope;
} PLL_CONFIG;

const PLL_CONFIG C_PLL_CONFIG[] = {
	{ 500, 1, 2, 3932160, 119, 208, 189 }, //  500 MHz
	{ 525, 2, 1, 4128768, 125, 139, 297 }, //  525 MHz
	{ 550, 2, 1, 4325376, 131, 139, 311 }, //  550 MHz
	{ 575, 2, 1, 4521984, 137, 139, 326 }, //  575 MHz
	{ 600, 2, 1, 4718592, 143, 138, 339 }, //  600 MHz
	{ 625, 1, 1, 3276800, 99, 208, 157 }, //  625 MHz
	{ 650, 1, 1, 3407872, 103, 208, 164 }, //  650 MHz
	{ 675, 1, 1, 3538944, 107, 208, 170 }, //  675 MHz
	{ 700, 0, 0, 917504, 27, 416, 22 }, //  700 MHz
	{ 725, 1, 1, 3801088, 115, 208, 182 }, //  725 MHz
	{ 750, 0, 0, 983040, 29, 416, 23 }, //  750 MHz
	{ 775, 3, 0, 4063232, 123, 104, 390 }, //  775 MHz
	{ 800, 3, 0, 4194304, 127, 104, 403 }, //  800 MHz
	{ 825, 3, 0, 4325376, 131, 104, 415 }, //  825 MHz
	{ 850, 2, 0, 3342336, 101, 139, 241 }, //  850 MHz
	{ 875, 2, 0, 3440640, 104, 139, 248 }, //  875 MHz
	{ 900, 2, 0, 3538944, 107, 139, 255 }, //  900 MHz
	{ 925, 2, 0, 3637248, 110, 139, 262 }, //  925 MHz
	{ 950, 2, 0, 3735552, 113, 139, 269 }, //  950 MHz
	{ 975, 2, 0, 3833856, 116, 139, 276 }, //  975 MHz
	{ 1000, 2, 0, 3932160, 119, 139, 283 }, // 1000 MHz
};

#define PLL_BYPASS (1<<1)
#define SAT_ENABLE (1<<3)

#define PLL_OUTDIV_SHIFT	4
#define PLL_REFDIV_SHIFT	8
#define PLL_BWADJ_SHIFT		16

#define PLL_LOW_FREQ	500
#define PLL_FREQ_STEP	25
static void plla_configure(int outdiv, int refdiv, int fbdiv, int bwadj,
                           int sfreq, int sslope)
{
	setbits_le32(SYS_CTRL_PLLA_CTRL0, PLL_BYPASS);
	udelay(10);
	reset_block(SYS_CTRL_RST_PLLA, 1);
	udelay(10);

	writel((refdiv << PLL_REFDIV_SHIFT) | (outdiv << PLL_OUTDIV_SHIFT) |
	       SAT_ENABLE | PLL_BYPASS,
	       SYS_CTRL_PLLA_CTRL0);

	writel(fbdiv, SYS_CTRL_PLLA_CTRL1);
	writel((bwadj << PLL_BWADJ_SHIFT) | sfreq, SYS_CTRL_PLLA_CTRL2);
	writel(sslope, SYS_CTRL_PLLA_CTRL3);

	udelay(10); // 5us delay required (from TCI datasheet), use 10us

	reset_block(SYS_CTRL_RST_PLLA, 0);

	udelay(100); // Delay for PLL to lock

	printf("  plla_ctrl0 : %08x\n", readl(SYS_CTRL_PLLA_CTRL0));
	printf("  plla_ctrl1 : %08x\n", readl(SYS_CTRL_PLLA_CTRL1));
	printf("  plla_ctrl2 : %08x\n", readl(SYS_CTRL_PLLA_CTRL2));
	printf("  plla_ctrl3 : %08x\n", readl(SYS_CTRL_PLLA_CTRL3));

	clrbits_le32(SYS_CTRL_PLLA_CTRL0, PLL_BYPASS); // Take PLL out of bypass
	puts("\nPLLA Set\n");
}

int plla_set_config(int mhz)
{
	int index = (mhz - PLL_LOW_FREQ) / PLL_FREQ_STEP;
	const PLL_CONFIG *cfg;

	if (index < 0 || index > ARRAY_SIZE(C_PLL_CONFIG)) {
		debug("Freq %d MHz out of range, default to lowest\n", mhz);
		index = 0;
	}
	cfg = &C_PLL_CONFIG[index];

	printf("Attempting to set PLLA to %d MHz ...\n", (unsigned) cfg->mhz);
	plla_configure(cfg->outdiv, cfg->refdiv, cfg->fbdiv, cfg->bwadj,
	               cfg->sfreq, cfg->sslope);

	return cfg->mhz;
}

