#ifndef __XGMAC_EXT_H__
#define __XGMAC_EXT_H__

#include <linux/clk.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/phylink.h>

#define SF_GMAC_DUNMMY_ID 0xfa

#define GMAC_COMMON_STRUCT 			\
	void __iomem *ioaddr;			\
	struct device *dev; 			\
	struct clk *csr_clk;			\
	struct xgmac_dma_priv *dma; 		\
	struct regmap *ethsys; 			\
	struct phylink *phylink; 		\
	struct phylink_config phylink_config; 	\
	u8 id; 					\
	bool phy_supports_eee;			\
	bool tx_lpi_enabled;			\
	struct platform_device *pcs_dev;	\
	void *dp_port;

struct gmac_common {
	GMAC_COMMON_STRUCT;
};

#endif
