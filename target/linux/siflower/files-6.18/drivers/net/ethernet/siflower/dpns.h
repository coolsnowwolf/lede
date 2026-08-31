#ifndef __SF_DPNS_H__
#define __SF_DPNS_H__
#include <asm/mmio.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/reset.h>

#define PKT_ERR_STG_CFG2		0x80038
#define  ARP_REPLY_ERR_OP		GENMASK(18, 16)
#define  ARP_REPLY_ERR_DROP		BIT(18)
#define  ARP_REPLY_ERR_UP		BIT(17)
#define  ARP_REPLY_ERR_FWD		BIT(16)
#define  ARP_REQ_ERR_OP			GENMASK(14, 12)
#define  ARP_REQ_ERR_DROP		BIT(14)
#define  ARP_REQ_ERR_UP			BIT(13)
#define  ARP_REQ_ERR_FWD		BIT(12)

#define  PKT_ERR_ACTION_DROP		BIT(2)
#define  PKT_ERR_ACTION_UP		BIT(1)
#define  PKT_ERR_ACTION_FWD		BIT(0)

#define NPU_MIB_BASE			0x380000
#define NPU_MIB(x)			(NPU_MIB_BASE + (x) * 4)
#define NPU_MIB_PKT_RCV_PORT(x)		(NPU_MIB_BASE + 0x2000 + (x) * 4)
#define NPU_MIB_NCI_RD_DATA2		(NPU_MIB_BASE + 0x301c)
#define NPU_MIB_NCI_RD_DATA3		(NPU_MIB_BASE + 0x3020)

struct dpns_priv {
	void __iomem *ioaddr;
	struct clk *clk;
	struct reset_control *npu_rst;
	struct device *dev;
	struct dentry *debugfs;
};

static inline u32 dpns_r32(struct dpns_priv *priv, unsigned reg)
{
	return readl(priv->ioaddr + reg);
}

static inline void dpns_w32(struct dpns_priv *priv, unsigned reg, u32 val)
{
	writel(val, priv->ioaddr + reg);
}

static inline void dpns_rmw(struct dpns_priv *priv, unsigned reg, u32 clr,
			    u32 set)
{
	u32 val = dpns_r32(priv, reg);
	val &= ~clr;
	val |= set;
	dpns_w32(priv, reg, val);
}

int dpns_se_init(struct dpns_priv *priv);
int dpns_tmu_init(struct dpns_priv *priv);
void sf_dpns_debugfs_init(struct dpns_priv *priv);

#endif /* __SF_DPNS_H__ */
