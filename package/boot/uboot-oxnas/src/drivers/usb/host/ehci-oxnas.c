/*
 * drivers/usb/host/ehci-oxnas.c
 *
 * Tzachi Perelstein <tzachi@marvell.com>
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sysctl.h>
#include <asm/arch/clock.h>

#include "ehci.h"

static struct ehci_hcor *ghcor;

static int start_oxnas_usb_ehci(void)
{
#ifdef CONFIG_USB_PLLB_CLK
	reset_block(SYS_CTRL_RST_PLLB, 0);
	enable_clock(SYS_CTRL_CLK_REF600);

	writel((1 << PLLB_ENSAT) | (1 << PLLB_OUTDIV) | (2 << PLLB_REFDIV),
			SEC_CTRL_PLLB_CTRL0);
	/* 600MHz pllb divider for 12MHz */
	writel(PLLB_DIV_INT(50) | PLLB_DIV_FRAC(0), SEC_CTRL_PLLB_DIV_CTRL);
#else
	/* ref 300 divider for 12MHz */
	writel(REF300_DIV_INT(25) | REF300_DIV_FRAC(0), SYS_CTRL_REF300_DIV);
#endif

	/* Ensure the USB block is properly reset */
	reset_block(SYS_CTRL_RST_USBHS, 1);
	reset_block(SYS_CTRL_RST_USBHS, 0);

	reset_block(SYS_CTRL_RST_USBHSPHYA, 1);
	reset_block(SYS_CTRL_RST_USBHSPHYA, 0);

	reset_block(SYS_CTRL_RST_USBHSPHYB, 1);
	reset_block(SYS_CTRL_RST_USBHSPHYB, 0);

	/* Force the high speed clock to be generated all the time, via serial
	 programming of the USB HS PHY */
	writel((2UL << USBHSPHY_TEST_ADD) |
		   (0xe0UL << USBHSPHY_TEST_DIN), SYS_CTRL_USBHSPHY_CTRL);

	writel((1UL << USBHSPHY_TEST_CLK) |
		   (2UL << USBHSPHY_TEST_ADD) |
		   (0xe0UL << USBHSPHY_TEST_DIN), SYS_CTRL_USBHSPHY_CTRL);

	writel((0xfUL << USBHSPHY_TEST_ADD) |
		   (0xaaUL << USBHSPHY_TEST_DIN), SYS_CTRL_USBHSPHY_CTRL);

	writel((1UL << USBHSPHY_TEST_CLK) |
		   (0xfUL << USBHSPHY_TEST_ADD) |
		   (0xaaUL << USBHSPHY_TEST_DIN), SYS_CTRL_USBHSPHY_CTRL);

#ifdef CONFIG_USB_PLLB_CLK /* use pllb clock */
		writel(USB_CLK_INTERNAL | USB_INT_CLK_PLLB, SYS_CTRL_USB_CTRL);
#else /* use ref300 derived clock */
		writel(USB_CLK_INTERNAL | USB_INT_CLK_REF300, SYS_CTRL_USB_CTRL);
#endif
	/* Enable the clock to the USB block */
	enable_clock(SYS_CTRL_CLK_USBHS);

	return 0;
}
int ehci_hcd_init(int index, enum usb_init_type init, struct ehci_hccr **hccr,
		  struct ehci_hcor **hcor)
{
	start_oxnas_usb_ehci();
	*hccr = (struct ehci_hccr *)(USB_HOST_BASE + 0x100);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
	ghcor = *hcor;
	return 0;
}

int ehci_hcd_stop(int index)
{
	reset_block(SYS_CTRL_RST_USBHS, 1);
	disable_clock(SYS_CTRL_CLK_USBHS);
	return 0;
}

extern void __ehci_set_usbmode(int index);
void ehci_set_usbmode(int index)
{
	#define  or_txttfill_tuning	_reserved_1_[0]
	u32 tmp;

	__ehci_set_usbmode(index);

	tmp = ehci_readl(&ghcor->or_txfilltuning);
	tmp &= ~0x00ff0000;
	tmp |= 0x003f0000; /* set burst pre load count to 0x40 (63 * 4 bytes)  */
	tmp |= 0x16; /* set sheduler overhead to 22 * 1.267us (HS) or 22 * 6.33us (FS/LS)*/
	ehci_writel(&ghcor->or_txfilltuning, tmp);

	tmp = ehci_readl(&ghcor->or_txttfill_tuning);
	tmp |= 0x2; /* set sheduler overhead to 2 * 6.333us */
	ehci_writel(&ghcor->or_txttfill_tuning, tmp);
}
