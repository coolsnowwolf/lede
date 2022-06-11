/*
 * B53 PHY Fixup call
 *
 * Copyright (C) 2013 Jonas Gorski <jogo@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/phy.h>

#define B53_PSEUDO_PHY	0x1e /* Register Access Pseudo PHY */

#define B53_BRCM_OUI_1	0x0143bc00
#define B53_BRCM_OUI_2	0x03625c00
#define B53_BRCM_OUI_3	0x00406300

static int b53_phy_fixup(struct phy_device *dev)
{
	struct mii_bus *bus = dev->mdio.bus;
	u32 phy_id;

	if (dev->mdio.addr != B53_PSEUDO_PHY)
		return 0;

	/* read the first port's id */
	phy_id = mdiobus_read(bus, 0, 2) << 16;
	phy_id |= mdiobus_read(bus, 0, 3);

	if ((phy_id & 0xfffffc00) == B53_BRCM_OUI_1 ||
	    (phy_id & 0xfffffc00) == B53_BRCM_OUI_2 ||
	    (phy_id & 0xffffff00) == B53_BRCM_OUI_3) {
		dev->phy_id = phy_id;
	}

	return 0;
}

int __init b53_phy_fixup_register(void)
{
	return phy_register_fixup_for_id(PHY_ANY_ID, b53_phy_fixup);
}

subsys_initcall(b53_phy_fixup_register);
