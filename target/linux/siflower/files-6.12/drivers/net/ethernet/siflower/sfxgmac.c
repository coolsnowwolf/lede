#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/etherdevice.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/ethtool.h>
#include <linux/if.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <linux/platform_device.h>
#include <linux/phylink.h>
#include <linux/reset.h>

#include "dma.h"
#include "eth.h"
#include "sfxgmac-ext.h"

struct xgmac_mib_desc {
	char name[ETH_GSTRING_LEN];
	u16 offset;
};
#define MIB_DESC(n, o, s) { .name = (n), .offset = (o), }

static const struct xgmac_mib_desc xgmac_mib[] = {
	MIB_DESC("tx_bytes",	MMC_XGMAC_TX_OCTET_GB, 1),
	MIB_DESC("tx_packets",	MMC_XGMAC_TX_PKT_GB, 1),
	MIB_DESC("tx_broadcast_packets_good",	MMC_XGMAC_TX_BROAD_PKT_G, 1),
	MIB_DESC("tx_multicast_packets_good",	MMC_XGMAC_TX_MULTI_PKT_G, 1),
	MIB_DESC("tx_64_byte_packets",	MMC_XGMAC_TX_64OCT_GB, 1),
	MIB_DESC("tx_65_to_127_byte_packets",	MMC_XGMAC_TX_65OCT_GB, 1),
	MIB_DESC("tx_128_to_255_byte_packets",	MMC_XGMAC_TX_128OCT_GB, 1),
	MIB_DESC("tx_256_to_511_byte_packets",	MMC_XGMAC_TX_256OCT_GB, 1),
	MIB_DESC("tx_512_to_1023_byte_packets",	MMC_XGMAC_TX_512OCT_GB, 1),
	MIB_DESC("tx_1024_to_max_byte_packets",	MMC_XGMAC_TX_1024OCT_GB, 1),
	MIB_DESC("tx_unicast_packets",	MMC_XGMAC_TX_UNI_PKT_GB, 1),
	MIB_DESC("tx_multicast_packets",	MMC_XGMAC_TX_MULTI_PKT_GB, 1),
	MIB_DESC("tx_broadcast_packets",	MMC_XGMAC_TX_BROAD_PKT_GB, 1),
	MIB_DESC("tx_underflow_errors",	MMC_XGMAC_TX_UNDER, 1),
	MIB_DESC("tx_bytes_good",	MMC_XGMAC_TX_OCTET_G, 1),
	MIB_DESC("tx_packets_good",	MMC_XGMAC_TX_PKT_G, 1),
	MIB_DESC("tx_pause_frames",	MMC_XGMAC_TX_PAUSE, 1),
	MIB_DESC("tx_vlan_packets_good",	MMC_XGMAC_TX_VLAN_PKT_G, 1),
	MIB_DESC("tx_lpi_usec",	MMC_XGMAC_TX_LPI_USEC, 0),
	MIB_DESC("tx_lpi_tran",	MMC_XGMAC_TX_LPI_TRAN, 0),
	MIB_DESC("rx_packets",	MMC_XGMAC_RX_PKT_GB, 1),
	MIB_DESC("rx_bytes",	MMC_XGMAC_RX_OCTET_GB, 1),
	MIB_DESC("rx_bytes_good",	MMC_XGMAC_RX_OCTET_G, 1),
	MIB_DESC("rx_broadcast_packets_good",	MMC_XGMAC_RX_BROAD_PKT_G, 1),
	MIB_DESC("rx_multicast_packets_good",	MMC_XGMAC_RX_MULTI_PKT_G, 1),
	MIB_DESC("rx_crc_errors",	MMC_XGMAC_RX_CRC_ERR, 1),
	MIB_DESC("rx_crc_errors_small_packets",	MMC_XGMAC_RX_RUNT_ERR, 0),
	MIB_DESC("rx_crc_errors_giant_packets",	MMC_XGMAC_RX_JABBER_ERR, 0),
	MIB_DESC("rx_undersize_packets_good",	MMC_XGMAC_RX_UNDER, 0),
	MIB_DESC("rx_oversize_packets_good",	MMC_XGMAC_RX_OVER, 0),
	MIB_DESC("rx_64_byte_packets",	MMC_XGMAC_RX_64OCT_GB, 1),
	MIB_DESC("rx_65_to_127_byte_packets",	MMC_XGMAC_RX_65OCT_GB, 1),
	MIB_DESC("rx_128_to_255_byte_packets",	MMC_XGMAC_RX_128OCT_GB, 1),
	MIB_DESC("rx_256_to_511_byte_packets",	MMC_XGMAC_RX_256OCT_GB, 1),
	MIB_DESC("rx_512_to_1023_byte_packets",	MMC_XGMAC_RX_512OCT_GB, 1),
	MIB_DESC("rx_1024_to_max_byte_packets",	MMC_XGMAC_RX_1024OCT_GB, 1),
	MIB_DESC("rx_unicast_packets_good",	MMC_XGMAC_RX_UNI_PKT_G, 1),
	MIB_DESC("rx_length_errors",	MMC_XGMAC_RX_LENGTH_ERR, 1),
	MIB_DESC("rx_out_of_range_errors",	MMC_XGMAC_RX_RANGE, 1),
	MIB_DESC("rx_pause_frames",	MMC_XGMAC_RX_PAUSE, 1),
	MIB_DESC("rx_fifo_overflow_errors",	MMC_XGMAC_RX_FIFOOVER_PKT, 1),
	MIB_DESC("rx_vlan_packets",	MMC_XGMAC_RX_VLAN_PKT_GB, 1),
	MIB_DESC("rx_watchdog_errors",	MMC_XGMAC_RX_WATCHDOG_ERR, 0),
	MIB_DESC("rx_lpi_usec",	MMC_XGMAC_RX_LPI_USEC, 0),
	MIB_DESC("rx_lpi_tran",	MMC_XGMAC_RX_LPI_TRAN, 0),
	MIB_DESC("rx_discard_packets",	MMC_XGMAC_RX_DISCARD_PKT_GB, 1),
	MIB_DESC("rx_discard_bytes",	MMC_XGMAC_RX_DISCARD_OCT_GB, 1),
	MIB_DESC("rx_alignment_errors",	MMC_XGMAC_RX_ALIGN_ERR_PKT, 0),
	MIB_DESC("tx_single_collision_packets",	MMC_XGMAC_TX_SINGLE_COL_G, 0),
	MIB_DESC("tx_multiple_collision_packets",	MMC_XGMAC_TX_MULTI_COL_G, 0),
	MIB_DESC("tx_deferred_packets",	MMC_XGMAC_TX_DEFER, 0),
	MIB_DESC("tx_late_collision_errors",	MMC_XGMAC_TX_LATE_COL, 0),
	MIB_DESC("tx_excessive_collision_errors",	MMC_XGMAC_TX_EXCESSIVE_COL, 0),
	MIB_DESC("tx_carrier_sense_errors",	MMC_XGMAC_TX_CARRIER, 0),
	MIB_DESC("tx_excessive_deferral_errors",	MMC_XGMAC_TX_EXCESSIVE_DEFER, 0),
	MIB_DESC("rx_ipv4_packets_good",	MMC_XGMAC_RX_IPV4_PKT_G, 1),
	MIB_DESC("rx_ipv4_header_error_packets",	MMC_XGMAC_RX_IPV4_HDRERR_PKT, 1),
	MIB_DESC("rx_ipv4_no_payload_packets",	MMC_XGMAC_RX_IPV4_NOPAY_PKT, 1),
	MIB_DESC("rx_ipv4_fragment_packets",	MMC_XGMAC_RX_IPV4_FRAG_PKT, 1),
	MIB_DESC("rx_ipv4_udp_sum_zero_packets",	MMC_XGMAC_RX_IPV4_UDSBL_PKT, 1),
	MIB_DESC("rx_ipv6_packets_good",	MMC_XGMAC_RX_IPV6_PKT_G, 1),
	MIB_DESC("rx_ipv6_header_error_packets",	MMC_XGMAC_RX_IPV6_HDRERR_PKT, 1),
	MIB_DESC("rx_ipv6_no_payload_packets",	MMC_XGMAC_RX_IPV6_NOPAY_PKT, 1),
	MIB_DESC("rx_udp_packets_good",	MMC_XGMAC_RX_UDP_PKT_G, 1),
	MIB_DESC("rx_udp_sum_error_packets",	MMC_XGMAC_RX_UDP_ERR_PKT, 1),
	MIB_DESC("rx_tcp_packets_good",	MMC_XGMAC_RX_TCP_PKT_G, 1),
	MIB_DESC("rx_tcp_sum_error_packets",	MMC_XGMAC_RX_TCP_ERR_PKT, 1),
	MIB_DESC("rx_icmp_packets_good",	MMC_XGMAC_RX_ICMP_PKT_G, 1),
	MIB_DESC("rx_icmp_sum_error_packets",	MMC_XGMAC_RX_ICMP_ERR_PKT, 1),
	MIB_DESC("rx_ipv4_bytes_good",	MMC_XGMAC_RX_IPV4_OCTET_G, 1),
	MIB_DESC("rx_ipv4_header_error_bytes",	MMC_XGMAC_RX_IPV4_HDRERR_OCTET, 1),
	MIB_DESC("rx_ipv4_no_payload_bytes",	MMC_XGMAC_RX_IPV4_NOPAY_OCTET, 1),
	MIB_DESC("rx_ipv4_fragment_bytes",	MMC_XGMAC_RX_IPV4_FRAG_OCTET, 1),
	MIB_DESC("rx_ipv4_udp_sum_zero_bytes",	MMC_XGMAC_RX_IPV4_UDSBL_OCTET, 1),
	MIB_DESC("rx_ipv6_bytes_good",	MMC_XGMAC_RX_IPV6_OCTET_G, 1),
	MIB_DESC("rx_ipv6_header_error_bytes",	MMC_XGMAC_RX_IPV6_HDRERR_OCTET, 1),
	MIB_DESC("rx_ipv6_no_payload_bytes",	MMC_XGMAC_RX_IPV6_NOPAY_OCTET, 1),
	MIB_DESC("rx_udp_bytes_good",	MMC_XGMAC_RX_UDP_OCTET_G, 1),
	MIB_DESC("rx_udp_sum_error_bytes",	MMC_XGMAC_RX_UDP_ERR_OCTET, 1),
	MIB_DESC("rx_tcp_bytes_good",	MMC_XGMAC_RX_TCP_OCTET_G, 1),
	MIB_DESC("rx_tcp_sum_error_bytes",	MMC_XGMAC_RX_TCP_ERR_OCTET, 1),
	MIB_DESC("rx_icmp_bytes_good",	MMC_XGMAC_RX_ICMP_OCTET_G, 1),
	MIB_DESC("rx_icmp_sum_error_bytes",	MMC_XGMAC_RX_ICMP_ERR_OCTET, 1),
};

struct xgmac_priv {
	GMAC_COMMON_STRUCT;
	struct mii_bus *mii;
	wait_queue_head_t mdio_wait;
	spinlock_t stats_lock;
	u32 mdio_ctrl;
	int sbd_irq;
	char irq_name[16];
	u64 mib_cache[ARRAY_SIZE(xgmac_mib)];
	struct phylink_pcs *pcs;
};

// Used by ndo_get_stats64, don't change this without changing xgmac_mib[]!
enum {
	STATS64_TX_PKT_GB = 1,
	STATS64_TX_UNDER = 13,
	STATS64_TX_OCTET_G = 14,
	STATS64_TX_PKT_G = 15,
	STATS64_RX_PKT_GB = 20,
	STATS64_RX_OCTET_G = 22,
	STATS64_RX_BROAD_PKT_G = 23,
	STATS64_RX_MULTI_PKT_G = 24,
	STATS64_RX_CRC_ERR = 25,
	STATS64_RX_UNI_PKT_G = 36,
	STATS64_RX_LENGTH_ERR = 37,
	STATS64_RX_RANGE = 38,
	STATS64_RX_FIFOOVER_PKT = 40,
	STATS64_RX_ALIGN_ERR_PKT = 47,
	STATS64_TX_SINGLE_COL_G = 48,
	STATS64_TX_MULTI_COL_G = 49,
	STATS64_TX_LATE_COL = 51,
	STATS64_TX_EXCESSIVE_COL = 52,
	STATS64_TX_CARRIER = 53,
};

// Sync MIB counters to software. Caller must hold stats_lock.
static void xgmac_mib_sync(struct xgmac_priv *priv)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(xgmac_mib); i++)
		priv->mib_cache[i] += reg_read(priv, xgmac_mib[i].offset);
}

static void xgmac_mib_irq_enable(struct xgmac_priv *priv)
{
	reg_write(priv, MMC_XGMAC_RX_INT_EN, ~0);
	reg_write(priv, MMC_XGMAC_TX_INT_EN, ~0);
	reg_write(priv, MMC_XGMAC_RX_IPC_INTR_MASK, 0);
}

static void xgmac_mib_irq_disable(struct xgmac_priv *priv)
{
	reg_write(priv, MMC_XGMAC_RX_INT_EN, 0);
	reg_write(priv, MMC_XGMAC_TX_INT_EN, 0);
	reg_write(priv, MMC_XGMAC_RX_IPC_INTR_MASK, ~0);
}

static void xgmac_mib_sync_begin(struct xgmac_priv *priv)
{
	xgmac_mib_irq_disable(priv);
	spin_lock(&priv->stats_lock);
}

static void xgmac_mib_sync_end(struct xgmac_priv *priv)
{
	spin_unlock(&priv->stats_lock);
	xgmac_mib_irq_enable(priv);
}

static int xgmac_mdio_wait(struct xgmac_priv *priv)
{
	unsigned long ret;
	u32 val;

	ret = wait_event_timeout(priv->mdio_wait,
				 !((val = reg_read(priv, XGMAC_MDIO_DATA)) & MII_XGMAC_BUSY),
				 HZ);
	if (ret)
		return FIELD_GET(MII_DATA_MASK, val);

	return -ETIMEDOUT;
}

static int xgmac_mdio_read(struct mii_bus *bus, int addr, int regnum)
{
	struct xgmac_priv *priv = bus->priv;
	u32 reg;
	int ret;

	ret = xgmac_mdio_wait(priv);
	if (ret < 0)
		return ret;

	reg_set(priv, XGMAC_MDIO_C22P, BIT(addr));

	reg = FIELD_PREP(MII_XGMAC_PA, addr) | FIELD_PREP(MII_XGMAC_RA, regnum);
	reg_write(priv, XGMAC_MDIO_ADDR, reg);

	reg = MII_XGMAC_BUSY | MII_XGMAC_READ | priv->mdio_ctrl;
	reg_write(priv, XGMAC_MDIO_DATA, reg);

	return xgmac_mdio_wait(priv);
}

static int xgmac_mdio_write(struct mii_bus *bus, int addr, int regnum, u16 val)
{
	struct xgmac_priv *priv = bus->priv;
	u32 reg;
	int ret;

	ret = xgmac_mdio_wait(priv);
	if (ret < 0)
		return ret;

	reg_set(priv, XGMAC_MDIO_C22P, BIT(addr));

	reg = FIELD_PREP(MII_XGMAC_PA, addr) | FIELD_PREP(MII_XGMAC_RA, regnum);
	reg_write(priv, XGMAC_MDIO_ADDR, reg);

	reg = MII_XGMAC_BUSY | MII_XGMAC_WRITE | priv->mdio_ctrl |
	      FIELD_PREP(MII_DATA_MASK, val);
	reg_write(priv, XGMAC_MDIO_DATA, reg);

	ret = xgmac_mdio_wait(priv);
	return ret < 0 ? ret : 0;
}

static int xgmac_mdio_read_c45(struct mii_bus *bus, int addr, int devnum, int regnum)
{
	struct xgmac_priv *priv = bus->priv;
	u32 reg;
	int ret;

	ret = xgmac_mdio_wait(priv);
	if (ret < 0)
		return ret;

	reg_clear(priv, XGMAC_MDIO_C22P, BIT(addr));

	reg = FIELD_PREP(MII_XGMAC_PA, addr) | FIELD_PREP(MII_XGMAC_DA, devnum) |
	      FIELD_PREP(MII_XGMAC_RA, regnum);
	reg_write(priv, XGMAC_MDIO_ADDR, reg);

	reg = MII_XGMAC_BUSY | MII_XGMAC_READ | priv->mdio_ctrl;
	reg_write(priv, XGMAC_MDIO_DATA, reg);

	return xgmac_mdio_wait(priv);
}

static int xgmac_mdio_write_c45(struct mii_bus *bus, int addr, int devnum, int regnum, u16 val)
{
	struct xgmac_priv *priv = bus->priv;
	u32 reg;
	int ret;

	ret = xgmac_mdio_wait(priv);
	if (ret < 0)
		return ret;

	reg_clear(priv, XGMAC_MDIO_C22P, BIT(addr));

	reg = FIELD_PREP(MII_XGMAC_PA, addr) | FIELD_PREP(MII_XGMAC_DA, devnum) |
	      FIELD_PREP(MII_XGMAC_RA, regnum);
	reg_write(priv, XGMAC_MDIO_ADDR, reg);

	reg = MII_XGMAC_BUSY | MII_XGMAC_WRITE | priv->mdio_ctrl |
	      FIELD_PREP(MII_DATA_MASK, val);
	reg_write(priv, XGMAC_MDIO_DATA, reg);

	ret = xgmac_mdio_wait(priv);
	return ret < 0 ? ret : 0;
}

static int xgmac_mdio_init(struct xgmac_priv *priv)
{
	struct device_node *mdio_node = NULL, *np = priv->dev->of_node;
	u32 csr_freq = clk_get_rate(priv->csr_clk);
	int ret = -ENOMEM;
	u32 freq;

	mdio_node = of_get_child_by_name(np, "mdio");
	if (!mdio_node)
		return 0;

	if (of_property_read_bool(mdio_node, "suppress-preamble"))
		priv->mdio_ctrl |= MII_XGMAC_PSE;

	if (of_property_read_u32(mdio_node, "clock-frequency", &freq))
		freq = 2500000;

	if (freq > DIV_ROUND_UP(csr_freq, 4))
		dev_warn(priv->dev,
			 "MDC frequency %uHz too high, reducing to %uHz\n",
			 freq, DIV_ROUND_UP(csr_freq, 4));
	else if (freq < csr_freq / 202)
		dev_warn(priv->dev,
			 "MDC frequency %uHz too low, increasing to %uHz\n",
			 freq, csr_freq / 202);

	if (freq >= DIV_ROUND_UP(csr_freq, 4))
		priv->mdio_ctrl |= MII_XGMAC_CRS | FIELD_PREP(MII_XGMAC_CR, 0);
	else if (freq >= DIV_ROUND_UP(csr_freq, 6))
		priv->mdio_ctrl |= MII_XGMAC_CRS | FIELD_PREP(MII_XGMAC_CR, 1);
	else if (freq >= DIV_ROUND_UP(csr_freq, 8))
		priv->mdio_ctrl |= MII_XGMAC_CRS | FIELD_PREP(MII_XGMAC_CR, 2);
	else if (freq >= DIV_ROUND_UP(csr_freq, 10))
		priv->mdio_ctrl |= MII_XGMAC_CRS | FIELD_PREP(MII_XGMAC_CR, 3);
	else if (freq >= DIV_ROUND_UP(csr_freq, 12))
		priv->mdio_ctrl |= MII_XGMAC_CRS | FIELD_PREP(MII_XGMAC_CR, 4);
	else if (freq >= DIV_ROUND_UP(csr_freq, 14))
		priv->mdio_ctrl |= MII_XGMAC_CRS | FIELD_PREP(MII_XGMAC_CR, 5);
	else if (freq >= DIV_ROUND_UP(csr_freq, 16))
		priv->mdio_ctrl |= MII_XGMAC_CRS | FIELD_PREP(MII_XGMAC_CR, 6);
	else if (freq >= DIV_ROUND_UP(csr_freq, 18))
		priv->mdio_ctrl |= MII_XGMAC_CRS | FIELD_PREP(MII_XGMAC_CR, 7);
	else if (freq >= DIV_ROUND_UP(csr_freq, 62))
		priv->mdio_ctrl |= FIELD_PREP(MII_XGMAC_CR, 0);
	else if (freq >= DIV_ROUND_UP(csr_freq, 102))
		priv->mdio_ctrl |= FIELD_PREP(MII_XGMAC_CR, 1);
	else if (freq >= DIV_ROUND_UP(csr_freq, 122))
		priv->mdio_ctrl |= FIELD_PREP(MII_XGMAC_CR, 2);
	else if (freq >= DIV_ROUND_UP(csr_freq, 142))
		priv->mdio_ctrl |= FIELD_PREP(MII_XGMAC_CR, 3);
	else if (freq >= DIV_ROUND_UP(csr_freq, 162))
		priv->mdio_ctrl |= FIELD_PREP(MII_XGMAC_CR, 4);
	else
		priv->mdio_ctrl |= FIELD_PREP(MII_XGMAC_CR, 5);

	priv->mii = devm_mdiobus_alloc(priv->dev);
	if (!priv->mii)
		goto cleanup;

	priv->mii->name = "xgmac";
	priv->mii->priv = priv;
	priv->mii->read = xgmac_mdio_read;
	priv->mii->write = xgmac_mdio_write;
	priv->mii->read_c45 = xgmac_mdio_read_c45;
	priv->mii->write_c45 = xgmac_mdio_write_c45;
	snprintf(priv->mii->id, MII_BUS_ID_SIZE, "xgmac%u", priv->id);
	init_waitqueue_head(&priv->mdio_wait);
	reg_write(priv, XGMAC_MDIO_INT_EN, XGMAC_MDIO_INT_EN_SINGLE);

	ret = devm_of_mdiobus_register(priv->dev, priv->mii, mdio_node);
cleanup:
	of_node_put(mdio_node);
	return ret;
}

static irqreturn_t xgmac_irq(int irq, void *dev_id)
{
	struct xgmac_priv *priv = dev_id;
	irqreturn_t ret = IRQ_NONE;
	u32 status;

	status = reg_read(priv, XGMAC_INT_STATUS);
	if (status & XGMAC_MMCIS) {
		if (spin_trylock(&priv->stats_lock)) {
			xgmac_mib_sync(priv);
			spin_unlock(&priv->stats_lock);
		}
		ret = IRQ_HANDLED;
	}
	if (status & XGMAC_SMI) {
		reg_read(priv, XGMAC_MDIO_INT_STATUS);
		wake_up(&priv->mdio_wait);
		ret = IRQ_HANDLED;
	}
	/* LSI is set regardless of LSIE bit in XGMAC_INT_EN */
	if (status & XGMAC_LSI && reg_read(priv, XGMAC_INT_EN) & XGMAC_LSI) {
		phylink_mac_change(priv->phylink, status & XGMAC_RGMII_LS);
		ret = IRQ_HANDLED;
	}

	return ret;
}

static void xgmac_write_mac_addr(struct xgmac_priv *priv, const u8 *addr,
				 u32 reg)
{
	u32 val;

	/* For MAC Addr registers we have to set the Address Enable (AE)
	 * bit that has no effect on the High Reg 0 where the bit 31 (MO)
	 * is RO.
	 */
	val = GMAC_HI_REG_AE | (addr[5] << 8) | addr[4];
	reg_write(priv, XGMAC_ADDRx_HIGH(reg), val);
	val = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
	reg_write(priv, XGMAC_ADDRx_LOW(reg), val);
}

static int xgmac_open(struct net_device *dev)
{
	struct xgmac_priv *priv = netdev_priv(dev);
	int ret;

	ret = phylink_of_phy_connect(priv->phylink, priv->dev->of_node, 0);
	if (ret)
		return ret;

	ret = xgmac_dma_open(priv->dma, dev, priv->id);
	if (ret) {
		phylink_disconnect_phy(priv->phylink);
		return ret;
	}

	phylink_start(priv->phylink);
	netif_tx_start_all_queues(dev);

	return 0;
}

static int xgmac_stop(struct net_device *dev)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	phylink_stop(priv->phylink);
	netif_tx_stop_all_queues(dev);
	phylink_disconnect_phy(priv->phylink);

	return xgmac_dma_stop(priv->dma, dev, priv->id);
}

static void xgmac_set_rx_mode(struct net_device *dev)
{
	struct xgmac_priv *priv = netdev_priv(dev);
	unsigned int value = 0;
	unsigned int perfect_addr_number = 4;

	pr_debug("%s: # mcasts %d, # unicast %d\n", __func__,
		 netdev_mc_count(dev), netdev_uc_count(dev));

	if (dev->flags & IFF_PROMISC) {
		value = XGMAC_FILTER_PR;
	} else if ((dev->flags & IFF_ALLMULTI) || !netdev_mc_empty(dev)) {
		value = XGMAC_FILTER_PM;	/* pass all multi */
	}

	/* Handle multiple unicast addresses (perfect filtering) */
	if (netdev_uc_count(dev) > perfect_addr_number)
		/* Switch to promiscuous mode if more than unicast
		 * addresses are requested than supported by hardware.
		 */
		value |= XGMAC_FILTER_PR;
	else {
		int reg = 1;
		struct netdev_hw_addr *ha;

		netdev_for_each_uc_addr(ha, dev) {
			xgmac_write_mac_addr(priv, ha->addr, reg);
			reg++;
		}

		while (reg <= perfect_addr_number) {
			reg_write(priv, XGMAC_ADDRx_HIGH(reg), 0);
			reg_write(priv, XGMAC_ADDRx_LOW(reg), 0);
			reg++;
		}
	}

#ifdef FRAME_FILTER_DEBUG
	/* Enable Receive all mode (to debug filtering_fail errors) */
	value |= XGMAC_FILTER_RA;
#endif
	reg_write(priv, XGMAC_PACKET_FILTER, value);
}

static int xgmac_set_mac_address(struct net_device *dev, void *p)
{
	struct xgmac_priv *priv = netdev_priv(dev);
	int ret;

	ret = eth_mac_addr(dev, p);
	if (ret)
		return ret;

	xgmac_write_mac_addr(priv, dev->dev_addr, 0);

	return 0;
}

static int xgmac_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	return phylink_mii_ioctl(priv->phylink, ifr, cmd);
}

static int xgmac_set_features(struct net_device *dev,
			      netdev_features_t features)
{
	netdev_features_t diff = dev->features ^ features;
	struct xgmac_priv *priv = netdev_priv(dev);
	u32 ctrl = reg_read(priv, XGMAC_RX_CONFIG);

	if (diff & NETIF_F_LOOPBACK) {
		if (features & NETIF_F_LOOPBACK) {
			netdev_info(dev, "MAC internal loopback enabled\n");
			ctrl |= XGMAC_CONFIG_LM;
		} else {
			netdev_info(dev, "MAC internal loopback disabled\n");
			ctrl &= ~XGMAC_CONFIG_LM;
		}
	}

	if (diff & NETIF_F_RXFCS) {
		if (features & NETIF_F_RXFCS) {
			netdev_info(dev, "MAC FCS stripping disabled\n");
			ctrl &= ~(XGMAC_CONFIG_ACS | XGMAC_CONFIG_CST);
		} else {
			netdev_info(dev, "MAC FCS stripping enabled\n");
			ctrl |= XGMAC_CONFIG_ACS | XGMAC_CONFIG_CST;
		}
	}

	if (diff & NETIF_F_RXCSUM) {
		if (features & NETIF_F_RXCSUM) {
			netdev_info(dev, "MAC Rx checksum offload enabled\n");
			ctrl |= XGMAC_CONFIG_IPC;
		} else {
			netdev_info(dev, "MAC Rx checksum offload disabled\n");
			ctrl &= ~XGMAC_CONFIG_IPC;
		}
	}

	if (diff & NETIF_F_RXALL) {
		u32 rff = reg_read(priv, XGMAC_PACKET_FILTER);

		if (features & NETIF_F_RXALL) {
			ctrl |= XGMAC_CONFIG_DCRCC;
			rff |= XGMAC_FILTER_RA;
		} else {
			ctrl &= ~XGMAC_CONFIG_DCRCC;
			rff &= ~XGMAC_FILTER_RA;
		}
		reg_write(priv, XGMAC_PACKET_FILTER, rff);
	}
	reg_write(priv, XGMAC_RX_CONFIG, ctrl);

	return 0;
}

static int xgmac_get_phys_port_id(struct net_device *dev,
					  struct netdev_phys_item_id *ppid)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	ppid->id[0] = priv->id;
	ppid->id_len = 1;

	return 0;
}

static int xgmac_get_port_parent_id(struct net_device *dev,
					  struct netdev_phys_item_id *ppid)
{
	ppid->id[0] = SF_GMAC_DUNMMY_ID;
	ppid->id_len = 1;

	return 0;
}

static void xgmac_neigh_destroy(struct net_device *dev,
				      struct neighbour *n)
{
	// struct xgmac_priv *priv = netdev_priv(dev);

	/** TODO: call dpns->ops->port_neigh_destroy(dp_port, n); */
	return;
}
static void xgmac_get_stats64(struct net_device *dev,
			      struct rtnl_link_stats64 *stats)
{
	struct xgmac_priv *priv = netdev_priv(dev);
	const u64 *mib;

	// Compile-time check in case someone changes the order
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_PKT_GB].offset != MMC_XGMAC_TX_PKT_GB);
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_UNDER].offset != MMC_XGMAC_TX_UNDER);
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_OCTET_G].offset != MMC_XGMAC_TX_OCTET_G);
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_PKT_G].offset != MMC_XGMAC_TX_PKT_G);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_PKT_GB].offset != MMC_XGMAC_RX_PKT_GB);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_OCTET_G].offset != MMC_XGMAC_RX_OCTET_G);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_BROAD_PKT_G].offset != MMC_XGMAC_RX_BROAD_PKT_G);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_MULTI_PKT_G].offset != MMC_XGMAC_RX_MULTI_PKT_G);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_CRC_ERR].offset != MMC_XGMAC_RX_CRC_ERR);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_UNI_PKT_G].offset != MMC_XGMAC_RX_UNI_PKT_G);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_LENGTH_ERR].offset != MMC_XGMAC_RX_LENGTH_ERR);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_RANGE].offset != MMC_XGMAC_RX_RANGE);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_FIFOOVER_PKT].offset != MMC_XGMAC_RX_FIFOOVER_PKT);
	BUILD_BUG_ON(xgmac_mib[STATS64_RX_ALIGN_ERR_PKT].offset != MMC_XGMAC_RX_ALIGN_ERR_PKT);
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_SINGLE_COL_G].offset != MMC_XGMAC_TX_SINGLE_COL_G);
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_MULTI_COL_G].offset != MMC_XGMAC_TX_MULTI_COL_G);
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_LATE_COL].offset != MMC_XGMAC_TX_LATE_COL);
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_EXCESSIVE_COL].offset != MMC_XGMAC_TX_EXCESSIVE_COL);
	BUILD_BUG_ON(xgmac_mib[STATS64_TX_CARRIER].offset != MMC_XGMAC_TX_CARRIER);

	xgmac_mib_sync_begin(priv);
	xgmac_mib_sync(priv);
	mib = priv->mib_cache;
	stats->rx_packets = mib[STATS64_RX_BROAD_PKT_G] +
			    mib[STATS64_RX_MULTI_PKT_G] +
			    mib[STATS64_RX_UNI_PKT_G];
	stats->tx_packets = mib[STATS64_TX_PKT_G];
	stats->rx_bytes = mib[STATS64_RX_OCTET_G];
	stats->tx_bytes = mib[STATS64_TX_OCTET_G];
	stats->rx_errors = mib[STATS64_RX_PKT_GB] - stats->rx_packets;
	stats->tx_errors = mib[STATS64_TX_PKT_GB] - stats->tx_packets;
	stats->multicast = mib[STATS64_RX_BROAD_PKT_G] +
			   mib[STATS64_RX_MULTI_PKT_G];
	stats->collisions = mib[STATS64_TX_SINGLE_COL_G] +
			    mib[STATS64_TX_MULTI_COL_G];
	stats->rx_length_errors = mib[STATS64_RX_LENGTH_ERR] +
				  mib[STATS64_RX_RANGE];
	stats->rx_over_errors = mib[STATS64_RX_FIFOOVER_PKT];
	stats->rx_crc_errors = mib[STATS64_RX_CRC_ERR];
	stats->rx_frame_errors = mib[STATS64_RX_ALIGN_ERR_PKT];
	stats->tx_aborted_errors = mib[STATS64_TX_EXCESSIVE_COL];
	stats->tx_carrier_errors = mib[STATS64_TX_CARRIER];
	stats->tx_fifo_errors = mib[STATS64_TX_UNDER];
	stats->tx_window_errors = mib[STATS64_TX_LATE_COL];
	xgmac_mib_sync_end(priv);
}

static int xgmac_change_mtu(struct net_device *dev, int new_mtu)
{
	struct xgmac_priv *priv = netdev_priv(dev);
	u32 step;

	dev->mtu = new_mtu;

	/* Configure GPSL field in XGMAC_RX_CONFIG. L2 header and FCS must
	 * be taken into account. For a MTU of 1500 bytes, this field is set
	 * to 1518. For VLAN tagged packets, the hardware adds 4 bytes
	 * (single tagged) or 8 bytes (double tagged) to the programmed value.
	 */
	new_mtu += ETH_HLEN + ETH_FCS_LEN;
	reg_rmw(priv, XGMAC_RX_CONFIG, XGMAC_CONFIG_GPSL,
		FIELD_PREP(XGMAC_CONFIG_GPSL, new_mtu) | XGMAC_CONFIG_GPSLCE);

	/* Configure Rx watchdog and Tx jabber threshold, with 1KB step.
	 * Unlike the GPSL field, the hardware does not count the VLAN tag
	 * overhead so it must be manually added.
	 */
	new_mtu = min(new_mtu + VLAN_HLEN * 2, MAX_FRAME_SIZE);
	step = DIV_ROUND_UP((u32)new_mtu, SZ_1K);
	// The step begins at 2KB
	step = step < 2 ? 0 : step - 2;
	reg_write(priv, XGMAC_WD_JB_TIMEOUT, XGMAC_PJE | XGMAC_PWE |
		  FIELD_PREP(XGMAC_JTO, step) | FIELD_PREP(XGMAC_WTO, step));

	return 0;
}

static const struct net_device_ops xgmac_netdev_ops = {
	.ndo_open		= xgmac_open,
	.ndo_stop		= xgmac_stop,
	.ndo_start_xmit		= xgmac_dma_xmit_fast,
	.ndo_set_rx_mode	= xgmac_set_rx_mode,
	.ndo_set_mac_address	= xgmac_set_mac_address,
	.ndo_do_ioctl		= xgmac_ioctl,
	.ndo_set_features	= xgmac_set_features,
	.ndo_get_phys_port_id	= xgmac_get_phys_port_id,
	.ndo_get_port_parent_id	= xgmac_get_port_parent_id,
	.ndo_neigh_destroy	= xgmac_neigh_destroy,
	.ndo_get_stats64	= xgmac_get_stats64,
	.ndo_change_mtu		= xgmac_change_mtu,
};

static struct xgmac_priv *sfxgmac_phylink_to_port(struct phylink_config *config)
{
	return container_of(config, struct xgmac_priv, phylink_config);
}

static struct phylink_pcs *xgmac_mac_selct_pcs(struct phylink_config *config,
					    phy_interface_t interface)
{
	struct xgmac_priv *priv = sfxgmac_phylink_to_port(config);

	return priv->pcs;
}

static void xgmac_toggle_tx_lpi(struct xgmac_priv *priv)
{
	if (priv->phy_supports_eee && priv->tx_lpi_enabled) {
		reg_set(priv, XGMAC_LPI_CTRL,
			XGMAC_LPIATE | XGMAC_LPITXA | XGMAC_LPITXEN);
	} else {
		reg_clear(priv, XGMAC_LPI_CTRL,
			  XGMAC_LPIATE | XGMAC_LPITXA | XGMAC_LPITXEN);
	}
}

static void xgmac_mac_config(struct phylink_config *config, unsigned int mode,
			     const struct phylink_link_state *state)
{
	struct xgmac_priv *priv = netdev_priv(to_net_dev(config->dev));

	/* Enable link change interrupt for RGMII */
	if (phy_interface_mode_is_rgmii(state->interface) &&
	    phylink_autoneg_inband(mode))
		reg_set(priv, XGMAC_INT_EN, XGMAC_LSI);
	else
		reg_clear(priv, XGMAC_INT_EN, XGMAC_LSI);
}

static void xgmac_mac_reset(struct xgmac_priv *priv)
{
	struct net_device *ndev = priv_to_netdev(priv);
	int ret;

	if (priv->id < 5) {
		ret = regmap_clear_bits(priv->ethsys, ETHSYS_RST, BIT(priv->id));
		if (ret)
			return;

		ret = regmap_set_bits(priv->ethsys, ETHSYS_RST, BIT(priv->id));
		if (ret)
			return;
	}else {
		ret = regmap_clear_bits(priv->ethsys, ETHSYS_RST, ETHSYS_RST_MAC5);
		if (ret)
			return;

		/* set mac5 phy mode to rgmii, should set under mac reset */
		ret = regmap_write(priv->ethsys, ETHSYS_MAC5_CTRL,
				FIELD_PREP(MAC5_PHY_INTF_SEL, 1));
		if (ret)
			return;

		ret = regmap_set_bits(priv->ethsys, ETHSYS_RST, ETHSYS_RST_MAC5);
		if (ret)
			return;
	}

	regmap_set_bits(priv->ethsys, ETHSYS_TX_DIS, BIT(priv->id));
	reg_clear(priv, XGMAC_TX_CONFIG, XGMAC_CONFIG_TE);
	reg_clear(priv, XGMAC_INT_EN, XGMAC_LSI);
	xgmac_write_mac_addr(priv, ndev->dev_addr, 0);
	reg_set(priv, MMC_XGMAC_CONTROL,
		MMC_XGMAC_CONTROL_RESET | MMC_XGMAC_CONTROL_RSTONRD);
	reg_write(priv, XGMAC_TX_CONFIG, XGMAC_CORE_INIT_TX);
	reg_write(priv, XGMAC_RX_CONFIG, XGMAC_CORE_INIT_RX);
	reg_write(priv, XGMAC_WD_JB_TIMEOUT, XGMAC_PJE | XGMAC_PWE);
	reg_write(priv, XGMAC_VLAN_TAG, XGMAC_VLAN_EDVLP);
	reg_write(priv, XGMAC_LPI_TIMER_CTRL,
		  FIELD_PREP(XGMAC_LPI_LST, XGMAC_LPI_LST_DEFAULT) |
		  FIELD_PREP(XGMAC_LPI_TWT, XGMAC_LPI_TWT_DEFAULT));
	reg_write(priv, XGMAC_LPI_AUTO_EN, XGMAC_LPI_AUTO_EN_DEFAULT);
	xgmac_mib_irq_enable(priv);
	reg_write(priv, XGMAC_LPI_1US,
		  clk_get_rate(priv->csr_clk) / 1000000 - 1);
}

static void wait_queue_empty(struct xgmac_priv *priv,
		phy_interface_t interface)
{
	unsigned long timeout = jiffies + HZ/50;

	do {
		if (!(reg_read(priv, XGMAC_MTL_TXQ_OPMODE(0)) & XGMAC_FTQ))
			return;

		cond_resched();
	} while (time_after(timeout, jiffies));

	xgmac_mac_reset(priv);
	printk("wait queue empty timed out\n");
}

static void xgmac_mac_link_down(struct phylink_config *config,
				unsigned int mode, phy_interface_t interface)
{
	struct xgmac_priv *priv = netdev_priv(to_net_dev(config->dev));

	regmap_set_bits(priv->ethsys, ETHSYS_TX_DIS, BIT(priv->id));
	reg_clear(priv, XGMAC_TX_CONFIG, XGMAC_CONFIG_TE);
	reg_set(priv, XGMAC_MTL_TXQ_OPMODE(0), XGMAC_FTQ);
	wait_queue_empty(priv, interface);
	reg_clear(priv, XGMAC_LPI_CTRL, XGMAC_PLS);
}

static void xgmac_mac_link_up(struct phylink_config *config,
			      struct phy_device *phy, unsigned int mode,
			      phy_interface_t interface, int speed, int duplex,
			      bool tx_pause, bool rx_pause) {
	struct xgmac_priv *priv = netdev_priv(to_net_dev(config->dev));
	u32 txc, rxfc, txfc;

	txc = reg_read(priv, XGMAC_TX_CONFIG);
	txc |= XGMAC_CONFIG_TE;
	txc &= ~XGMAC_CONFIG_SS_MASK;

	switch (speed) {
	case SPEED_2500:
		txc |= XGMAC_CONFIG_SS_2500_GMII;
		break;
	case SPEED_1000:
		txc |= XGMAC_CONFIG_SS_1000_GMII;
		break;
	case SPEED_100:
		txc |= XGMAC_CONFIG_SS_100_MII;
		break;
	case SPEED_10:
		txc |= XGMAC_CONFIG_SS_10_MII;
		break;
	default:
		return;
	}

	if (duplex == DUPLEX_FULL)
		reg_clear(priv, XGMAC_MAC_EXT_CONFIG, XGMAC_HD);
	else
		reg_set(priv, XGMAC_MAC_EXT_CONFIG, XGMAC_HD);

	rxfc = XGMAC_UP;
	if (rx_pause)
		rxfc |= XGMAC_RFE;

	reg_write(priv, XGMAC_RX_FLOW_CTRL, rxfc);

	txfc = 0;
	if (tx_pause) {
		txfc |= XGMAC_TFE;
		if (duplex == DUPLEX_FULL)
			txfc |= FIELD_PREP(XGMAC_PT, 0x400);
	}
	reg_write(priv, XGMAC_Qx_TX_FLOW_CTRL(0), txfc);

	reg_write(priv, XGMAC_TX_CONFIG, txc);
	reg_set(priv, XGMAC_RX_CONFIG, XGMAC_CONFIG_RE);
	reg_set(priv, XGMAC_LPI_CTRL, XGMAC_PLS);
	regmap_clear_bits(priv->ethsys, ETHSYS_TX_DIS, BIT(priv->id));

	if (phy)
		priv->phy_supports_eee = !phy_init_eee(phy, true);
	else
		priv->phy_supports_eee = false;

	xgmac_toggle_tx_lpi(priv);
}

static const struct phylink_mac_ops xgmac_phylink_mac_ops = {
	.mac_select_pcs = xgmac_mac_selct_pcs,
	.mac_config	= xgmac_mac_config,
	.mac_link_down	= xgmac_mac_link_down,
	.mac_link_up	= xgmac_mac_link_up,
};

static void xgmac_ethtool_get_wol(struct net_device *dev,
				 struct ethtool_wolinfo *wol)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	return phylink_ethtool_get_wol(priv->phylink, wol);
}

static int xgmac_ethtool_set_wol(struct net_device *dev,
				struct ethtool_wolinfo *wol)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	return phylink_ethtool_set_wol(priv->phylink, wol);
}

static int xgmac_ethtool_nway_reset(struct net_device *dev)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	return phylink_ethtool_nway_reset(priv->phylink);
}

static void xgmac_ethtool_get_pauseparam(struct net_device *dev,
					 struct ethtool_pauseparam *pause)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	phylink_ethtool_get_pauseparam(priv->phylink, pause);
}

static int xgmac_ethtool_set_pauseparam(struct net_device *dev,
					struct ethtool_pauseparam *pause)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	return phylink_ethtool_set_pauseparam(priv->phylink, pause);
}


static void xgmac_ethtool_get_strings(struct net_device *dev, u32 stringset,
				      u8 *data)
{
	int i;

	if (stringset != ETH_SS_STATS)
		return;

	for (i = 0; i < ARRAY_SIZE(xgmac_mib); i++) {
		memcpy(data, xgmac_mib[i].name, ETH_GSTRING_LEN);
		data += ETH_GSTRING_LEN;
	}
}

static void xgmac_ethtool_get_stats(struct net_device *dev,
				    struct ethtool_stats *stats, u64 *data)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	xgmac_mib_sync_begin(priv);
	xgmac_mib_sync(priv);
	memcpy(data, priv->mib_cache, sizeof(priv->mib_cache));
	xgmac_mib_sync_end(priv);
}

static int xgmac_ethtool_reset(struct net_device *dev, u32 *flags)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	if (*flags & ETH_RESET_MGMT) {
		xgmac_mib_sync_begin(priv);
		reg_set(priv, MMC_XGMAC_CONTROL, MMC_XGMAC_CONTROL_RESET);
		memset(priv->mib_cache, 0, sizeof(priv->mib_cache));
		xgmac_mib_sync_end(priv);
		*flags &= ~ETH_RESET_MGMT;
	}

	return 0;
}

static int xgmac_ethtool_get_sset_count(struct net_device *dev, int stringset)
{
	switch (stringset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(xgmac_mib);
	default:
		return -EOPNOTSUPP;
	}
}

static int xgmac_ethtool_get_link_ksettings(struct net_device *dev,
					    struct ethtool_link_ksettings *cmd)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	return phylink_ethtool_ksettings_get(priv->phylink, cmd);
}

static int xgmac_ethtool_get_eee(struct net_device *dev, struct ethtool_keee *eee)
{
	struct xgmac_priv *priv = netdev_priv(dev);
	int ret;

	ret = phylink_ethtool_get_eee(priv->phylink, eee);
	if (ret)
		return ret;

	eee->tx_lpi_enabled = priv->tx_lpi_enabled;
	eee->tx_lpi_timer = reg_read(priv, XGMAC_LPI_AUTO_EN);

	return 0;
}

static int xgmac_ethtool_set_eee(struct net_device *dev, struct ethtool_keee *eee)
{
	struct xgmac_priv *priv = netdev_priv(dev);
	int ret;

	if (eee->tx_lpi_timer > XGMAC_LPI_AUTO_EN_MAX)
		return -EINVAL;

	ret = phylink_ethtool_set_eee(priv->phylink, eee);
	if (ret)
		return ret;

	priv->tx_lpi_enabled = eee->tx_lpi_enabled;
	xgmac_toggle_tx_lpi(priv);

	reg_write(priv, XGMAC_LPI_AUTO_EN, eee->tx_lpi_timer);

	return 0;
}

static int xgmac_ethtool_set_link_ksettings(struct net_device *dev,
					    const struct ethtool_link_ksettings *cmd)
{
	struct xgmac_priv *priv = netdev_priv(dev);

	return phylink_ethtool_ksettings_set(priv->phylink, cmd);
}

static const struct ethtool_ops xgmac_ethtool_ops = {
	.get_wol		= xgmac_ethtool_get_wol,
	.set_wol		= xgmac_ethtool_set_wol,
	.nway_reset		= xgmac_ethtool_nway_reset,
	.get_link		= ethtool_op_get_link,
	.get_pauseparam		= xgmac_ethtool_get_pauseparam,
	.set_pauseparam		= xgmac_ethtool_set_pauseparam,
	.get_strings		= xgmac_ethtool_get_strings,
	.get_ethtool_stats	= xgmac_ethtool_get_stats,
	.reset			= xgmac_ethtool_reset,
	.get_sset_count		= xgmac_ethtool_get_sset_count,
	.get_eee		= xgmac_ethtool_get_eee,
	.set_eee		= xgmac_ethtool_set_eee,
	.get_link_ksettings	= xgmac_ethtool_get_link_ksettings,
	.set_link_ksettings	= xgmac_ethtool_set_link_ksettings,
};

static int xgmac_rgmii_delay(struct xgmac_priv *priv, phy_interface_t phy_mode)
{
	u32 reg = 0, rxd = MAC5_DELAY_DEFAULT, txd = MAC5_DELAY_DEFAULT;

	of_property_read_u32(priv->dev->of_node, "rx-internal-delay-ps", &rxd);
	of_property_read_u32(priv->dev->of_node, "tx-internal-delay-ps", &txd);

	rxd = DIV_ROUND_CLOSEST(rxd, MAC5_DELAY_STEP);
	txd = DIV_ROUND_CLOSEST(txd, MAC5_DELAY_STEP);

	if (rxd > 256 || txd > 256)
		return -EINVAL;

	if (rxd)
		reg |= FIELD_PREP(MAC5_RX_DELAY, rxd - 1) | MAC5_RX_DELAY_EN;

	if (txd)
		reg |= FIELD_PREP(MAC5_TX_DELAY, txd - 1) | MAC5_TX_DELAY_EN;

	return regmap_update_bits(priv->ethsys, ETHSYS_MAC(5),
				  MAC5_DELAY_MASK, reg);
}

static int xgmac_phy_setup(struct xgmac_priv *priv)
{
	struct device *dev = priv->dev;
	struct device_node *np = dev->of_node;
	phy_interface_t phy_mode;
	struct phylink *phylink;
	struct platform_device *pcs_dev;
	struct phylink_pcs *pcs = NULL;
	int ret;

	ret = of_get_phy_mode(np, &phy_mode);
	if (ret)
		return ret;

	if (phy_interface_mode_is_rgmii(phy_mode)) {
		ret = xgmac_rgmii_delay(priv, phy_mode);
		if (ret)
			return ret;

		priv->csr_clk = devm_clk_get_enabled(dev, "rgmii");
		if (IS_ERR(priv->csr_clk))
			return PTR_ERR(priv->csr_clk);

		if (ret)
			return ret;
	} else {
		struct of_phandle_args pcs_args;
		ret = of_parse_phandle_with_fixed_args(np, "pcs-handle", 1, 0,
						       &pcs_args);
		if (ret)
			return ret;

		pcs_dev = of_find_device_by_node(pcs_args.np);
		of_node_put(pcs_args.np);
		if (!pcs_dev)
			return -ENODEV;

		pcs = xpcs_port_get(pcs_dev, pcs_args.args[0]);
		if (IS_ERR(pcs))
			return PTR_ERR(pcs);

		priv->pcs_dev = pcs_dev;
	}

	priv->phylink_config.dev = &priv_to_netdev(priv)->dev;
	priv->phylink_config.type = PHYLINK_NETDEV;

	__set_bit(phy_mode, priv->phylink_config.supported_interfaces);

	priv->phylink_config.mac_capabilities = MAC_SYM_PAUSE | MAC_ASYM_PAUSE |
						MAC_10 | MAC_100 | MAC_1000FD |
						MAC_2500FD;


	phylink = phylink_create(&priv->phylink_config, dev->fwnode, phy_mode,
				 &xgmac_phylink_mac_ops);
	if (IS_ERR(phylink))
		return PTR_ERR(phylink);

	if (pcs)
		priv->pcs = pcs;

	priv->phylink = phylink;

	return 0;
}

static int xgmac_probe(struct platform_device *pdev)
{
	static bool mac_disable_tx_set;
	struct platform_device *dma_pdev;
	struct device_node *dma_node;
	struct net_device *ndev;
	struct xgmac_priv *priv;
	struct resource *r;
	u32 ver;
	int ret;

	dma_node = of_parse_phandle(pdev->dev.of_node, "dmas", 0);
	if (!dma_node)
		return -ENODEV;

	dma_pdev = of_find_device_by_node(dma_node);
	of_node_put(dma_node);
	if (!dma_pdev)
		return -ENODEV;

	ndev = devm_alloc_etherdev_mqs(&pdev->dev, sizeof(*priv), DMA_CH_MAX,
				       DMA_CH_MAX);
	if (!ndev)
		return -ENOMEM;

	SET_NETDEV_DEV(ndev, &pdev->dev);
	platform_set_drvdata(pdev, ndev);
	priv = netdev_priv(ndev);
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->ioaddr = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(priv->ioaddr))
		return PTR_ERR(priv->ioaddr);

	priv->csr_clk = devm_clk_get_enabled(&pdev->dev, "csr");
	if (IS_ERR(priv->csr_clk))
		return PTR_ERR(priv->csr_clk);

	priv->id = offset_to_id(r->start);
	priv->ethsys = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
						       "ethsys");
	if (IS_ERR(priv->ethsys))
		return PTR_ERR(priv->ethsys);

	if (priv->id < 5) {
		/* mac reset and release reset*/
		ret = regmap_clear_bits(priv->ethsys, ETHSYS_RST, BIT(priv->id));
		if (ret)
			return ret;

		ret = regmap_set_bits(priv->ethsys, ETHSYS_RST, BIT(priv->id));
		if (ret)
			return ret;

	} else {
		ret = regmap_clear_bits(priv->ethsys, ETHSYS_RST, ETHSYS_RST_MAC5);
		if (ret)
			return ret;

		/* set mac5 phy mode to rgmii, should set under mac reset */
		ret = regmap_write(priv->ethsys, ETHSYS_MAC5_CTRL,
				FIELD_PREP(MAC5_PHY_INTF_SEL, 1));
		if (ret)
			return ret;

		ret = regmap_set_bits(priv->ethsys, ETHSYS_RST, ETHSYS_RST_MAC5);
		if (ret)
			return ret;
	}

	if (!mac_disable_tx_set) {
		/* Disable all MAC Tx, once and only once */
		mac_disable_tx_set = true;
		ret = regmap_write(priv->ethsys, ETHSYS_TX_DIS, 0xff);
		if (ret)
			return ret;
	}

	ver = reg_read(priv, XGMAC_VERSION);
	if (FIELD_GET(XGMAC_VERSION_ID_MASK, ver) != XGMAC_VERSION_ID)
		return -ENODEV;

	spin_lock_init(&priv->stats_lock);
	priv->dev = &pdev->dev;
	priv->dma = platform_get_drvdata(dma_pdev);
	if (!priv->dma)
		return -EPROBE_DEFER;

	ndev->base_addr = r->start;
	ndev->netdev_ops = &xgmac_netdev_ops;
	ndev->ethtool_ops = &xgmac_ethtool_ops;
	ndev->features = NETIF_F_RXHASH | NETIF_F_RXCSUM | NETIF_F_GRO |
			 NETIF_F_SG | NETIF_F_HW_TC |
			 NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM | NETIF_F_TSO |
			 NETIF_F_TSO6;
	ndev->hw_features = (ndev->features & ~NETIF_F_RXHASH) |
			    NETIF_F_LOOPBACK | NETIF_F_RXFCS | NETIF_F_RXALL |
			    NETIF_F_HW_L2FW_DOFFLOAD;
	ndev->vlan_features = ndev->features;
	ndev->priv_flags |= IFF_UNICAST_FLT | IFF_LIVE_ADDR_CHANGE;
	ndev->max_mtu = MAX_FRAME_SIZE - ETH_HLEN - ETH_FCS_LEN;

	/* read-clear interrupt status before registering */
	reg_read(priv, XGMAC_MDIO_INT_STATUS);
	reg_read(priv, XGMAC_INT_STATUS);
	ret = platform_get_irq(pdev, 0);
	if (ret < 0)
		return ret;

	reg_clear(priv, XGMAC_INT_EN, XGMAC_LSI);
	snprintf(priv->irq_name, sizeof(priv->irq_name), "xgmac%u_sbd",
		 priv->id);
	ret = devm_request_irq(&pdev->dev, ret, xgmac_irq, 0, priv->irq_name,
			       priv);
	if (ret)
		return ret;

	ret = xgmac_mdio_init(priv);
	if (ret)
		return ret;

	ret = of_get_ethdev_address(pdev->dev.of_node, ndev);
	if (ret == -EPROBE_DEFER)
		return ret;

	if (ret) {
		eth_hw_addr_random(ndev);
		dev_warn(&pdev->dev, "generated random MAC address %pM\n",
			 ndev->dev_addr);
	}

	ret = xgmac_phy_setup(priv);
	if (ret)
		return ret;

	xgmac_write_mac_addr(priv, ndev->dev_addr, 0);
	reg_set(priv, MMC_XGMAC_CONTROL,
		MMC_XGMAC_CONTROL_RESET | MMC_XGMAC_CONTROL_RSTONRD);
	reg_write(priv, XGMAC_TX_CONFIG, XGMAC_CORE_INIT_TX);
	reg_write(priv, XGMAC_RX_CONFIG, XGMAC_CORE_INIT_RX);
	reg_write(priv, XGMAC_WD_JB_TIMEOUT, XGMAC_PJE | XGMAC_PWE);
	reg_write(priv, XGMAC_VLAN_TAG, XGMAC_VLAN_EDVLP);
	reg_write(priv, XGMAC_LPI_TIMER_CTRL,
		  FIELD_PREP(XGMAC_LPI_LST, XGMAC_LPI_LST_DEFAULT) |
		  FIELD_PREP(XGMAC_LPI_TWT, XGMAC_LPI_TWT_DEFAULT));
	reg_write(priv, XGMAC_LPI_AUTO_EN, XGMAC_LPI_AUTO_EN_DEFAULT);
	xgmac_mib_irq_enable(priv);
	reg_write(priv, XGMAC_LPI_1US,
		  clk_get_rate(priv->csr_clk) / 1000000 - 1);

	ret = register_netdev(ndev);
	if (ret)
		goto phy_cleanup;

	return 0;
phy_cleanup:
	phylink_destroy(priv->phylink);
	if (priv->pcs_dev)
		xpcs_port_put(priv->pcs_dev);
	return ret;
}

static void xgmac_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct xgmac_priv *priv = netdev_priv(dev);

	unregister_netdev(dev);
	phylink_destroy(priv->phylink);
	if (priv->pcs_dev)
		xpcs_port_put(priv->pcs_dev);
	regmap_clear_bits(priv->ethsys, ETHSYS_RST, BIT(priv->id));
}

static const struct of_device_id xgmac_match[] = {
	{ .compatible = "siflower,sf21-xgmac" },
	{},
};
MODULE_DEVICE_TABLE(of, xgmac_match);

static struct platform_driver xgmac_driver = {
	.probe	= xgmac_probe,
	.remove	= xgmac_remove,
	.driver	= {
		.name		= "sfxgmac",
		.of_match_table	= xgmac_match,
	},
};
module_platform_driver(xgmac_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qingfang Deng <qingfang.deng@siflower.com.cn>");
MODULE_DESCRIPTION("Ethernet XGMAC driver for SF21A6826/SF21H8898 SoC");
