#include <linux/of_device.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include "dpns.h"
#include "sf_dpns_tmu.h"

static u32 tmu_rm32(struct dpns_priv *priv, u32 reg, u32 mask, u32 shift)
{
	u32 t;

	t = dpns_r32(priv, reg);
	t &= mask;
	t >>= shift;

	return t;
}

static void tmu_rmw32(struct dpns_priv *priv, u32 reg, u32 mask, u32 shift, u32 val)
{
	u32 t;

	val <<= shift;
	val &= mask;
	t = dpns_r32(priv, reg);
	t &= ~mask;
	t |= val;
	dpns_w32(priv, reg, t);
}

static int is_valid_port_idx(struct dpns_priv *priv, u32 port)
{
	if (port >= TMU_MAX_PORT_CNT)
		return 0;

	return 1;
}

static int is_valid_queue_idx(u32 q)
{
	if (q >= QUE_MAX_NUM_PER_PORT)
		return 0;

	return 1;
}

static int is_valid_sched_idx(struct dpns_priv *priv, u32 sched)
{
	if (sched >= QUE_SCH_NUM_PER_PORT)
		return 0;

	return 1;
}

static int is_valid_shaper_idx(struct dpns_priv *priv, u32 shaper)
{
	if (shaper >= QUE_SHAPER_NUM_PER_PORT)
		return 0;

	return 1;
}

static int tmu_port_writel(struct dpns_priv *priv, u32 port, u32 reg, u32 val)
{
	if (!is_valid_port_idx(priv, port))
		return -EINVAL;

	dpns_w32(priv, TMU_PORT_BASE(port) + reg, val);
	return 0;
}

static int tmu_port_rm32(struct dpns_priv *priv, u32 port, u32 reg, u32 mask, u32 shift, u32 *val)
{
	if (!is_valid_port_idx(priv, port))
		return -EINVAL;

	*val = tmu_rm32(priv, TMU_PORT_BASE(port) + reg, mask, shift);
	return 0;
}

static int tmu_port_rmw32(struct dpns_priv *priv, u32 port, u32 reg, u32 mask, u32 shift, u32 val)
{
	if (!is_valid_port_idx(priv, port))
		return -EINVAL;

	tmu_rmw32(priv, TMU_PORT_BASE(port) + reg, mask, shift, val);

	return 0;
}

static int tmu_queue_writel(struct dpns_priv *priv, u32 port, u32 queue, u32 reg, u32 val)
{
	if (!is_valid_queue_idx(queue))
		return -EINVAL;

	return tmu_port_writel(priv, port, TMU_QUEUE_BASE(queue) + reg, val);
}

static int tmu_sched_writel(struct dpns_priv *priv, u32 port, u32 sched, u32 reg, u32 val)
{
	if (!is_valid_sched_idx(priv, sched))
		return -EINVAL;

	return tmu_port_writel(priv, port, TMU_SCHED_BASE(sched) + reg, val);
}

static int tmu_shaper_writel(struct dpns_priv *priv, u32 port, u32 shaper, u32 reg, u32 val)
{
	if (!is_valid_shaper_idx(priv, shaper))
		return -EINVAL;

	return tmu_port_writel(priv, port, TMU_SHAPER_BASE(shaper) + reg, val);
}

static int tmu_shaper_rmw32(struct dpns_priv *priv, u32 port, u32 shaper, u32 reg, u32 mask, u32 shift, u32 val)
{
	if (!is_valid_shaper_idx(priv, shaper))
		return -EINVAL;

	return tmu_port_rmw32(priv, port, TMU_SHAPER_BASE(shaper) + reg, mask, shift, val);
}

static int tdq_ctrl_is_configurable(struct dpns_priv *priv, u32 port)
{
	u32 val = 0;
	int err;

	if ((err = tmu_port_rm32(priv, port,
	                         TMU_TDQ_CTRL,
	                         TMU_TDQ_ALLOW_CFG,
	                         TMU_TDQ_ALLOW_CFG_SHIFT,
	                         &val))) {
		return 0;
	}

	return val;
}

static void tmu_port_queue_cfg(struct dpns_priv *priv, u32 port)
{
	int comp;

	for (comp = 0; comp < QUE_MAX_NUM_PER_PORT; comp++) {
		tmu_queue_writel(priv, port, comp, TMU_PORT_QUEUE_CFG0, 0x00011f00);

		tmu_queue_writel(priv, port, comp, TMU_PORT_QUEUE_CFG1, 0x00000000);
		tmu_queue_writel(priv, port, comp, TMU_PORT_QUEUE_CFG2, 0x00000000);
		tmu_queue_writel(priv, port, comp, TMU_PORT_QUEUE_STS0, 0x00000000);
		tmu_queue_writel(priv, port, comp, TMU_PORT_QUEUE_STS1, 0x00000000);
		tmu_queue_writel(priv, port, comp, TMU_PORT_QUEUE_STS2, 0x00000000);
		tmu_queue_writel(priv, port, comp, TMU_PORT_QUEUE_CFG3, 0x000005ee);
	}
}

static void tmu_port_sched_cfg(struct dpns_priv *priv, u32 port)
{
	int comp;
	for (comp = 0; comp < QUE_SCH_NUM_PER_PORT; comp++) {
		tmu_sched_writel(priv, port, comp, TMU_SCH_CTRL,      0x00000000);
		tmu_sched_writel(priv, port, comp, TMU_SCH_Q0_WEIGHT, 0x00000000);
		tmu_sched_writel(priv, port, comp, TMU_SCH_Q1_WEIGHT, 0x00000000);
		tmu_sched_writel(priv, port, comp, TMU_SCH_Q2_WEIGHT, 0x00000000);
		tmu_sched_writel(priv, port, comp, TMU_SCH_Q3_WEIGHT, 0x00000000);
		tmu_sched_writel(priv, port, comp, TMU_SCH_Q4_WEIGHT, 0x00000000);
		tmu_sched_writel(priv, port, comp, TMU_SCH_Q5_WEIGHT, 0x00000000);
		tmu_sched_writel(priv, port, comp, TMU_SCH_Q6_WEIGHT, 0x00000000);
		tmu_sched_writel(priv, port, comp, TMU_SCH_Q7_WEIGHT, 0x00000000);

		switch (comp) {
		case 0:
			tmu_sched_writel(priv, port, comp, TMU_SCH_QUEUE_ALLOC0, 0x03020100);
			tmu_sched_writel(priv, port, comp, TMU_SCH_QUEUE_ALLOC1, 0x08080808);
			break;

		case 1:
			tmu_sched_writel(priv, port, comp, TMU_SCH_QUEUE_ALLOC0, 0x06050400);
			tmu_sched_writel(priv, port, comp, TMU_SCH_QUEUE_ALLOC1, 0x08080807);
			break;

		default:
			break;
		}

		tmu_sched_writel(priv, port, comp, TMU_SCH_BIT_RATE, 0x00000000);

		if (comp == 0)
			tmu_sched_writel(priv, port, comp, TMU_SCH0_POS, 0x00000000);
	}
}

static void tmu_port_shaper_cfg(struct dpns_priv *priv, u32 port)
{
	int comp;
	for (comp = 0; comp < QUE_SHAPER_NUM_PER_PORT; comp++) {
		tmu_shaper_writel(priv, port, comp, TMU_SHP_CTRL,       0x00000000);
		tmu_shaper_writel(priv, port, comp, TMU_SHP_WEIGHT,     0x00000000);
		tmu_shaper_writel(priv, port, comp, TMU_SHP_CTRL2,      0x00000000);
		tmu_shaper_writel(priv, port, comp, TMU_SHP_MIN_CREDIT, 0x0003ff00);
		tmu_shaper_writel(priv, port, comp, TMU_SHP_MAX_CREDIT, 0x00000400);
		tmu_shaper_rmw32(priv, port, comp, TMU_SHP_CTRL2, TMU_SHP_POS, TMU_SHP_POS_SHIFT, comp);
	}
}

static void _tmu_reset(struct dpns_priv *priv, u32 port)
{
	tmu_port_queue_cfg(priv, port);
	tmu_port_sched_cfg(priv, port);
	tmu_port_shaper_cfg(priv, port);

	// Cause tmu shaper rate limit not include pkt preamble(8byte)/IFG(12byte)/FCS(4Byte)
	// so config 24 byte here
	tmu_port_writel(priv, port, TMU_TDQ_IFG, 0x00000018);

	if (tdq_ctrl_is_configurable(priv, port))
		tmu_port_writel(priv, port, TMU_TDQ_CTRL, 0x0000002f);
}

static int tmu_reset(struct dpns_priv *priv)
{
	int port;

	dpns_w32(priv, TMU_CTRL, 0x00000006);
	dpns_w32(priv, TMU_LLM_FIFO_CTRL0, 0x07fe07ff);
	dpns_w32(priv, TMU_LLM_FIFO_CTRL1, 0x00280024);

	for (port = 0; port < TMU_MAX_PORT_CNT; port++) {
		_tmu_reset(priv, port);
	}

	return 0;
}

int dpns_tmu_init(struct dpns_priv *priv)
{
	int err;

	if ((err = tmu_reset(priv)) != 0)
		return err;

	return err;
}
