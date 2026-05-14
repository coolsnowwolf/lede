#define pr_fmt(fmt)	"xpcs: " fmt

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/interrupt.h> 
#include <linux/clk.h>
#include <linux/mfd/syscon.h>
#include <linux/phylink.h>
#include <linux/of_platform.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/regmap.h>
#include <asm-generic/bug.h>

#include "sfxpcs.h"
#include "eth.h"

#define DEV_MASK    GENMASK(20, 16)
#define REG_MASK    GENMASK(15, 0)

#define DW_PCS_PORTS		4
#define DW_QSGMII_MMD1          0x1a
#define DW_QSGMII_MMD2          0x1b
#define DW_QSGMII_MMD3          0x1c
#define MDIO_CTRL1			MII_BMCR
#define MDIO_CTRL1_RESET    BMCR_RESET

static inline void *PDE_DATA(const struct inode *inode) {BUG(); return NULL;};

enum {
	XPCS_CLK_REF,
	XPCS_CLK_EEE,
	XPCS_CLK_CSR,
	XPCS_NUM_CLKS
};

struct xpcs_port {
	struct phylink_pcs pcs;
	unsigned int index;
};

struct xpcs_priv {
	void __iomem *ioaddr;
	struct regmap *ethsys;
	struct clk_bulk_data clks[XPCS_NUM_CLKS];
	u8 power_save_count;
	u8 port_count;
	u8 id;
	struct xpcs_port ports[DW_PCS_PORTS];
};

static int xpcs_qsgmii_port_to_devad(unsigned int port)
{
	switch (port) {
	case 0:
		return MDIO_MMD_VEND2;
	case 1:
		return DW_QSGMII_MMD1;
	case 2:
		return DW_QSGMII_MMD2;
	case 3:
		return DW_QSGMII_MMD3;
	default:
		BUG();
		return -EINVAL;
	}
}

static u16 xpcs_read(struct xpcs_priv *priv, int devad, int reg)
{
	ulong r;

	r = FIELD_PREP(REG_MASK, reg) | FIELD_PREP(DEV_MASK, devad);
	r <<= 2;

	return readw_relaxed(priv->ioaddr + r);
}

static void xpcs_write(struct xpcs_priv *priv, int devad, int reg, u16 val)
{
	ulong r;

	r = FIELD_PREP(REG_MASK, reg) | FIELD_PREP(DEV_MASK, devad);
	r <<= 2;

	writew_relaxed(val, priv->ioaddr + r);
}

static inline void xpcs_clear(struct xpcs_priv *priv, int devad, int reg, u16 clear)
{
	xpcs_write(priv, devad, reg, xpcs_read(priv, devad, reg) & ~clear);
}

static inline void xpcs_set(struct xpcs_priv *priv, int devad, int reg, u16 set)
{
	xpcs_write(priv, devad, reg, xpcs_read(priv, devad, reg) | set);
}

static int xpcs_poll_reset(struct xpcs_priv *priv, int devad)
{
	int timeout = 100000;

	while (xpcs_read(priv, devad, MDIO_CTRL1) & MDIO_CTRL1_RESET) {
		if (!--timeout) {
			pr_err("Timed out waiting for reset\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int xpcs_poll_pg(struct xpcs_priv *priv, int devad, u16 val)
{
	u32 timeout = 0;

	while (FIELD_GET(PSEQ_STATE,
				xpcs_read(priv, devad, DW_VR_MII_DIG_STS)) != val) {
		if (timeout >= 100) {
			pr_err("Timed out waiting for power state\n");
			return -ETIMEDOUT;
		}
		timeout++;
		udelay(100);
	}
	return 0;
}

static int xpcs_serdes_power_down(struct xpcs_priv *priv)
{
	xpcs_write(priv, MDIO_MMD_VEND2, MII_BMCR, BMCR_PDOWN);
	return xpcs_poll_pg(priv, MDIO_MMD_VEND2, PSEQ_STATE_DOWN);
}

static int xpcs_serdes_power_up(struct xpcs_priv *priv)
{
	/* When powered down, this register cannot be read.
	 * speed/duplex/AN will be configured in pcs_config/pcs_link_up.
	 */
	xpcs_write(priv, MDIO_MMD_VEND2, MII_BMCR, 0);
	return xpcs_poll_pg(priv, MDIO_MMD_VEND2, PSEQ_STATE_GOOD);
}

/* Read AN result for 1000Base-X/2500Base-X */
static void xpcs_8023z_resolve_link(struct xpcs_priv *priv,
				    struct phylink_link_state *state,
				    int fd_bit)
{
	bool tx_pause, rx_pause;
	u16 adv, lpa;

	adv = xpcs_read(priv, MDIO_MMD_VEND2, MII_ADVERTISE);
	lpa = xpcs_read(priv, MDIO_MMD_VEND2, MII_LPA);

	mii_lpa_mod_linkmode_x(state->lp_advertising, lpa, fd_bit);

	if (linkmode_test_bit(fd_bit, state->advertising) &&
	    linkmode_test_bit(fd_bit, state->lp_advertising)) {
		state->duplex = DUPLEX_FULL;
	} else {
		/* negotiation failure */
		state->link = false;
	}

	linkmode_resolve_pause(state->advertising, state->lp_advertising,
			       &tx_pause, &rx_pause);

	if (tx_pause)
		state->pause |= MLO_PAUSE_TX;
	if (rx_pause)
		state->pause |= MLO_PAUSE_RX;
}

static void xpcs_get_state(struct phylink_pcs *pcs,
			   struct phylink_link_state *state)
{
	struct xpcs_port *port;
	struct xpcs_priv *priv;
	u16 intrsts, bmsr;
	int mmd;

	port = container_of(pcs, struct xpcs_port, pcs);
	priv = container_of(port, struct xpcs_priv, ports[port->index]);
	bmsr = xpcs_read(priv, MDIO_MMD_VEND2, MII_BMSR);

	state->link = !!(bmsr & BMSR_LSTATUS);
	state->an_complete = !!(bmsr & BMSR_ANEGCOMPLETE);
	if (!state->link)
		return;

	switch (state->interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_QSGMII:
		mmd = xpcs_qsgmii_port_to_devad(port->index);
		/* For SGMII/QSGMII, link speed and duplex can be read from
		 * DW_VR_MII_AN_INTR_STS */
		intrsts = xpcs_read(priv, mmd, DW_VR_MII_AN_INTR_STS);

		state->link = !!(intrsts & DW_VR_MII_C37_ANSGM_SP_LNKSTS);
		if (!state->link)
			break;

		switch (FIELD_GET(DW_VR_MII_AN_STS_C37_ANSGM_SP, intrsts)) {
		case DW_VR_MII_C37_ANSGM_SP_10:
			state->speed = SPEED_10;
			break;
		case DW_VR_MII_C37_ANSGM_SP_100:
			state->speed = SPEED_100;
			break;
		case DW_VR_MII_C37_ANSGM_SP_1000:
			state->speed = SPEED_1000;
			break;
		}

		state->duplex = (intrsts & DW_VR_MII_AN_STS_C37_ANSGM_FD) ?
				DUPLEX_FULL : DUPLEX_HALF;
		break;
	case PHY_INTERFACE_MODE_1000BASEX:
		state->speed = SPEED_1000;
		xpcs_8023z_resolve_link(priv, state,
					ETHTOOL_LINK_MODE_1000baseX_Full_BIT);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		state->speed = SPEED_2500;
		xpcs_8023z_resolve_link(priv, state,
					ETHTOOL_LINK_MODE_2500baseX_Full_BIT);
		break;
	default:
		break;
	}
}

static void xpcs_qsgmii_init(struct xpcs_priv *priv)
{
	u16 reg;

	reg = xpcs_read(priv, MDIO_MMD_VEND2, DW_VR_MII_AN_CTRL);
	/* Already configured for QSGMII? skip. */
	if (FIELD_GET(DW_VR_MII_PCS_MODE_MASK, reg) == DW_VR_MII_PCS_MODE_C37_QSGMII)
		return;

	reg = FIELD_PREP(DW_VR_MII_PCS_MODE_MASK, DW_VR_MII_PCS_MODE_C37_QSGMII);
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_AN_CTRL, reg);

	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL1, 0x28);

	reg = xpcs_read(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL0);
	reg &= ~LANE_10BIT_SEL;
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL0, reg);

	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MISC_CTRL1, 0x0);

	reg = xpcs_read(priv, MDIO_MMD_VEND2, DW_VR_MII_DIG_CTRL1);
	reg &= ~DW_VR_MII_DIG_CTRL1_2G5_EN;
	reg &= ~DW_VR_MII_DIG_CTRL1_CL37_TMR_OVR_RIDE;
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_DIG_CTRL1, reg);

	xpcs_serdes_power_down(priv);
	xpcs_serdes_power_up(priv);
}

static void xpcs_1000basex_sgmii_common_init(struct xpcs_priv *priv)
{
	u16 reg;

	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL1, 0x28);

	reg = xpcs_read(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL0);
	reg |= LANE_10BIT_SEL;
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL0, reg);

	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MISC_CTRL1, 0xa);

	reg = xpcs_read(priv, MDIO_MMD_VEND2, DW_VR_MII_DIG_CTRL1);
	reg &= ~DW_VR_MII_DIG_CTRL1_2G5_EN;
	reg &= ~DW_VR_MII_DIG_CTRL1_CL37_TMR_OVR_RIDE;
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_DIG_CTRL1, reg);

	xpcs_serdes_power_down(priv);
	xpcs_serdes_power_up(priv);
}

static void xpcs_1000basex_init(struct xpcs_priv *priv)
{
	u16 reg;

	reg = FIELD_PREP(DW_VR_MII_PCS_MODE_MASK, DW_VR_MII_PCS_MODE_C37_1000BASEX);
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_AN_CTRL, reg);

	xpcs_1000basex_sgmii_common_init(priv);
}

static void xpcs_sgmii_init(struct xpcs_priv *priv)
{
	u16 reg;

	reg = FIELD_PREP(DW_VR_MII_PCS_MODE_MASK, DW_VR_MII_PCS_MODE_C37_SGMII);
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_AN_CTRL, reg);

	xpcs_1000basex_sgmii_common_init(priv);
}

static void xpcs_2500basex_init(struct xpcs_priv *priv)
{
	u16 reg;

	reg = FIELD_PREP(DW_VR_MII_PCS_MODE_MASK, DW_VR_MII_PCS_MODE_C37_1000BASEX);
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_AN_CTRL, reg);

	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL1, 0x32);

	reg = xpcs_read(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL0);
	reg |= LANE_10BIT_SEL;
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MPLL_CTRL0, reg);

	reg = xpcs_read(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_RXGENCTRL0);
	reg |= RX_ALIGN_EN_0;
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_RXGENCTRL0, reg);

	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_MP_6G_MISC_CTRL1, 0x5);

	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_LINK_TIMER_CTRL,
		   DW_VR_MII_LINK_TIMER_2500BASEX);

	reg = xpcs_read(priv, MDIO_MMD_VEND2, DW_VR_MII_DIG_CTRL1);
	reg |= DW_VR_MII_DIG_CTRL1_2G5_EN;
	reg |= DW_VR_MII_DIG_CTRL1_CL37_TMR_OVR_RIDE;
	xpcs_write(priv, MDIO_MMD_VEND2, DW_VR_MII_DIG_CTRL1, reg);

	xpcs_serdes_power_down(priv);
	xpcs_serdes_power_up(priv);
}

static int xpcs_config(struct phylink_pcs *pcs, unsigned int mode,
		       phy_interface_t interface,
		       const unsigned long *advertising,
		       bool permit_pause_to_mac)
{
	struct xpcs_port *port;
	struct xpcs_priv *priv;
	u16 val;
	int mmd;

	port = container_of(pcs, struct xpcs_port, pcs);
	priv = container_of(port, struct xpcs_priv, ports[port->index]);

	/* Port 1,2,3 only exist in QSGMII mode */
	if (port->index && interface != PHY_INTERFACE_MODE_QSGMII)
		return -EINVAL;

	/* Disable AN */
	mmd = xpcs_qsgmii_port_to_devad(port->index);
	xpcs_clear(priv, mmd, MII_BMCR, BMCR_ANENABLE);

	switch (interface) {
	case PHY_INTERFACE_MODE_QSGMII:
		xpcs_qsgmii_init(priv);
		break;
	case PHY_INTERFACE_MODE_SGMII:
		xpcs_sgmii_init(priv);
		break;
	case PHY_INTERFACE_MODE_2500BASEX:
		xpcs_2500basex_init(priv);
		break;
	case PHY_INTERFACE_MODE_1000BASEX:
		xpcs_1000basex_init(priv);
		break;
	default:
		return -EINVAL;
	}

	/* Enable interrupt for in-band status */
	val = xpcs_read(priv, mmd, DW_VR_MII_AN_CTRL);
	if (phylink_autoneg_inband(mode))
		val |= DW_VR_MII_AN_INTR_EN;
	else
		val &= ~DW_VR_MII_AN_INTR_EN;
	xpcs_write(priv, mmd, DW_VR_MII_AN_CTRL, val);

	if (interface != PHY_INTERFACE_MODE_2500BASEX) {
		val = xpcs_read(priv, mmd, DW_VR_MII_DIG_CTRL1);
		/* Enable speed auto switch for SGMII/QSGMII */
		val |= DW_VR_MII_DIG_CTRL1_MAC_AUTO_SW;
		xpcs_write(priv, mmd, DW_VR_MII_DIG_CTRL1, val);
	}

	/* Configure AN ADV for 802.3z modes */
	if (phy_interface_mode_is_8023z(interface)) {
		int fd_bit;
		u16 adv;

		fd_bit = interface == PHY_INTERFACE_MODE_1000BASEX ?
			 ETHTOOL_LINK_MODE_1000baseX_Full_BIT :
			 ETHTOOL_LINK_MODE_2500baseX_Full_BIT;
		adv = linkmode_adv_to_mii_adv_x(advertising, fd_bit);

		xpcs_write(priv, MDIO_MMD_VEND2, MII_ADVERTISE, adv);
	}

	/* Enable AN */
	if (interface != PHY_INTERFACE_MODE_2500BASEX)
		xpcs_write(priv, mmd, MII_BMCR, BMCR_ANENABLE);

	return 0;
}

static void xpcs_an_restart(struct phylink_pcs *pcs)
{
	struct xpcs_port *port;
	struct xpcs_priv *priv;
	int mmd;

	port = container_of(pcs, struct xpcs_port, pcs);
	priv = container_of(port, struct xpcs_priv, ports[port->index]);

	mmd = xpcs_qsgmii_port_to_devad(port->index);
	xpcs_set(priv, mmd, MII_BMCR, BMCR_ANRESTART);
}

static void xpcs_link_up(struct phylink_pcs *pcs, unsigned int mode,
			 phy_interface_t interface, int speed, int duplex)
{
	struct xpcs_port *port;
	struct xpcs_priv *priv;
	u16 bmcr;
	int mmd;

	/* Skip speed and duplex configuration for SGMII/QSGMII in-band */
	if (phylink_autoneg_inband(mode) &&
	    !phy_interface_mode_is_8023z(interface))
		return;

	/* 1000/2500 BaseX should only use the max speed */
	if (phy_interface_mode_is_8023z(interface))
		speed = SPEED_1000;

	port = container_of(pcs, struct xpcs_port, pcs);
	priv = container_of(port, struct xpcs_priv, ports[port->index]);

	mmd = xpcs_qsgmii_port_to_devad(port->index);
	bmcr = xpcs_read(priv, mmd, MII_BMCR);
	bmcr &= ~(BMCR_SPEED1000 | BMCR_SPEED100 | BMCR_SPEED10);

	switch (speed) {
	case SPEED_2500:
	case SPEED_1000:
		bmcr |= BMCR_SPEED1000;
		break;
	case SPEED_100:
		bmcr |= BMCR_SPEED100;
		break;
	case SPEED_10:
		bmcr |= BMCR_SPEED10;
		break;
	}
	if (duplex == DUPLEX_FULL)
		bmcr |= BMCR_FULLDPLX;
	else
		bmcr &= ~BMCR_FULLDPLX;

	xpcs_write(priv, mmd, MII_BMCR, bmcr);
}

static const struct phylink_pcs_ops xpcs_phylink_ops = {
	.pcs_get_state	= xpcs_get_state,
	.pcs_config	= xpcs_config,
	.pcs_an_restart	= xpcs_an_restart,
	.pcs_link_up	= xpcs_link_up,
};

struct phylink_pcs *xpcs_port_get(struct platform_device *pdev,
				  unsigned int port)
{
	struct xpcs_priv *priv = platform_get_drvdata(pdev);

	if (port >= DW_PCS_PORTS)
		return ERR_PTR(-EINVAL);

	priv->port_count++;
	priv->power_save_count++;
	return &priv->ports[port].pcs;
}
EXPORT_SYMBOL(xpcs_port_get);



void xpcs_port_put(struct platform_device *pdev)
{
	struct xpcs_priv *priv = platform_get_drvdata(pdev);

	priv->port_count--;
	priv->power_save_count--;
}
EXPORT_SYMBOL(xpcs_port_put);

static irqreturn_t xpcs_irq(int irq, void *dev_id)
{
	struct xpcs_priv *priv = dev_id;
	irqreturn_t ret = IRQ_NONE;
	int i;

	for (i = 0; i < DW_PCS_PORTS; i++) {
		int mmd = xpcs_qsgmii_port_to_devad(i);
		u16 intrsts = xpcs_read(priv, mmd, DW_VR_MII_AN_INTR_STS);
		bool up;

		if (!(intrsts & DW_VR_MII_C37_ANCMPLT_INTR))
			continue;

		xpcs_write(priv, mmd, DW_VR_MII_AN_INTR_STS, 0);
		up = xpcs_read(priv, MDIO_MMD_VEND2, MII_BMSR) & BMSR_LSTATUS;
		up |= intrsts & DW_VR_MII_C37_ANSGM_SP_LNKSTS;
		phylink_pcs_change(&priv->ports[i].pcs, up);
		ret = IRQ_HANDLED;
	}

	return ret;
}

static int xpcs_probe(struct platform_device *pdev)
{
	struct xpcs_priv *priv;
	struct resource *r;
	int ret, i;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);
	for (i = 0; i < DW_PCS_PORTS; i++) {
		priv->ports[i].index = i;
		priv->ports[i].pcs.ops = &xpcs_phylink_ops;
	}

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->ioaddr = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(priv->ioaddr))
		return PTR_ERR(priv->ioaddr);

	priv->id = !!(r->start & BIT(24));
	priv->ethsys = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
						       "ethsys");
	if (IS_ERR(priv->ethsys))
		return PTR_ERR(priv->ethsys);

	priv->clks[XPCS_CLK_REF].id = "ref";
	priv->clks[XPCS_CLK_EEE].id = "eee";
	priv->clks[XPCS_CLK_CSR].id = "csr";
	ret = devm_clk_bulk_get(&pdev->dev, XPCS_NUM_CLKS, priv->clks);
	if (ret)
		return ret;

	ret = clk_bulk_prepare_enable(XPCS_NUM_CLKS, priv->clks);
	if (ret)
		return ret;

	ret = regmap_set_bits(priv->ethsys, ETHSYS_RST, BIT(5 + priv->id));
	if (ret)
		return ret;

	ret = regmap_write(priv->ethsys,
			   ETHSYS_QSG_CTRL + priv->id * sizeof(u32), 0x601);
	if (ret)
		return ret;

	/* set ethtsuclk to 100MHz */
	ret = clk_set_rate(priv->clks[XPCS_CLK_EEE].clk, 100000000);
	if (ret)
		return ret;

	/* Soft reset the PCS */
	xpcs_write(priv, MDIO_MMD_VEND2, MII_BMCR, BMCR_RESET);
	ret = xpcs_poll_reset(priv, MDIO_MMD_VEND2);
	if (ret)
		return ret;

	/* Enable EEE */
	xpcs_set(priv, MDIO_MMD_VEND2, DW_VR_MII_EEE_MCTRL0,
		 DW_VR_MII_EEE_LTX_EN | DW_VR_MII_EEE_LRX_EN);

	/* Start from the power up state */
	ret = xpcs_serdes_power_up(priv);
	if (ret)
		return ret;

	ret = platform_get_irq(pdev, 0);
	if (ret < 0)
		return ret;

	ret = devm_request_irq(&pdev->dev, ret, xpcs_irq, 0, KBUILD_MODNAME, priv);
	if (ret)
		return ret;

	return 0;
}

static void xpcs_remove(struct platform_device *pdev)
{
	struct xpcs_priv *priv = platform_get_drvdata(pdev);

	clk_bulk_disable_unprepare(XPCS_NUM_CLKS, priv->clks);
	regmap_clear_bits(priv->ethsys, ETHSYS_RST, BIT(5 + priv->id));
}

static const struct of_device_id xpcs_match[] = {
	{ .compatible = "siflower,sf21-xpcs" },
	{},
};
MODULE_DEVICE_TABLE(of, xpcs_match);

static struct platform_driver xpcs_driver = {
	.probe	= xpcs_probe,
	.remove_new	= xpcs_remove,
	.driver	= {
		.name		= "sfxpcs",
		.of_match_table	= xpcs_match,
	},
};
module_platform_driver(xpcs_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qingfang Deng <qingfang.deng@siflower.com.cn>");
MODULE_DESCRIPTION("XPCS driver for SF21A6826/SF21H8898 SoC");
