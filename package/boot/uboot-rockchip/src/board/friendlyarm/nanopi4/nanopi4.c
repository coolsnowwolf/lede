// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 FriendlyElec Computer Tech. Co., Ltd.
 * (http://www.friendlyarm.com)
 */

#include <common.h>
#include <dm.h>
#include <env.h>
#include <hash.h>
#include <linux/bitops.h>
#include <i2c.h>
#include <init.h>
#include <net.h>
#include <netdev.h>
#include <syscon.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/grf_rk3399.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/misc.h>
#include <asm/io.h>
#include <asm/setup.h>
#include <u-boot/sha256.h>

#ifdef CONFIG_MISC_INIT_R
static void setup_iodomain(void)
{
	struct rk3399_grf_regs *grf =
	    syscon_get_first_range(ROCKCHIP_SYSCON_GRF);

	/* BT565 and AUDIO is in 1.8v domain */
	rk_setreg(&grf->io_vsel, BIT(0) | BIT(1));
}

static int __maybe_unused mac_read_from_generic_eeprom(u8 *addr)
{
	struct udevice *i2c_dev;
	int ret;

	/* Microchip 24AA02xxx EEPROMs with EUI-48 Node Identity */
	ret = i2c_get_chip_for_busnum(2, 0x51, 1, &i2c_dev);
	if (!ret)
		ret = dm_i2c_read(i2c_dev, 0xfa, addr, 6);

	return ret;
}

static void setup_macaddr(void)
{
#if CONFIG_IS_ENABLED(CMD_NET)
	int ret;
	const char *cpuid = env_get("cpuid#");
	u8 hash[SHA256_SUM_LEN];
	int size = sizeof(hash);
	u8 mac_addr[6];
	int from_eeprom = 0;
	int lockdown = 0;

#ifndef CONFIG_ENV_IS_NOWHERE
	lockdown = env_get_yesno("lockdown") == 1;
#endif
	if (lockdown && env_get("ethaddr"))
		return;

	ret = mac_read_from_generic_eeprom(mac_addr);
	if (!ret && is_valid_ethaddr(mac_addr)) {
		eth_env_set_enetaddr("ethaddr", mac_addr);
		from_eeprom = 1;
	}

	if (!cpuid) {
		debug("%s: could not retrieve 'cpuid#'\n", __func__);
		return;
	}

	ret = hash_block("sha256", (void *)cpuid, strlen(cpuid), hash, &size);
	if (ret) {
		debug("%s: failed to calculate SHA256\n", __func__);
		return;
	}

	/* Copy 6 bytes of the hash to base the MAC address on */
	memcpy(mac_addr, hash, 6);

	/* Make this a valid MAC address and set it */
	mac_addr[0] &= 0xfe;  /* clear multicast bit */
	mac_addr[0] |= 0x02;  /* set local assignment bit (IEEE802) */

	if (from_eeprom) {
		eth_env_set_enetaddr("eth1addr", mac_addr);
	} else {
		eth_env_set_enetaddr("ethaddr", mac_addr);

		if (lockdown && env_get("eth1addr"))
			return;

		/* Ugly, copy another 4 bytes to generate a similar address */
		memcpy(mac_addr + 2, hash + 8, 4);
		if (!memcmp(hash + 2, hash + 8, 4))
			mac_addr[5] ^= 0xff;

		eth_env_set_enetaddr("eth1addr", mac_addr);
	}
#endif

	return;
}

int misc_init_r(void)
{
	const u32 cpuid_offset = 0x7;
	const u32 cpuid_length = 0x10;
	u8 cpuid[cpuid_length];
	int ret;

	setup_iodomain();

	ret = rockchip_cpuid_from_efuse(cpuid_offset, cpuid_length, cpuid);
	if (ret)
		return ret;

	ret = rockchip_cpuid_set(cpuid, cpuid_length);
	if (ret)
		return ret;

	setup_macaddr();
	bd_hwrev_init();

	return 0;
}
#endif

#ifdef CONFIG_SERIAL_TAG
void get_board_serial(struct tag_serialnr *serialnr)
{
	char *serial_string;
	u64 serial = 0;

	serial_string = env_get("serial#");

	if (serial_string)
		serial = simple_strtoull(serial_string, NULL, 16);

	serialnr->high = (u32)(serial >> 32);
	serialnr->low = (u32)(serial & 0xffffffff);
}
#endif
