#include "dpns.h"
#include <linux/iopoll.h>
#include <linux/bitfield.h>
#include "sf_dpns_se.h"

static int dpns_populate_table(struct dpns_priv *priv)
{
	void __iomem *ioaddr = priv->ioaddr;
	int ret, i;
	u32 reg;

	dpns_rmw(priv, SE_CONFIG0, SE_IPSPL_ZERO_LIMIT,
		 SE_IPORT_TABLE_VALID);
	dpns_w32(priv, SE_TB_WRDATA(0), 0xa0000);
	for (i = 0; i < 6; i++) {
		reg = SE_TB_OP_WR | FIELD_PREP(SE_TB_OP_REQ_ADDR, i) |
		      FIELD_PREP(SE_TB_OP_REQ_ID, SE_TB_IPORT);
		dpns_w32(priv, SE_TB_OP, reg);
		ret = readl_poll_timeout(ioaddr + SE_TB_OP, reg,
					 !(reg & SE_TB_OP_BUSY), 0, 100);
		if (ret)
			return ret;
	}

	return 0;
}

int dpns_se_init(struct dpns_priv *priv) {
	int ret;
	u32 reg;

	dpns_w32(priv, SE_CLR_RAM_CTRL, SE_CLR_RAM_ALL);
	dpns_w32(priv, SE_TCAM_CLR, SE_TCAM_CLR);

	ret = readl_poll_timeout(priv->ioaddr + SE_CLR_RAM_CTRL, reg, !reg, 0,
				 1000);
	if (ret)
		return ret;
	ret = readl_poll_timeout(priv->ioaddr + SE_TCAM_CLR, reg, !reg, 0,
				 1000);
	if (ret)
		return ret;

	/* Upload ARP packets which NPU considers invalid to host. */
	dpns_rmw(priv, PKT_ERR_STG_CFG2, ARP_REQ_ERR_OP | ARP_REPLY_ERR_OP,
		 ARP_REQ_ERR_UP | ARP_REPLY_ERR_UP);

	ret = dpns_populate_table(priv);
	return ret;
}
