// SPDX-License-Identifier: GPL-2.0-only
/* Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd. */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/time64.h>
#include <linux/ptp_classify.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/net_tstamp.h>
#include <linux/circ_buf.h>
#include <linux/spinlock.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/acpi.h>
#include <linux/arm-smccc.h>
#include "phytmac.h"
#include "phytmac_v1.h"

static int phytmac_enable_promise(struct phytmac *pdata, int enable)
{
	u32 value = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);

	if (enable)
		value |= PHYTMAC_BIT(PROMISC);
	else
		value &= ~PHYTMAC_BIT(PROMISC);

	PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, value);

	return 0;
}

static int phytmac_enable_multicast(struct phytmac *pdata, int enable)
{
	u32 config;

	if (enable) {
		PHYTMAC_WRITE(pdata, PHYTMAC_HASHB, -1);
		PHYTMAC_WRITE(pdata, PHYTMAC_HASHT, -1);
		config = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);
		config |= PHYTMAC_BIT(MH_EN);
		PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, config);
	} else {
		PHYTMAC_WRITE(pdata, PHYTMAC_HASHB, 0);
		PHYTMAC_WRITE(pdata, PHYTMAC_HASHT, 0);
		config = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);
		config &= ~PHYTMAC_BIT(MH_EN);
		PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, config);
	}

	return 0;
}

static int phytmac_set_mc_hash(struct phytmac *pdata, unsigned long *mc_filter)
{
	u32 config;

	PHYTMAC_WRITE(pdata, PHYTMAC_HASHB, mc_filter[0]);
	PHYTMAC_WRITE(pdata, PHYTMAC_HASHT, mc_filter[1]);
	config = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);
	config |= PHYTMAC_BIT(MH_EN);
	PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, config);

	return 0;
}

static int phytmac_enable_rxcsum(struct phytmac *pdata, int enable)
{
	u32 value = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);

	if (enable)
		value |= PHYTMAC_BIT(RCO_EN);
	else
		value &= ~PHYTMAC_BIT(RCO_EN);

	PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, value);

	return 0;
}

static int phytmac_enable_txcsum(struct phytmac *pdata, int enable)
{
	u32 value = PHYTMAC_READ(pdata, PHYTMAC_DCONFIG);

	if (enable)
		value |= PHYTMAC_BIT(TCO_EN);
	else
		value &= ~PHYTMAC_BIT(TCO_EN);

	PHYTMAC_WRITE(pdata, PHYTMAC_DCONFIG, value);

	return 0;
}

static int phytmac_enable_mdio(struct phytmac *pdata, int enable)
{
	u32 value = PHYTMAC_READ(pdata, PHYTMAC_NCTRL);

	if (enable)
		value |= PHYTMAC_BIT(MPE);
	else
		value &= ~PHYTMAC_BIT(MPE);

	PHYTMAC_WRITE(pdata, PHYTMAC_NCTRL, value);

	return 0;
}

static int phytmac_enable_pause(struct phytmac *pdata, int enable)
{
	u32 value = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);

	if (enable)
		value |= PHYTMAC_BIT(PAUSE_EN);
	else
		value &= ~PHYTMAC_BIT(PAUSE_EN);

	PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, value);
	return 0;
}

static int phytmac_enable_network(struct phytmac *pdata, int enable, int rx_tx)
{
	u32 old_ctrl = PHYTMAC_READ(pdata, PHYTMAC_NCTRL);
	u32 ctrl;

	ctrl = old_ctrl;

	if (rx_tx & PHYTMAC_TX) {
		if (enable)
			ctrl |= PHYTMAC_BIT(TE);
		else
			ctrl &= ~PHYTMAC_BIT(TE);
	}

	if (rx_tx & PHYTMAC_RX) {
		if (enable)
			ctrl |= PHYTMAC_BIT(RE);
		else
			ctrl &= ~PHYTMAC_BIT(RE);
	}

	if (ctrl ^ old_ctrl)
		PHYTMAC_WRITE(pdata, PHYTMAC_NCTRL, ctrl);

	return 0;
}

static int phytmac_enable_autoneg(struct phytmac *pdata, int enable)
{
	u32 value = PHYTMAC_READ(pdata, PHYTMAC_PCSCTRL);

	if (enable)
		value |= PHYTMAC_BIT(AUTONEG);
	else
		value &= ~PHYTMAC_BIT(AUTONEG);

	PHYTMAC_WRITE(pdata, PHYTMAC_PCSCTRL, value);

	return 0;
}

static int phytmac_pcs_software_reset(struct phytmac *pdata, int reset)
{
	u32 value = PHYTMAC_READ(pdata, PHYTMAC_PCSCTRL);

	if (reset)
		value |= PHYTMAC_BIT(PCS_RESET);
	else
		value &= ~PHYTMAC_BIT(PCS_RESET);

	PHYTMAC_WRITE(pdata, PHYTMAC_PCSCTRL, value);

	return 0;
}

static void phytmac_phy_speed_switch(struct phytmac *pdata, int speed)
{
	struct arm_smccc_res res;
	struct phyinit_cfg cfg;
	int id;

	switch (pdata->ndev->base_addr) {
	case MAC0_ADDR_BASE:
		id = 0;
		break;
	case MAC1_ADDR_BASE:
		id = 1;
		break;
	default:
		return;
	}

	switch (speed) {
	case SPEED_1000:
	case SPEED_100:
	case SPEED_10:
		cfg.phy_speed = PHY_SPEED_1000(id);
		break;
	case SPEED_2500:
		cfg.phy_speed = PHY_SPEED_2500(id);
		break;
	default:
		return;
	}

	cfg.phy_domain = PHY_INIT(id);
	cfg.phy_multiplex = PHY_MULTIPLEX_GMAC(id);
	cfg.phy_mode = PHY_MODE_SGMII(id);

	arm_smccc_smc(PHY_INIT_SMC_ID, cfg.phy_domain, cfg.phy_multiplex,
		      cfg.phy_mode, cfg.phy_speed, 0, 0, 0, &res);
	if (res.a0 == SMCCC_RET_NOT_SUPPORTED) {
		netdev_warn(pdata->ndev, "PHY_INIT_SMC_ID not implemented, skipping ....\n");
		goto out;
	}
	netdev_info(pdata->ndev, "switch speed to %dMb/s successfully\n", speed);

out:
	return;
}

static int phytmac_mac_linkup(struct phytmac *pdata, phy_interface_t interface,
			      int speed, int duplex)
{
	u32 ctrl, config;

	config = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);

	config &= ~(PHYTMAC_BIT(SPEED) | PHYTMAC_BIT(FD) | PHYTMAC_BIT(GM_EN));

	if (speed == SPEED_100)
		config |= PHYTMAC_BIT(SPEED);
	else if (speed == SPEED_1000 || speed == SPEED_2500)
		config |= PHYTMAC_BIT(GM_EN);

	if (duplex)
		config |= PHYTMAC_BIT(FD);

	PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, config);

	if (speed == SPEED_2500) {
		ctrl = PHYTMAC_READ(pdata, PHYTMAC_NCTRL);
		ctrl |= PHYTMAC_BIT(2PT5G);
		PHYTMAC_WRITE(pdata, PHYTMAC_NCTRL, ctrl);
	}

	if (speed == SPEED_10000)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_10000M);
	else if (speed == SPEED_5000)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_5000M);
	else if (speed == SPEED_2500)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_2500M);
	else if (speed == SPEED_1000)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_1000M);
	else
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_100M);

	if (interface == PHY_INTERFACE_MODE_SGMII) {
		if (speed == SPEED_2500)
			phytmac_enable_autoneg(pdata, 0);
		else
			phytmac_enable_autoneg(pdata, 1);
	}

	return 0;
}

static int phytmac_mac_linkdown(struct phytmac *pdata)
{
	return 0;
}

static int phytmac_pcs_linkup(struct phytmac *pdata, phy_interface_t interface,
			      int speed, int duplex)
{
	u32 config;

	if (interface == PHY_INTERFACE_MODE_USXGMII ||
	    interface == PHY_INTERFACE_MODE_10GBASER) {
		config = PHYTMAC_READ(pdata, PHYTMAC_USXCTRL);
		if (speed == SPEED_10000) {
			config = PHYTMAC_SET_BITS(config, SERDES_RATE, PHYTMAC_SERDES_RATE_10G);
			config = PHYTMAC_SET_BITS(config, USX_SPEED, PHYTMAC_SPEED_10000M);
		} else if (speed == SPEED_5000) {
			config = PHYTMAC_SET_BITS(config, SERDES_RATE, PHYTMAC_SERDES_RATE_5G);
			config = PHYTMAC_SET_BITS(config, USX_SPEED, PHYTMAC_SPEED_5000M);
		}

		/* reset */
		config &= ~(PHYTMAC_BIT(RX_EN) | PHYTMAC_BIT(TX_EN));
		config |= PHYTMAC_BIT(RX_SYNC_RESET);

		PHYTMAC_WRITE(pdata, PHYTMAC_USXCTRL, config);

		/* enable rx and tx */
		config &= ~(PHYTMAC_BIT(RX_SYNC_RESET));
		config |= PHYTMAC_BIT(RX_EN) | PHYTMAC_BIT(TX_EN);

		PHYTMAC_WRITE(pdata, PHYTMAC_USXCTRL, config);
	}

	return 0;
}

static int phytmac_pcs_linkdown(struct phytmac *pdata)
{
	return 0;
}

static int phytmac_get_mac_addr(struct phytmac *pdata, u8 *addr)
{
	u32 bottom;
	u16 top;

	bottom = PHYTMAC_READ(pdata, PHYTMAC_MAC1B);
	top = PHYTMAC_READ(pdata, PHYTMAC_MAC1T);

	addr[0] = bottom & 0xff;
	addr[1] = (bottom >> 8) & 0xff;
	addr[2] = (bottom >> 16) & 0xff;
	addr[3] = (bottom >> 24) & 0xff;
	addr[4] = top & 0xff;
	addr[5] = (top >> 8) & 0xff;

	return 0;
}

static int phytmac_set_mac_addr(struct phytmac *pdata, const u8 *addr)
{
	u32 bottom;
	u16 top;

	bottom = cpu_to_le32(*((u32 *)addr));
	PHYTMAC_WRITE(pdata, PHYTMAC_MAC1B, bottom);
	top = cpu_to_le16(*((u16 *)(addr + 4)));
	PHYTMAC_WRITE(pdata, PHYTMAC_MAC1T, top);

	return 0;
}

static void phytmac_reset_hw(struct phytmac *pdata)
{
	unsigned int q;
	u32 ctrl;

	ctrl = PHYTMAC_READ(pdata, PHYTMAC_NCTRL);

	ctrl &= ~(PHYTMAC_BIT(RE) | PHYTMAC_BIT(TE));
	ctrl |= PHYTMAC_BIT(CLEARSTAT);
	PHYTMAC_WRITE(pdata, PHYTMAC_NCTRL, ctrl);

	/* Disable and clear all interrupts and disable queues */
	for (q = 0; q < pdata->queues_max_num; ++q) {
		if (q == 0) {
			PHYTMAC_WRITE(pdata, PHYTMAC_ID, -1);
			PHYTMAC_WRITE(pdata, PHYTMAC_IS, -1);
			PHYTMAC_WRITE(pdata, PHYTMAC_TXPTR_Q0, 1);
			PHYTMAC_WRITE(pdata, PHYTMAC_RXPTR_Q0, 1);
		} else {
			PHYTMAC_WRITE(pdata, PHYTMAC_IDR(q - 1), -1);
			PHYTMAC_WRITE(pdata, PHYTMAC_ISR(q - 1), -1);
			PHYTMAC_WRITE(pdata, PHYTMAC_TXPTR(q - 1), 1);
			PHYTMAC_WRITE(pdata, PHYTMAC_RXPTR(q - 1), 1);
		}

		PHYTMAC_WRITE(pdata, PHYTMAC_TXPTRH(q), 0);
		PHYTMAC_WRITE(pdata, PHYTMAC_RXPTRH(q), 0);

		if (pdata->capacities & PHYTMAC_CAPS_TAILPTR)
			PHYTMAC_WRITE(pdata, PHYTMAC_TX_TAILPTR(q), 0);

		if (pdata->capacities & PHYTMAC_CAPS_RXPTR)
			PHYTMAC_WRITE(pdata, PHYTMAC_RX_TAILPTR(q), 0);
	}
}

static void phytmac_get_regs(struct phytmac *pdata, u32 *reg_buff)
{
	reg_buff[0] = PHYTMAC_READ(pdata, PHYTMAC_NCTRL);
	reg_buff[1] = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);
	reg_buff[2] = PHYTMAC_READ(pdata, PHYTMAC_NSTATUS);
	reg_buff[3] = PHYTMAC_READ(pdata, PHYTMAC_DCONFIG);
	reg_buff[4] = PHYTMAC_READ(pdata, PHYTMAC_TXPTR_Q0);
	reg_buff[5] = PHYTMAC_READ(pdata, PHYTMAC_RXPTR_Q0);
	reg_buff[6] = PHYTMAC_READ(pdata, PHYTMAC_TXPTR(1));
	reg_buff[7] = PHYTMAC_READ(pdata, PHYTMAC_RXPTR(1));
	reg_buff[8] = PHYTMAC_READ(pdata, PHYTMAC_TXPTR(2));
	reg_buff[9] = PHYTMAC_READ(pdata, PHYTMAC_RXPTR(2));
	reg_buff[10] = PHYTMAC_READ(pdata, PHYTMAC_TXPTR(3));
	reg_buff[11] = PHYTMAC_READ(pdata, PHYTMAC_RXPTR(3));
	reg_buff[12] = PHYTMAC_READ(pdata, PHYTMAC_HCONFIG);
	reg_buff[13] = PHYTMAC_READ(pdata, PHYTMAC_IM);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_USXGMII ||
	    pdata->phy_interface == PHY_INTERFACE_MODE_10GBASER) {
		reg_buff[14] = PHYTMAC_READ(pdata, PHYTMAC_USXCTRL);
		reg_buff[15] = PHYTMAC_READ(pdata, PHYTMAC_USXSTATUS);
	} else {
		reg_buff[14] = PHYTMAC_READ(pdata, PHYTMAC_PCSCTRL);
		reg_buff[15] = PHYTMAC_READ(pdata, PHYTMAC_PCSSTATUS);
	}
}

static int phytmac_init_hw(struct phytmac *pdata)
{
	u32 config = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);
	u32 dmaconfig;
	u32 nctrlconfig;
	u32 ptrconfig = 0;

	nctrlconfig = PHYTMAC_READ(pdata, PHYTMAC_NCTRL);
	nctrlconfig |= PHYTMAC_BIT(MPE);
	PHYTMAC_WRITE(pdata, PHYTMAC_NCTRL, nctrlconfig);

	phytmac_set_mac_addr(pdata, pdata->ndev->dev_addr);

	PHYTMAC_WRITE(pdata, PHYTMAC_AXICTRL, 0x1010);

	/* jumbo */
	if (pdata->capacities & PHYTMAC_CAPS_JUMBO)
		config |= PHYTMAC_BIT(JUMBO_EN);
	else
		config |= PHYTMAC_BIT(RCV_BIG);
	/* promisc */
	if (pdata->ndev->flags & IFF_PROMISC)
		config |= PHYTMAC_BIT(PROMISC);
	if (pdata->ndev->features & NETIF_F_RXCSUM)
		config |= PHYTMAC_BIT(RCO_EN);
	if (pdata->ndev->flags & IFF_BROADCAST)
		config &= ~PHYTMAC_BIT(NO_BCAST);
	else
		config |= PHYTMAC_BIT(NO_BCAST);

	/* pause enable */
	config |= PHYTMAC_BIT(PAUSE_EN);
	/* Rx Fcs remove */
	config |= PHYTMAC_BIT(FCS_REMOVE);
	if (pdata->dma_data_width == PHYTMAC_DBW_64)
		config |= PHYTMAC_BIT(DBW64);
	if (pdata->dma_data_width == PHYTMAC_DBW_128)
		config |= PHYTMAC_BIT(DBW128);
	/* mdc div */
	config = PHYTMAC_SET_BITS(config, MCD, 6);
	netdev_dbg(pdata->ndev, "phytmac configure NetConfig with 0x%08x\n",
		   config);
	PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, config);

	/* init dma */
	dmaconfig = PHYTMAC_READ(pdata, PHYTMAC_DCONFIG);
	if (pdata->dma_burst_length)
		dmaconfig = PHYTMAC_SET_BITS(dmaconfig, BURST, pdata->dma_burst_length);
	/* default in small endian */
	dmaconfig &= ~(PHYTMAC_BIT(ENDIA_PKT) | PHYTMAC_BIT(ENDIA_DESC));

	if (pdata->ndev->features & NETIF_F_HW_CSUM)
		dmaconfig |= PHYTMAC_BIT(TCO_EN);
	else
		dmaconfig &= ~PHYTMAC_BIT(TCO_EN);

	if (pdata->dma_addr_width)
		dmaconfig |= PHYTMAC_BIT(ABW);

	/* fdir ethtype -- ipv4 */
	PHYTMAC_WRITE(pdata, PHYTMAC_ETHT(0), (uint16_t)ETH_P_IP);

	if (IS_REACHABLE(CONFIG_PHYTMAC_ENABLE_PTP))
		dmaconfig |= PHYTMAC_BIT(RX_EXBD_EN) | PHYTMAC_BIT(TX_EXBD_EN);

	PHYTMAC_WRITE(pdata, PHYTMAC_DCONFIG, dmaconfig);

	if (pdata->capacities & PHYTMAC_CAPS_TAILPTR)
		ptrconfig |= PHYTMAC_BIT(TXPTR_EN);
	if (pdata->capacities & PHYTMAC_CAPS_RXPTR)
		ptrconfig |= PHYTMAC_BIT(RXPTR_EN);

	PHYTMAC_WRITE(pdata, PHYTMAC_TAIL_ENABLE, ptrconfig);

	if (pdata->capacities & PHYTMAC_CAPS_START)
		PHYTMAC_WRITE(pdata, PHYTMAC_TXSTARTSEL, PHYTMAC_BIT(TSTARTSEL_ENABLE));

	if (phy_interface_mode_is_8023z(pdata->phy_interface))
		phytmac_pcs_software_reset(pdata, 1);

	return 0;
}

static int phytmac_powerup_hw(struct phytmac *pdata, int on)
{
	u32 status, data0, data1, rdata1;
	int ret;

	if (!(pdata->capacities & PHYTMAC_CAPS_PWCTRL)) {
		pdata->power_state = on;
		return 0;
	}

	if (has_acpi_companion(pdata->dev)) {
#ifdef CONFIG_ACPI
		acpi_handle handle;
		union acpi_object args[3];
		struct acpi_object_list arg_list = {
			.pointer = args,
			.count = ARRAY_SIZE(args),
		};
		acpi_status acpi_sts;
		unsigned long long rv;

		handle = ACPI_HANDLE(pdata->dev);

		netdev_info(pdata->ndev, "set gmac power %s\n",
			    on == PHYTMAC_POWERON ? "on" : "off");
		args[0].type = ACPI_TYPE_INTEGER;
		args[0].integer.value = PHYTMAC_PWCTL_GMAC_ID;
		args[1].type = ACPI_TYPE_INTEGER;
		args[1].integer.value = PHYTMAC_PWCTL_DEFAULT_VAL;
		args[2].type = ACPI_TYPE_INTEGER;
		args[2].integer.value = PHYTMAC_PWCTL_DEFAULT_VAL;

		if (on == PHYTMAC_POWERON) {
			acpi_sts = acpi_evaluate_integer(handle, "PPWO", &arg_list, &rv);
			if (ACPI_FAILURE(acpi_sts))
				netdev_err(pdata->ndev, "NO PPWO Method\n");
			if (rv)
				netdev_err(pdata->ndev, "Failed to power on\n");
		} else {
			acpi_sts = acpi_evaluate_integer(handle, "PPWD", &arg_list, &rv);
			if (ACPI_FAILURE(acpi_sts))
				netdev_err(pdata->ndev, "NO PPWD Method\n");
			if (rv)
				netdev_err(pdata->ndev, "Failed to power off\n");
		}
#endif
	} else {
		ret = readx_poll_timeout(PHYTMAC_READ_STAT, pdata, status, !status,
					 1, PHYTMAC_TIMEOUT);
		if (ret)
			netdev_err(pdata->ndev, "mnh status is busy, status=%x\n", status);

		ret = readx_poll_timeout(PHYTMAC_READ_DATA0, pdata, data0,
					 data0 & PHYTMAC_BIT(DATA0_FREE),
					 1, PHYTMAC_TIMEOUT);
		if (ret)
			netdev_err(pdata->ndev, "mnh data0 is busy, data0=%x\n", data0);

		data0 = 0;
		data0 = PHYTMAC_SET_BITS(data0, DATA0_MSG, PHYTMAC_MSG_PM);
		data0 = PHYTMAC_SET_BITS(data0, DATA0_PRO, PHYTMAC_PRO_ID);
		PHYTMAC_MHU_WRITE(pdata, PHYTMAC_MHU_CPP_DATA0, data0);
		data1 = 0;

		if (on == PHYTMAC_POWERON) {
			data1 = PHYTMAC_SET_BITS(data1, DATA1_STAT, PHYTMAC_STATON);
			data1 = PHYTMAC_SET_BITS(data1, DATA1_STATTYPE, PHYTMAC_STATTYPE);
			PHYTMAC_MHU_WRITE(pdata, PHYTMAC_MHU_CPP_DATA1, data1);
		} else {
			data1 = PHYTMAC_SET_BITS(data1, DATA1_STAT, PHYTMAC_STATOFF);
			data1 = PHYTMAC_SET_BITS(data1, DATA1_STATTYPE, PHYTMAC_STATTYPE);
			PHYTMAC_MHU_WRITE(pdata, PHYTMAC_MHU_CPP_DATA1, data1);
		}

		PHYTMAC_MHU_WRITE(pdata, PHYTMAC_MHU_AP_CPP_SET, 1);
		ret = readx_poll_timeout(PHYTMAC_READ_DATA0, pdata, data0,
					 data0 & PHYTMAC_BIT(DATA0_FREE),
					 1, PHYTMAC_TIMEOUT);
		if (ret)
			netdev_err(pdata->ndev, "mnh data0 is busy\n");

		rdata1 = PHYTMAC_MHU_READ(pdata, PHYTMAC_MHU_CPP_DATA1);
		if (rdata1 == data1)
			netdev_err(pdata->ndev, "gmac power %s success, data1 = %x, rdata1=%x\n",
				   on == PHYTMAC_POWERON ? "up" : "down", data1, rdata1);
		else
			netdev_err(pdata->ndev, "gmac power %s failed, data1 = %x, rdata1=%x\n",
				   on == PHYTMAC_POWERON ? "up" : "down", data1, rdata1);
	}

	pdata->power_state = on;

	return 0;
}

static int phytmac_set_wake(struct phytmac *pdata, int wake)
{
	u32 value = 0;

	if (wake & PHYTMAC_WAKE_MAGIC)
		value |= PHYTMAC_BIT(MAGIC);
	if (wake & PHYTMAC_WAKE_ARP)
		value |= PHYTMAC_BIT(ARP);
	if (wake & PHYTMAC_WAKE_UCAST)
		value |= PHYTMAC_BIT(UCAST);
	if (wake & PHYTMAC_WAKE_MCAST)
		value |= PHYTMAC_BIT(MCAST);

	PHYTMAC_WRITE(pdata, PHYTMAC_WOL, value);
	if (wake) {
		PHYTMAC_WRITE(pdata, PHYTMAC_IE, PHYTMAC_BIT(WOL_RECEIVE_ENABLE));
		value = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG) | PHYTMAC_BIT(IGNORE_RX_FCS);
		PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, value);
	} else {
		PHYTMAC_WRITE(pdata, PHYTMAC_ID, PHYTMAC_BIT(WOL_RECEIVE_DISABLE));
		value = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG) & ~PHYTMAC_BIT(IGNORE_RX_FCS);
		PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, value);
	}

	return 0;
}

static int phytmac_mdio_idle(struct phytmac *pdata)
{
	u32 val;
	int ret;

	/* wait for end of transfer */
	ret = readx_poll_timeout(PHYTMAC_READ_NSTATUS, pdata, val, val & PHYTMAC_BIT(MDIO_IDLE),
				 1, PHYTMAC_MDIO_TIMEOUT);
	if (ret)
		netdev_err(pdata->ndev, "mdio wait for idle time out!");

	return ret;
}

static int phytmac_mdio_data_read_c22(struct phytmac *pdata, int mii_id, int regnum)
{
	u16 data;

	PHYTMAC_WRITE(pdata, PHYTMAC_MDATA, (PHYTMAC_BITS(CLAUSE_SEL, PHYTMAC_CLAUSE_C22)
		      | PHYTMAC_BITS(OPS, PHYTMAC_OPS_C22_READ)
		      | PHYTMAC_BITS(PHY_ADDR, mii_id)
		      | PHYTMAC_BITS(REG_ADDR, regnum)
		      | PHYTMAC_BITS(MUST, 2)));

	phytmac_mdio_idle(pdata);
	data = PHYTMAC_READ(pdata, PHYTMAC_MDATA) & 0xffff;
	phytmac_mdio_idle(pdata);

	return data;
}

static int phytmac_mdio_data_write_c22(struct phytmac *pdata, int mii_id,
				       int regnum,  u16 data)
{
	PHYTMAC_WRITE(pdata, PHYTMAC_MDATA, (PHYTMAC_BITS(CLAUSE_SEL, PHYTMAC_CLAUSE_C22)
		      | PHYTMAC_BITS(OPS, PHYTMAC_OPS_C22_WRITE)
		      | PHYTMAC_BITS(PHY_ADDR, mii_id)
		      | PHYTMAC_BITS(REG_ADDR, regnum)
		      | PHYTMAC_BITS(DATA, data)
		      | PHYTMAC_BITS(MUST, 2)));
	phytmac_mdio_idle(pdata);

	return 0;
}

static int phytmac_mdio_data_read_c45(struct phytmac *pdata, int mii_id, int devad, int regnum)
{
	u16 data;

	PHYTMAC_WRITE(pdata, PHYTMAC_MDATA, (PHYTMAC_BITS(CLAUSE_SEL, PHYTMAC_CLAUSE_C45)
		      | PHYTMAC_BITS(OPS, PHYTMAC_OPS_C45_ADDR)
		      | PHYTMAC_BITS(PHY_ADDR, mii_id)
		      | PHYTMAC_BITS(REG_ADDR, devad & 0x1F)
		      | PHYTMAC_BITS(DATA, regnum & 0xFFFF)
		      | PHYTMAC_BITS(MUST, 2)));
	phytmac_mdio_idle(pdata);
	PHYTMAC_WRITE(pdata, PHYTMAC_MDATA, (PHYTMAC_BITS(CLAUSE_SEL, PHYTMAC_CLAUSE_C45)
		      | PHYTMAC_BITS(OPS, PHYTMAC_OPS_C45_READ)
		      | PHYTMAC_BITS(PHY_ADDR, mii_id)
		      | PHYTMAC_BITS(REG_ADDR, devad & 0x1F)
		      | PHYTMAC_BITS(DATA, regnum & 0xFFFF)
		      | PHYTMAC_BITS(MUST, 2)));

	phytmac_mdio_idle(pdata);
	data = PHYTMAC_READ(pdata, PHYTMAC_MDATA) & 0xffff;
	phytmac_mdio_idle(pdata);

	return data;
}

static int phytmac_mdio_data_write_c45(struct phytmac *pdata, int mii_id, int devad,
				       int regnum, u16 data)
{
	PHYTMAC_WRITE(pdata, PHYTMAC_MDATA, (PHYTMAC_BITS(CLAUSE_SEL, PHYTMAC_CLAUSE_C45)
		      | PHYTMAC_BITS(OPS, PHYTMAC_OPS_C45_ADDR)
		      | PHYTMAC_BITS(PHY_ADDR, mii_id)
		      | PHYTMAC_BITS(REG_ADDR, devad & 0x1F)
		      | PHYTMAC_BITS(DATA, regnum & 0xFFFF)
		      | PHYTMAC_BITS(MUST, 2)));
	phytmac_mdio_idle(pdata);
	PHYTMAC_WRITE(pdata, PHYTMAC_MDATA, (PHYTMAC_BITS(CLAUSE_SEL, PHYTMAC_CLAUSE_C45)
		      | PHYTMAC_BITS(OPS, PHYTMAC_OPS_C45_WRITE)
		      | PHYTMAC_BITS(PHY_ADDR, mii_id)
		      | PHYTMAC_BITS(REG_ADDR, devad & 0x1F)
		      | PHYTMAC_BITS(DATA, data)
		      | PHYTMAC_BITS(MUST, 2)));
	phytmac_mdio_idle(pdata);

	return 0;
}

static int phytmac_get_feature_all(struct phytmac *pdata)
{
	unsigned int queue_mask;
	unsigned int num_queues;
	int val;

	/* get max queues */
	queue_mask = 0x1;
	queue_mask |= PHYTMAC_READ(pdata, PHYTMAC_DEFAULT2) & 0xff;
	num_queues = hweight32(queue_mask);
	pdata->queues_max_num = num_queues;

	/* get dma desc prefetch number */
	val = PHYTMAC_READ_BITS(pdata, PHYTMAC_DEFAULT4, TXDESCRD);
	if (val)
		pdata->tx_bd_prefetch = (2 << (val - 1)) *
					sizeof(struct phytmac_dma_desc);

	val = PHYTMAC_READ_BITS(pdata, PHYTMAC_DEFAULT4, RXDESCRD);
	if (val)
		pdata->rx_bd_prefetch = (2 << (val - 1)) *
					sizeof(struct phytmac_dma_desc);

	/* dma bus width */
	pdata->dma_data_width = PHYTMAC_READ_BITS(pdata, PHYTMAC_DEFAULT1, DBW);

	/* dma addr width */
	if (PHYTMAC_READ_BITS(pdata, PHYTMAC_DEFAULT2, DAW64))
		pdata->dma_addr_width = 64;
	else
		pdata->dma_addr_width = 32;

	/* max rx fs */
	pdata->max_rx_fs = PHYTMAC_READ_BITS(pdata, PHYTMAC_DEFAULT3, SCR2CMP);

	if (pdata->version == VERSION_V3)
		pdata->capacities |= PHYTMAC_CAPS_RXPTR;

	if (netif_msg_hw(pdata))
		netdev_info(pdata->ndev, "get feature queue_num=%d, daw=%d, dbw=%d, rx_fs=%d, rx_bd=%d, tx_bd=%d\n",
			    pdata->queues_num, pdata->dma_addr_width, pdata->dma_data_width,
			    pdata->max_rx_fs, pdata->rx_bd_prefetch, pdata->tx_bd_prefetch);
	return 0;
}

static int phytmac_add_fdir_entry(struct phytmac *pdata, struct ethtool_rx_flow_spec *rx_flow)
{
	struct ethtool_tcpip4_spec *tp4sp_v, *tp4sp_m;
	u16 index = rx_flow->location;
	u32 tmp, fdir_ctrl;
	bool ipsrc = false;
	bool ipdst = false;
	bool port = false;

	tp4sp_v = &rx_flow->h_u.tcp_ip4_spec;
	tp4sp_m = &rx_flow->m_u.tcp_ip4_spec;

	if (tp4sp_m->ip4src == 0xFFFFFFFF) {
		tmp = 0;
		tmp = PHYTMAC_SET_BITS(tmp, OFFSET, ETHTYPE_SIP_OFFSET);
		tmp = PHYTMAC_SET_BITS(tmp, OFFSET_TYPE, PHYTMAC_OFFSET_AFTER_L2HEAD);
		tmp = PHYTMAC_SET_BITS(tmp, DIS_MASK, 1);
		PHYTMAC_WRITE(pdata, PHYTMAC_COMP1(3 * index), tp4sp_v->ip4src);
		PHYTMAC_WRITE(pdata, PHYTMAC_COMP2(3 * index), tmp);
		ipsrc = true;
	}

	if (tp4sp_m->ip4dst == 0xFFFFFFFF) {
		/* 2nd compare reg - IP destination address */
		tmp = 0;
		tmp = PHYTMAC_SET_BITS(tmp, OFFSET, ETHTYPE_DIP_OFFSET);
		tmp = PHYTMAC_SET_BITS(tmp, OFFSET_TYPE, PHYTMAC_OFFSET_AFTER_L2HEAD);
		tmp = PHYTMAC_SET_BITS(tmp, DIS_MASK, 1);
		PHYTMAC_WRITE(pdata, PHYTMAC_COMP1(3 * index + 1), tp4sp_v->ip4dst);
		PHYTMAC_WRITE(pdata, PHYTMAC_COMP2(3 * index + 1), tmp);
		ipdst = true;
	}

	if (tp4sp_m->psrc == 0xFFFF || tp4sp_m->pdst == 0xFFFF) {
		tmp = 0;
		tmp = PHYTMAC_SET_BITS(tmp, OFFSET_TYPE, PHYTMAC_OFFSET_AFTER_L3HEAD);
		if (tp4sp_m->psrc == tp4sp_m->pdst) {
			tmp = PHYTMAC_SET_BITS(tmp, DIS_MASK, 1);
			tmp = PHYTMAC_SET_BITS(tmp, OFFSET, IPHEAD_SPORT_OFFSET);
			PHYTMAC_WRITE(pdata, PHYTMAC_COMP1(3 * index + 2),
				      tp4sp_v->psrc | (u32)(tp4sp_v->pdst << 16));
		} else {
			tmp = PHYTMAC_SET_BITS(tmp, DIS_MASK, 0);
			if (tp4sp_m->psrc == 0xFFFF) { /* src port */
				tmp = PHYTMAC_SET_BITS(tmp, OFFSET, IPHEAD_SPORT_OFFSET);
				PHYTMAC_WRITE(pdata, PHYTMAC_COMP1(3 * index + 2),
					      tp4sp_m->psrc | (u32)(tp4sp_v->psrc << 16));
			} else { /* dst port */
				tmp = PHYTMAC_SET_BITS(tmp, OFFSET, IPHEAD_DPORT_OFFSET);
				PHYTMAC_WRITE(pdata, PHYTMAC_COMP1(3 * index + 2),
					      tp4sp_m->pdst | (u32)(tp4sp_v->pdst << 16));
			}
		}
		PHYTMAC_WRITE(pdata, PHYTMAC_COMP2(3 * index + 2), tmp);
		port = true;
	}

	fdir_ctrl = PHYTMAC_READ(pdata, PHYTMAC_FDIR(rx_flow->location));

	if (ipsrc) {
		fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, CA, 3 * index);
		fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, CA_EN, 1);
	}

	if (ipdst) {
		fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, CB, 3 * index + 1);
		fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, CB_EN, 1);
	}

	if (port) {
		fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, CC, 3 * index + 2);
		fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, CC_EN, 1);
	}

	fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, QUEUE_NUM, (rx_flow->ring_cookie) & 0xFF);
	fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, ETH_TYPE, 0);
	fdir_ctrl = PHYTMAC_SET_BITS(fdir_ctrl, ETH_EN, 1);

	PHYTMAC_WRITE(pdata, PHYTMAC_FDIR(rx_flow->location), fdir_ctrl);

	return 0;
}

static int phytmac_del_fdir_entry(struct phytmac *pdata, struct ethtool_rx_flow_spec *rx_flow)
{
	int i;
	int index = rx_flow->location;

	PHYTMAC_WRITE(pdata, PHYTMAC_FDIR(index), 0);
	for (i = 0; i < 3; i++) {
		PHYTMAC_WRITE(pdata, PHYTMAC_COMP1(3 * index + i), 0);
		PHYTMAC_WRITE(pdata, PHYTMAC_COMP2(3 * index + i), 0);
	}
	return 0;
}

static int phytmac_init_ring_hw(struct phytmac *pdata)
{
	struct phytmac_queue *queue;
	unsigned int q = 0;
	u32 buffer_size = pdata->rx_buffer_len / 64;

	for (q = 0, queue = pdata->queues; q < pdata->queues_num; ++q, ++queue) {
		if (q == 0) {
			PHYTMAC_WRITE(pdata, PHYTMAC_RXPTR_Q0,
				      lower_32_bits(queue->rx_ring_addr));
			PHYTMAC_WRITE(pdata, PHYTMAC_TXPTR_Q0,
				      lower_32_bits(queue->tx_ring_addr));
			PHYTMAC_WRITE(pdata, PHYTMAC_DCONFIG,
				      PHYTMAC_SET_BITS(PHYTMAC_READ(pdata, PHYTMAC_DCONFIG),
						       RX_BUF_LEN, buffer_size));
		} else {
			PHYTMAC_WRITE(pdata, PHYTMAC_RXPTR(q - 1),
				      lower_32_bits(queue->rx_ring_addr));
			PHYTMAC_WRITE(pdata, PHYTMAC_TXPTR(q - 1),
				      lower_32_bits(queue->tx_ring_addr));
			PHYTMAC_WRITE(pdata, PHYTMAC_RBQS(q - 1), buffer_size);
		}

		PHYTMAC_WRITE(pdata, PHYTMAC_TXPTRH(q), upper_32_bits(queue->tx_ring_addr));
		PHYTMAC_WRITE(pdata, PHYTMAC_RXPTRH(q), upper_32_bits(queue->rx_ring_addr));

		if (pdata->capacities & PHYTMAC_CAPS_TAILPTR)
			PHYTMAC_WRITE(pdata, PHYTMAC_TX_TAILPTR(q), queue->tx_tail);
	}

	return 0;
}

static u32 phytmac_get_irq_mask(u32 mask)
{
	u32 value = 0;

	value |= (mask & PHYTMAC_INT_TX_COMPLETE) ? PHYTMAC_BIT(TXCOMP) : 0;
	value |= (mask & PHYTMAC_INT_TX_ERR) ? PHYTMAC_BIT(BUS_ERR) : 0;
	value |= (mask & PHYTMAC_INT_RX_COMPLETE) ? PHYTMAC_BIT(RXCOMP) : 0;
	value |= (mask & PHYTMAC_INT_RX_OVERRUN) ? PHYTMAC_BIT(RXOVERRUN) : 0;
	value |= (mask & PHYTMAC_INT_RX_DESC_FULL) ? PHYTMAC_BIT(RUB) : 0;

	return value;
}

static u32 phytmac_get_irq_status(u32 value)
{
	u32 status = 0;

	status |= (value & PHYTMAC_BIT(TXCOMP)) ? PHYTMAC_INT_TX_COMPLETE : 0;
	status |= (value & PHYTMAC_BIT(BUS_ERR)) ? PHYTMAC_INT_TX_ERR : 0;
	status |= (value & PHYTMAC_BIT(RXCOMP)) ? PHYTMAC_INT_RX_COMPLETE : 0;
	status |= (value & PHYTMAC_BIT(RXOVERRUN)) ? PHYTMAC_INT_RX_OVERRUN : 0;
	status |= (value & PHYTMAC_BIT(RUB)) ? PHYTMAC_INT_RX_DESC_FULL : 0;

	return status;
}

static void phytmac_enable_irq(struct phytmac *pdata,
			       int queue_index, u32 mask)
{
	u32 value;

	value = phytmac_get_irq_mask(mask);

	if (queue_index == 0)
		PHYTMAC_WRITE(pdata, PHYTMAC_IE, value);
	else
		PHYTMAC_WRITE(pdata, PHYTMAC_IER(queue_index - 1), value);
}

static void phytmac_disable_irq(struct phytmac *pdata,
				int queue_index, u32 mask)
{
	u32 value;

	value = phytmac_get_irq_mask(mask);

	if (queue_index == 0)
		PHYTMAC_WRITE(pdata, PHYTMAC_ID, value);
	else
		PHYTMAC_WRITE(pdata, PHYTMAC_IDR(queue_index - 1), value);
}

static void phytmac_clear_irq(struct phytmac *pdata,
			      int queue_index, u32 mask)
{
	u32 value;

	value = phytmac_get_irq_mask(mask);

	if (queue_index == 0)
		PHYTMAC_WRITE(pdata, PHYTMAC_IS, value);
	else
		PHYTMAC_WRITE(pdata, PHYTMAC_ISR(queue_index - 1), value);
}

static unsigned int phytmac_get_intx_mask(struct phytmac *pdata)
{
	u32 value;

	value = PHYTMAC_READ(pdata, PHYTMAC_INTX_IRQ_MASK);
	PHYTMAC_WRITE(pdata, PHYTMAC_INTX_IRQ_MASK, value);

	return value;
}

static unsigned int phytmac_get_irq(struct phytmac *pdata, int queue_index)
{
	u32 status, value;

	if (queue_index == 0)
		value = PHYTMAC_READ(pdata, PHYTMAC_IS);
	else
		value = PHYTMAC_READ(pdata, PHYTMAC_ISR(queue_index - 1));

	status = phytmac_get_irq_status(value);

	return status;
}

static unsigned int phytmac_tx_map_desc(struct phytmac_queue *queue,
					u32 tx_tail, struct packet_info *packet)
{
	unsigned int i, ctrl;
	struct phytmac *pdata = queue->pdata;
	struct phytmac_dma_desc *desc;
	struct phytmac_tx_skb *tx_skb;
	unsigned int eof = 1;

	i = tx_tail;

	if (!(pdata->capacities & PHYTMAC_CAPS_TAILPTR)) {
		ctrl = PHYTMAC_BIT(TX_USED);
		desc = phytmac_get_tx_desc(queue, i);
		desc->desc1 = ctrl;
	}

	do {
		i--;
		tx_skb = phytmac_get_tx_skb(queue, i);
		desc = phytmac_get_tx_desc(queue, i);

		ctrl = (u32)tx_skb->length;
		if (eof) {
			ctrl |= PHYTMAC_BIT(TX_LAST);
			eof = 0;
		}

		if (unlikely(i == (pdata->tx_ring_size - 1)))
			ctrl |= PHYTMAC_BIT(TX_WRAP);

		if (i == queue->tx_tail) {
			ctrl |= PHYTMAC_BITS(TX_LSO, packet->lso);
			ctrl |= PHYTMAC_BITS(TX_TCP_SEQ_SRC, packet->seq);
			if (packet->nocrc)
				ctrl |= PHYTMAC_BIT(TX_NOCRC);
		} else {
			ctrl |= PHYTMAC_BITS(MSS_MFS, packet->mss);
		}

		desc->desc2 = upper_32_bits(tx_skb->addr);
		desc->desc0 = lower_32_bits(tx_skb->addr);
		/* Make newly descriptor visible to hardware */
		wmb();
		desc->desc1 = ctrl;
	} while (i != queue->tx_tail);

	return 0;
}

static void phytmac_init_rx_map_desc(struct phytmac_queue *queue,
				     u32 index)
{
	struct phytmac_dma_desc *desc;

	desc = phytmac_get_rx_desc(queue, index);

	desc->desc1 = 0;
	/* Make newly descriptor to hardware */
	dma_wmb();
	desc->desc0 |= PHYTMAC_BIT(RX_USED);
}

static unsigned int phytmac_rx_map_desc(struct phytmac_queue *queue,
					u32 index, dma_addr_t addr)
{
	struct phytmac *pdata = queue->pdata;
	struct phytmac_dma_desc *desc;

	desc = phytmac_get_rx_desc(queue, index);

	if (addr) {
		if (unlikely(index == (pdata->rx_ring_size - 1)))
			addr |= PHYTMAC_BIT(RX_WRAP);
		desc->desc1 = 0;
		desc->desc2 = upper_32_bits(addr);
		/* Make newly descriptor to hardware */
		if (!(pdata->capacities & PHYTMAC_CAPS_RXPTR))
			dma_wmb();
		desc->desc0 = lower_32_bits(addr);
	} else {
		desc->desc1 = 0;
		/* Make newly descriptor to hardware */
		dma_wmb();
		desc->desc0 &= ~PHYTMAC_BIT(RX_USED);
	}

	return 0;
}

static unsigned int phytmac_zero_rx_desc_addr(struct phytmac_dma_desc *desc)
{
	desc->desc2 = 0;
	desc->desc0 = (desc->desc0 & PHYTMAC_BIT(RX_TS_VALID)) | PHYTMAC_BIT(RX_USED);

	return 0;
}

static unsigned int phytmac_zero_tx_desc(struct phytmac_dma_desc *desc)
{
	desc->desc2 = 0;
	desc->desc0 = 0;
	desc->desc1 &= ~PHYTMAC_BIT(TX_USED);

	return 0;
}

static void phytmac_tx_start(struct phytmac_queue *queue)
{
	struct phytmac *pdata = queue->pdata;

	if (pdata->capacities & PHYTMAC_CAPS_TAILPTR)
		PHYTMAC_WRITE(pdata, PHYTMAC_TX_TAILPTR(queue->index),
			      PHYTMAC_BIT(TXTAIL_UPDATE) | queue->tx_tail);

	if (likely(pdata->capacities & PHYTMAC_CAPS_START))
		PHYTMAC_WRITE(pdata, PHYTMAC_TXSTART, PHYTMAC_BIT(TSTART));
}

static void phytmac_update_rx_tail(struct phytmac_queue *queue)
{
	struct phytmac *pdata = queue->pdata;

	if (pdata->capacities & PHYTMAC_CAPS_RXPTR)
		PHYTMAC_WRITE(pdata, PHYTMAC_RX_TAILPTR(queue->index), queue->rx_head);
}

static int phytmac_tx_complete(const struct phytmac_dma_desc *desc)
{
	return PHYTMAC_GET_BITS(desc->desc1, TX_USED);
}

static bool phytmac_rx_complete(const struct phytmac_dma_desc *desc)
{
	dma_addr_t addr;
	bool used;

	used = desc->desc0 & PHYTMAC_BIT(RX_USED);
	addr = ((u64)(desc->desc2) << 32);
	addr |= desc->desc0 & 0xfffffff8;

	if (used != 0 && addr != 0)
		return true;
	else
		return false;
}

static int phytmac_rx_pkt_len(struct phytmac *pdata, const struct phytmac_dma_desc *desc)
{
	if (pdata->capacities & PHYTMAC_CAPS_JUMBO)
		return desc->desc1 & PHYTMAC_JUMBO_FRAME_MASK;
	else
		return desc->desc1 & PHYTMAC_FRAME_MASK;
}

static bool phytmac_rx_checksum(const struct phytmac_dma_desc *desc)
{
	u32 value = desc->desc1;
	u32 check = value >> PHYTMAC_RX_CSUM_INDEX & 0x3;

	return (check == PHYTMAC_RX_CSUM_IP_TCP || check == PHYTMAC_RX_CSUM_IP_UDP);
}

static bool phytmac_rx_single_buffer(const struct phytmac_dma_desc *desc)
{
	u32 value = desc->desc1;

	return ((value & PHYTMAC_BIT(RX_SOF)) && (value & PHYTMAC_BIT(RX_EOF)));
}

static bool phytmac_rx_sof(const struct phytmac_dma_desc *desc)
{
	u32 value = desc->desc1;

	return (value & PHYTMAC_BIT(RX_SOF));
}

static bool phytmac_rx_eof(const struct phytmac_dma_desc *desc)
{
	u32 value = desc->desc1;

	return (value & PHYTMAC_BIT(RX_EOF));
}

static void phytmac_clear_rx_desc(struct phytmac_queue *queue, int begin, int end)
{
	unsigned int frag;
	unsigned int tmp = end;
	struct phytmac_dma_desc *desc;

	if (begin > end)
		tmp = end + queue->pdata->rx_ring_size;

	for (frag = begin; frag != tmp; frag++) {
		desc = phytmac_get_rx_desc(queue, frag);
		desc->desc0 &= ~PHYTMAC_BIT(RX_USED);
	}
}

static void phytmac_mac_interface_config(struct phytmac *pdata, unsigned int mode,
					 const struct phylink_link_state *state)
{
	u32 old_ctrl, old_config;
	u32 ctrl, config, usxctl;

	old_ctrl = PHYTMAC_READ(pdata, PHYTMAC_NCTRL);
	old_config = PHYTMAC_READ(pdata, PHYTMAC_NCONFIG);
	ctrl = old_ctrl;
	config = old_config;

	if (state->speed == SPEED_10000)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_10000M);
	else if (state->speed == SPEED_5000)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_5000M);
	else if (state->speed == SPEED_2500)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_2500M);
	else if (state->speed == SPEED_1000)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_1000M);
	else if (state->speed == SPEED_100 || state->speed == SPEED_10)
		PHYTMAC_WRITE(pdata, PHYTMAC_HCONFIG, PHYTMAC_SPEED_100M);

	config &= ~(PHYTMAC_BIT(SGMII_EN) | PHYTMAC_BIT(PCS_EN)
		    | PHYTMAC_BIT(SPEED) | PHYTMAC_BIT(FD) | PHYTMAC_BIT(GM_EN));
	ctrl &= ~(PHYTMAC_BIT(HIGHSPEED) | PHYTMAC_BIT(2PT5G));

	if (state->interface == PHY_INTERFACE_MODE_SGMII) {
		config |= PHYTMAC_BIT(SGMII_EN) | PHYTMAC_BIT(PCS_EN);
		if (state->speed == SPEED_1000)
			config |= PHYTMAC_BIT(GM_EN);
		else if (state->speed == SPEED_2500) {
			ctrl |= PHYTMAC_BIT(2PT5G);
			config |= PHYTMAC_BIT(GM_EN);
		}
	} else if (state->interface == PHY_INTERFACE_MODE_1000BASEX) {
		config |= PHYTMAC_BIT(PCS_EN) | PHYTMAC_BIT(GM_EN);
	} else if (state->interface == PHY_INTERFACE_MODE_2500BASEX) {
		ctrl |= PHYTMAC_BIT(2PT5G);
		config |= PHYTMAC_BIT(PCS_EN) | PHYTMAC_BIT(GM_EN);
	} else if (state->interface == PHY_INTERFACE_MODE_10GBASER ||
		   state->interface == PHY_INTERFACE_MODE_USXGMII ||
		   state->interface == PHY_INTERFACE_MODE_5GBASER) {
		usxctl = PHYTMAC_READ(pdata, PHYTMAC_USXCTRL);
		if (state->speed == SPEED_10000) {
			usxctl = PHYTMAC_SET_BITS(usxctl, SERDES_RATE, PHYTMAC_SERDES_RATE_10G);
			usxctl = PHYTMAC_SET_BITS(usxctl, USX_SPEED, PHYTMAC_SPEED_10000M);
		} else if (state->speed == SPEED_5000) {
			usxctl = PHYTMAC_SET_BITS(usxctl, SERDES_RATE, PHYTMAC_SERDES_RATE_5G);
			usxctl = PHYTMAC_SET_BITS(usxctl, USX_SPEED, PHYTMAC_SPEED_5000M);
		}
		usxctl |= PHYTMAC_BIT(RX_EN) | PHYTMAC_BIT(TX_EN);
		PHYTMAC_WRITE(pdata, PHYTMAC_USXCTRL, usxctl);

		config |= PHYTMAC_BIT(PCS_EN);
		ctrl |= PHYTMAC_BIT(HIGHSPEED);
	}

	if (state->duplex)
		config |= PHYTMAC_BIT(FD);

	if (old_ctrl ^ ctrl)
		PHYTMAC_WRITE(pdata, PHYTMAC_NCTRL, ctrl);

	if (old_config ^ config)
		PHYTMAC_WRITE(pdata, PHYTMAC_NCONFIG, config);

	/* Disable AN for SGMII fixed link or speed equal to 2.5G, enable otherwise.*/
	if (state->interface == PHY_INTERFACE_MODE_SGMII) {
		if (mode == MLO_AN_FIXED)
			phytmac_enable_autoneg(pdata, 0);
		else
			phytmac_enable_autoneg(pdata, 1);
	}
	if (state->interface == PHY_INTERFACE_MODE_1000BASEX)
		phytmac_enable_autoneg(pdata, 1);
	if (state->interface == PHY_INTERFACE_MODE_2500BASEX)
		phytmac_enable_autoneg(pdata, 0);
}

static unsigned int phytmac_pcs_get_link(struct phytmac *pdata,
					 phy_interface_t interface)
{
	if (interface == PHY_INTERFACE_MODE_SGMII ||
	    interface == PHY_INTERFACE_MODE_1000BASEX ||
	    interface == PHY_INTERFACE_MODE_2500BASEX)
		return PHYTMAC_READ_BITS(pdata, PHYTMAC_NSTATUS, PCS_LINK);
	else if (interface == PHY_INTERFACE_MODE_USXGMII ||
		 interface == PHY_INTERFACE_MODE_10GBASER)
		return PHYTMAC_READ_BITS(pdata, PHYTMAC_USXSTATUS, USX_PCS_LINK);

	return 0;
}

static void phytmac_clear_tx_desc(struct phytmac_queue *queue)
{
	struct phytmac *pdata = queue->pdata;
	struct phytmac_dma_desc *desc = NULL;
	int i;

	for (i = 0; i < queue->pdata->tx_ring_size; i++) {
		desc = phytmac_get_tx_desc(queue, i);
		desc->desc2 = 0;
		desc->desc0 = 0;
		/* make newly desc1 to hardware */
		wmb();
		desc->desc1 = PHYTMAC_BIT(TX_USED);
	}
	desc->desc1 |= PHYTMAC_BIT(TX_WRAP);

	if (pdata->capacities & PHYTMAC_CAPS_TAILPTR)
		PHYTMAC_WRITE(pdata, PHYTMAC_TX_TAILPTR(queue->index), queue->tx_tail);
}

static void phytmac_get_hw_stats(struct phytmac *pdata)
{
	u32 stats[PHYTMAC_STATIS_REG_NUM];
	int i, j;
	u64 val;
	u64 *p = &pdata->stats.tx_octets;

	for (i = 0 ; i < PHYTMAC_STATIS_REG_NUM; i++)
		stats[i] = PHYTMAC_READ(pdata, PHYTMAC_OCTTX + i * 4);

	for (i = 0, j = 0; i < PHYTMAC_STATIS_REG_NUM; i++) {
		if (i == 0 || i == 20) {
			val = (u64)stats[i + 1] << 32 | stats[i];
			*p += val;
			pdata->ethtool_stats[j] = *p;
			++j;
			++p;
		} else {
			if (i != 1 && i != 21) {
				val = stats[i];
				*p += val;
				pdata->ethtool_stats[j] = *p;
				++j;
				++p;
			}
		}
	}
}

static void phytmac_get_time(struct phytmac *pdata, struct timespec64 *ts)
{
	u32 ns, secl, sech;

	ns = PHYTMAC_READ(pdata, PHYTMAC_TN);
	secl = PHYTMAC_READ(pdata, PHYTMAC_TSL);
	sech = PHYTMAC_READ(pdata, PHYTMAC_TSH);

	ts->tv_nsec = ns;
	ts->tv_sec = (((u64)sech << 32) | secl) & SEC_MAX_VAL;
}

static void phytmac_set_time(struct phytmac *pdata, time64_t sec, long nsec)
{
	u32 secl, sech;

	secl = (u32)sec;
	sech = (sec >> 32) & (0xffff);

	PHYTMAC_WRITE(pdata, PHYTMAC_TN, 0);
	PHYTMAC_WRITE(pdata, PHYTMAC_TSH, sech);
	PHYTMAC_WRITE(pdata, PHYTMAC_TSL, secl);
	PHYTMAC_WRITE(pdata, PHYTMAC_TN, nsec);
}

static void phytmac_clear_time(struct phytmac *pdata)
{
	u32 value;

	pdata->ts_incr.sub_ns = 0;
	pdata->ts_incr.ns = 0;

	value = PHYTMAC_READ(pdata, PHYTMAC_TISN);
	value = PHYTMAC_SET_BITS(value, SUB_NSECL, 0);
	value = PHYTMAC_SET_BITS(value, SUB_NSECH, 0);
	PHYTMAC_WRITE(pdata, PHYTMAC_TISN, value);

	value = PHYTMAC_READ(pdata, PHYTMAC_TI);
	value = PHYTMAC_SET_BITS(value, INCR_NS, 0);
	PHYTMAC_WRITE(pdata, PHYTMAC_TI, value);

	PHYTMAC_WRITE(pdata, PHYTMAC_TA, 0);
}

static int phytmac_set_tsmode(struct phytmac *pdata, struct ts_ctrl *ctrl)
{
	if (ctrl->rx_control == TS_ALL_PTP_FRAMES)
		PHYTMAC_WRITE(pdata, PHYTMAC_NCTRL,
			      PHYTMAC_READ(pdata, PHYTMAC_NCTRL) | PHYTMAC_BIT(STORE_RX_TS));

	PHYTMAC_WRITE(pdata, PHYTMAC_TXBDCTRL, PHYTMAC_BITS(TX_TSMODE, ctrl->tx_control));
	PHYTMAC_WRITE(pdata, PHYTMAC_RXBDCTRL, PHYTMAC_BITS(RX_TSMODE, ctrl->rx_control));

	return 0;
}

static int phytmac_set_tsincr(struct phytmac *pdata, struct ts_incr *incr)
{
	u32 value;

	value = PHYTMAC_BITS(SUB_NSECL, incr->sub_ns) |
			PHYTMAC_BITS(SUB_NSECH, incr->sub_ns >> 8);
	PHYTMAC_WRITE(pdata, PHYTMAC_TISN, value);
	PHYTMAC_WRITE(pdata, PHYTMAC_TI, incr->ns);

	return 0;
}

static void phytmac_ptp_init_hw(struct phytmac *pdata)
{
	struct timespec64 ts;

	ts = ns_to_timespec64(ktime_to_ns(ktime_get_real()));
	phytmac_set_time(pdata, ts.tv_sec, ts.tv_nsec);

	phytmac_set_tsincr(pdata, &pdata->ts_incr);
}

static int phytmac_adjust_fine(struct phytmac *pdata, long ppm, bool negative)
{
	struct ts_incr ts_incr;
	u32 tmp;
	u64 adj;

	ts_incr.ns = pdata->ts_incr.ns;
	ts_incr.sub_ns = pdata->ts_incr.sub_ns;

	tmp = ((u64)ts_incr.ns << PHYTMAC_SUB_NSECL_INDEX) + ts_incr.sub_ns;
	adj = ((u64)ppm * tmp + (USEC_PER_SEC >> 1)) >> PHYTMAC_SUB_NSECL_INDEX;

	adj = div_u64(adj, USEC_PER_SEC);
	adj = negative ? (tmp - adj) : (tmp + adj);

	ts_incr.ns = (adj >> PHYTMAC_SUB_NSEC_WIDTH)
			& ((1 << PHYTMAC_SUB_NSECL_WIDTH) - 1);
	ts_incr.sub_ns = adj & ((1 << PHYTMAC_SUB_NSEC_WIDTH) - 1);

	phytmac_set_tsincr(pdata, &ts_incr);

	return 0;
}

static int phytmac_adjust_time(struct phytmac *pdata, s64 delta, int neg)
{
	u32 adj;

	if (delta > PHYTMAC_ADJUST_SEC_MAX) {
		struct timespec64 now, then;

		if (neg)
			then = ns_to_timespec64(-delta);
		else
			then = ns_to_timespec64(delta);
		phytmac_get_time(pdata, &now);
		now = timespec64_add(now, then);
		phytmac_set_time(pdata, now.tv_sec, now.tv_nsec);
	} else {
		adj = (neg << PHYTMAC_INCR_ADD_INDEX) | delta;
		PHYTMAC_WRITE(pdata, PHYTMAC_TA, adj);
	}

	return 0;
}

static int phytmac_ts_valid(struct phytmac *pdata, struct phytmac_dma_desc *desc, int direction)
{
	int ts_valid = 0;

	if (direction == PHYTMAC_TX)
		ts_valid = desc->desc1 & PHYTMAC_BIT(TX_TS_VALID);
	else if (direction == PHYTMAC_RX)
		ts_valid = desc->desc0 & PHYTMAC_BIT(RX_TS_VALID);
	return ts_valid;
}

static void phytmac_get_dma_ts(struct phytmac *pdata, u32 ts_1, u32 ts_2, struct timespec64 *ts)
{
	struct timespec64 ts2;

	ts->tv_sec = (PHYTMAC_GET_BITS(ts_2, DMA_SECH) << 2) |
				PHYTMAC_GET_BITS(ts_1, DMA_SECL);
	ts->tv_nsec = PHYTMAC_GET_BITS(ts_1, DMA_NSEC);

	phytmac_get_time(pdata, &ts2);

	if (((ts->tv_sec ^ ts2.tv_sec) & (PHYTMAC_DMA_SEC_TOP >> 1)) != 0)
		ts->tv_sec -= PHYTMAC_DMA_SEC_TOP;

	ts->tv_sec += (ts2.tv_sec & (~PHYTMAC_DMA_SEC_MASK));
}

static unsigned int phytmac_get_ts_rate(struct phytmac *pdata)
{
	return 300000000;
}

struct phytmac_hw_if phytmac_1p0_hw = {
	.reset_hw = phytmac_reset_hw,
	.init_hw = phytmac_init_hw,
	.init_ring_hw = phytmac_init_ring_hw,
	.get_feature = phytmac_get_feature_all,
	.get_regs = phytmac_get_regs,
	.get_stats = phytmac_get_hw_stats,
	.set_mac_address = phytmac_set_mac_addr,
	.get_mac_address = phytmac_get_mac_addr,
	.phy_speed_switch = phytmac_phy_speed_switch,
	.mdio_idle = phytmac_mdio_idle,
	.mdio_read = phytmac_mdio_data_read_c22,
	.mdio_write = phytmac_mdio_data_write_c22,
	.mdio_read_c45 = phytmac_mdio_data_read_c45,
	.mdio_write_c45 = phytmac_mdio_data_write_c45,
	.poweron = phytmac_powerup_hw,
	.set_wol = phytmac_set_wake,
	.enable_promise = phytmac_enable_promise,
	.enable_multicast = phytmac_enable_multicast,
	.set_hash_table = phytmac_set_mc_hash,
	.enable_rx_csum = phytmac_enable_rxcsum,
	.enable_tx_csum = phytmac_enable_txcsum,
	.enable_mdio_control = phytmac_enable_mdio,
	.enable_autoneg = phytmac_enable_autoneg,
	.enable_pause = phytmac_enable_pause,
	.enable_network = phytmac_enable_network,
	.add_fdir_entry = phytmac_add_fdir_entry,
	.del_fdir_entry = phytmac_del_fdir_entry,
	/* mac config */
	.mac_config = phytmac_mac_interface_config,
	.mac_linkup = phytmac_mac_linkup,
	.mac_linkdown = phytmac_mac_linkdown,
	.pcs_linkup = phytmac_pcs_linkup,
	.pcs_linkdown = phytmac_pcs_linkdown,
	.get_link = phytmac_pcs_get_link,
	/* irq */
	.enable_irq = phytmac_enable_irq,
	.disable_irq = phytmac_disable_irq,
	.clear_irq = phytmac_clear_irq,
	.get_irq = phytmac_get_irq,
	.get_intx_mask = phytmac_get_intx_mask,
	/* tx and rx */
	.tx_map = phytmac_tx_map_desc,
	.transmit = phytmac_tx_start,
	.update_rx_tail = phytmac_update_rx_tail,
	.tx_complete = phytmac_tx_complete,
	.rx_complete = phytmac_rx_complete,
	.get_rx_pkt_len = phytmac_rx_pkt_len,
	.init_rx_map = phytmac_init_rx_map_desc,
	.rx_map = phytmac_rx_map_desc,
	.rx_checksum = phytmac_rx_checksum,
	.rx_single_buffer = phytmac_rx_single_buffer,
	.rx_pkt_start = phytmac_rx_sof,
	.rx_pkt_end = phytmac_rx_eof,
	.clear_rx_desc = phytmac_clear_rx_desc,
	.clear_tx_desc = phytmac_clear_tx_desc,
	.zero_rx_desc_addr = phytmac_zero_rx_desc_addr,
	.zero_tx_desc = phytmac_zero_tx_desc,
	/* ptp */
	.init_ts_hw = phytmac_ptp_init_hw,
	.set_time = phytmac_set_time,
	.clear_time = phytmac_clear_time,
	.get_time = phytmac_get_time,
	.set_ts_config = phytmac_set_tsmode,
	.set_incr = phytmac_set_tsincr,
	.adjust_fine = phytmac_adjust_fine,
	.adjust_time = phytmac_adjust_time,
	.ts_valid = phytmac_ts_valid,
	.get_timestamp = phytmac_get_dma_ts,
	.get_ts_rate = phytmac_get_ts_rate,
};
EXPORT_SYMBOL_GPL(phytmac_1p0_hw);
