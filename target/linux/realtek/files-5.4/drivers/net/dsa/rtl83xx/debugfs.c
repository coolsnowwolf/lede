// SPDX-License-Identifier: GPL-2.0-only

#include <linux/debugfs.h>
#include <linux/kernel.h>

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include "rtl83xx.h"

#define RTL838X_DRIVER_NAME "rtl838x"

#define RTL8380_LED_GLB_CTRL			(0xA000)
#define RTL8380_LED_MODE_SEL			(0x1004)
#define RTL8380_LED_MODE_CTRL			(0xA004)
#define RTL8380_LED_P_EN_CTRL			(0xA008)
#define RTL8380_LED_SW_CTRL			(0xA00C)
#define RTL8380_LED0_SW_P_EN_CTRL		(0xA010)
#define RTL8380_LED1_SW_P_EN_CTRL		(0xA014)
#define RTL8380_LED2_SW_P_EN_CTRL		(0xA018)
#define RTL8380_LED_SW_P_CTRL(p)		(0xA01C + (((p) << 2)))

#define RTL8390_LED_GLB_CTRL			(0x00E4)
#define RTL8390_LED_SET_2_3_CTRL		(0x00E8)
#define RTL8390_LED_SET_0_1_CTRL		(0x00EC)
#define RTL8390_LED_COPR_SET_SEL_CTRL(p)	(0x00F0 + (((p >> 4) << 2)))
#define RTL8390_LED_FIB_SET_SEL_CTRL(p)		(0x0100 + (((p >> 4) << 2)))
#define RTL8390_LED_COPR_PMASK_CTRL(p)		(0x0110 + (((p >> 5) << 2)))
#define RTL8390_LED_FIB_PMASK_CTRL(p)		(0x00118 + (((p >> 5) << 2)))
#define RTL8390_LED_COMBO_CTRL(p)		(0x0120 + (((p >> 5) << 2)))
#define RTL8390_LED_SW_CTRL			(0x0128)
#define RTL8390_LED_SW_P_EN_CTRL(p)		(0x012C + (((p / 10) << 2)))
#define RTL8390_LED_SW_P_CTRL(p)		(0x0144 + (((p) << 2)))

#define RTL838X_MIR_QID_CTRL(grp)		(0xAD44 + (((grp) << 2)))
#define RTL838X_MIR_RSPAN_VLAN_CTRL(grp)	(0xA340 + (((grp) << 2)))
#define RTL838X_MIR_RSPAN_VLAN_CTRL_MAC(grp)	(0xAA70 + (((grp) << 2)))
#define RTL838X_MIR_RSPAN_TX_CTRL		(0xA350)
#define RTL838X_MIR_RSPAN_TX_TAG_RM_CTRL	(0xAA80)
#define RTL838X_MIR_RSPAN_TX_TAG_EN_CTRL	(0xAA84)
#define RTL839X_MIR_RSPAN_VLAN_CTRL(grp)	(0xA340 + (((grp) << 2)))
#define RTL839X_MIR_RSPAN_TX_CTRL		(0x69b0)
#define RTL839X_MIR_RSPAN_TX_TAG_RM_CTRL	(0x2550)
#define RTL839X_MIR_RSPAN_TX_TAG_EN_CTRL	(0x2554)
#define RTL839X_MIR_SAMPLE_RATE_CTRL                (0x2558)

int rtl83xx_port_get_stp_state(struct rtl838x_switch_priv *priv, int port);
void rtl83xx_port_stp_state_set(struct dsa_switch *ds, int port, u8 state);
void rtl83xx_fast_age(struct dsa_switch *ds, int port);
u32 rtl838x_get_egress_rate(struct rtl838x_switch_priv *priv, int port);
u32 rtl839x_get_egress_rate(struct rtl838x_switch_priv *priv, int port);
int rtl838x_set_egress_rate(struct rtl838x_switch_priv *priv, int port, u32 rate);
int rtl839x_set_egress_rate(struct rtl838x_switch_priv *priv, int port, u32 rate);

static ssize_t rtl838x_common_read(char __user *buffer, size_t count,
					loff_t *ppos, unsigned int value)
{
	char *buf;
	ssize_t len;

	if (*ppos != 0)
		return 0;

	buf = kasprintf(GFP_KERNEL, "0x%08x\n", value);
	if (!buf)
		return -ENOMEM;

	if (count < strlen(buf)) {
		kfree(buf);
		return -ENOSPC;
	}

	len = simple_read_from_buffer(buffer, count, ppos, buf, strlen(buf));
	kfree(buf);

	return len;
}

static ssize_t rtl838x_common_write(const char __user *buffer, size_t count,
				 loff_t *ppos, unsigned int *value)
{
	char b[32];
	ssize_t len;
	int ret;

	if (*ppos != 0)
		return -EINVAL;

	if (count >= sizeof(b))
		return -ENOSPC;

	len = simple_write_to_buffer(b, sizeof(b) - 1, ppos,
				     buffer, count);
	if (len < 0)
		return len;

	b[len] = '\0';
	ret = kstrtouint(b, 16, value);
	if (ret)
		return -EIO;

	return len;
}

static ssize_t stp_state_read(struct file *filp, char __user *buffer, size_t count,
			     loff_t *ppos)
{
	struct rtl838x_port *p = filp->private_data;
	struct dsa_switch *ds = p->dp->ds;
	int value = rtl83xx_port_get_stp_state(ds->priv, p->dp->index);

	if (value < 0)
		return -EINVAL;

	return rtl838x_common_read(buffer, count, ppos, (u32)value);
}

static ssize_t stp_state_write(struct file *filp, const char __user *buffer,
				size_t count, loff_t *ppos)
{
	struct rtl838x_port *p = filp->private_data;
	u32 value;
	size_t res = rtl838x_common_write(buffer, count, ppos, &value);
	if (res < 0)
		return res;

	rtl83xx_port_stp_state_set(p->dp->ds, p->dp->index, (u8)value);

	return res;
}

static const struct file_operations stp_state_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = stp_state_read,
	.write = stp_state_write,
};

static ssize_t age_out_read(struct file *filp, char __user *buffer, size_t count,
			     loff_t *ppos)
{
	struct rtl838x_port *p = filp->private_data;
	struct dsa_switch *ds = p->dp->ds;
	struct rtl838x_switch_priv *priv = ds->priv;
	int value = sw_r32(priv->r->l2_port_aging_out);

	if (value < 0)
		return -EINVAL;

	return rtl838x_common_read(buffer, count, ppos, (u32)value);
}

static ssize_t age_out_write(struct file *filp, const char __user *buffer,
				size_t count, loff_t *ppos)
{
	struct rtl838x_port *p = filp->private_data;
	u32 value;
	size_t res = rtl838x_common_write(buffer, count, ppos, &value);
	if (res < 0)
		return res;

	rtl83xx_fast_age(p->dp->ds, p->dp->index);

	return res;
}

static const struct file_operations age_out_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = age_out_read,
	.write = age_out_write,
};

static ssize_t port_egress_rate_read(struct file *filp, char __user *buffer, size_t count,
				loff_t *ppos)
{
	struct rtl838x_port *p = filp->private_data;
	struct dsa_switch *ds = p->dp->ds;
	struct rtl838x_switch_priv *priv = ds->priv;
	int value;
	if (priv->family_id == RTL8380_FAMILY_ID)
		value = rtl838x_get_egress_rate(priv, p->dp->index);
	else
		value = rtl839x_get_egress_rate(priv, p->dp->index);

	if (value < 0)
		return -EINVAL;

	return rtl838x_common_read(buffer, count, ppos, (u32)value);
}

static ssize_t port_egress_rate_write(struct file *filp, const char __user *buffer,
				size_t count, loff_t *ppos)
{
	struct rtl838x_port *p = filp->private_data;
	struct dsa_switch *ds = p->dp->ds;
	struct rtl838x_switch_priv *priv = ds->priv;
	u32 value;
	size_t res = rtl838x_common_write(buffer, count, ppos, &value);
	if (res < 0)
		return res;

	if (priv->family_id == RTL8380_FAMILY_ID)
		rtl838x_set_egress_rate(priv, p->dp->index, value);
	else
		rtl839x_set_egress_rate(priv, p->dp->index, value);

	return res;
}

static const struct file_operations port_egress_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = port_egress_rate_read,
	.write = port_egress_rate_write,
};


static const struct debugfs_reg32 port_ctrl_regs[] = {
	{ .name = "port_isolation", .offset = RTL838X_PORT_ISO_CTRL(0), },
	{ .name = "mac_force_mode", .offset = RTL838X_MAC_FORCE_MODE_CTRL, },
};

void rtl838x_dbgfs_cleanup(struct rtl838x_switch_priv *priv)
{
	debugfs_remove_recursive(priv->dbgfs_dir);

//	kfree(priv->dbgfs_entries);
}

static int rtl838x_dbgfs_port_init(struct dentry *parent, struct rtl838x_switch_priv *priv,
				   int port)
{
	struct dentry *port_dir;
	struct debugfs_regset32 *port_ctrl_regset;

	port_dir = debugfs_create_dir(priv->ports[port].dp->name, parent);

	if (priv->family_id == RTL8380_FAMILY_ID) {
		debugfs_create_x32("storm_rate_uc", 0644, port_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_STORM_CTRL_PORT_UC(port)));

		debugfs_create_x32("storm_rate_mc", 0644, port_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_STORM_CTRL_PORT_MC(port)));

		debugfs_create_x32("storm_rate_bc", 0644, port_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_STORM_CTRL_PORT_BC(port)));

		debugfs_create_x32("vlan_port_tag_sts_ctrl", 0644, port_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_VLAN_PORT_TAG_STS_CTRL 
				+ (port << 2)));
	} else {
		debugfs_create_x32("storm_rate_uc", 0644, port_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_STORM_CTRL_PORT_UC_0(port)));

		debugfs_create_x32("storm_rate_mc", 0644, port_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_STORM_CTRL_PORT_MC_0(port)));

		debugfs_create_x32("storm_rate_bc", 0644, port_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_STORM_CTRL_PORT_BC_0(port)));

		debugfs_create_x32("vlan_port_tag_sts_ctrl", 0644, port_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_VLAN_PORT_TAG_STS_CTRL
				+ (port << 2)));
	}

	debugfs_create_u32("id", 0444, port_dir, (u32 *)&priv->ports[port].dp->index);

	port_ctrl_regset = devm_kzalloc(priv->dev, sizeof(*port_ctrl_regset), GFP_KERNEL);
	if (!port_ctrl_regset)
		return -ENOMEM;

	port_ctrl_regset->regs = port_ctrl_regs;
	port_ctrl_regset->nregs = ARRAY_SIZE(port_ctrl_regs);
	port_ctrl_regset->base = (void *)(RTL838X_SW_BASE + (port << 2));
	debugfs_create_regset32("port_ctrl", 0400, port_dir, port_ctrl_regset);

	debugfs_create_file("stp_state", 0600, port_dir, &priv->ports[port], &stp_state_fops);
	debugfs_create_file("age_out", 0600, port_dir, &priv->ports[port], &age_out_fops);
	debugfs_create_file("port_egress_rate", 0600, port_dir, &priv->ports[port],
			    &port_egress_fops);
	return 0;
}

static int rtl838x_dbgfs_leds(struct dentry *parent, struct rtl838x_switch_priv *priv)
{
	struct dentry *led_dir;
	int p;
	char led_sw_p_ctrl_name[20];
	char port_led_name[20];

	led_dir = debugfs_create_dir("led", parent);

	if (priv->family_id == RTL8380_FAMILY_ID) {
		debugfs_create_x32("led_glb_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED_GLB_CTRL));
		debugfs_create_x32("led_mode_sel", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED_MODE_SEL));
		debugfs_create_x32("led_mode_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED_MODE_CTRL));
		debugfs_create_x32("led_p_en_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED_P_EN_CTRL));
		debugfs_create_x32("led_sw_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED_SW_CTRL));
		debugfs_create_x32("led0_sw_p_en_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED0_SW_P_EN_CTRL));
		debugfs_create_x32("led1_sw_p_en_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED1_SW_P_EN_CTRL));
		debugfs_create_x32("led2_sw_p_en_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED2_SW_P_EN_CTRL));
		for (p = 0; p < 28; p++) {
			snprintf(led_sw_p_ctrl_name, sizeof(led_sw_p_ctrl_name),
				 "led_sw_p_ctrl.%02d", p);
			debugfs_create_x32(led_sw_p_ctrl_name, 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8380_LED_SW_P_CTRL(p)));
		}
	} else if (priv->family_id == RTL8390_FAMILY_ID) {
		debugfs_create_x32("led_glb_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_GLB_CTRL));
		debugfs_create_x32("led_set_2_3", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_SET_2_3_CTRL));
		debugfs_create_x32("led_set_0_1", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_SET_0_1_CTRL));
		for (p = 0; p < 4; p++) {
			snprintf(port_led_name, sizeof(port_led_name), "led_copr_set_sel.%1d", p);
			debugfs_create_x32(port_led_name, 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_COPR_SET_SEL_CTRL(p << 4)));
			snprintf(port_led_name, sizeof(port_led_name), "led_fib_set_sel.%1d", p);
			debugfs_create_x32(port_led_name, 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_FIB_SET_SEL_CTRL(p << 4)));
		}
		debugfs_create_x32("led_copr_pmask_ctrl_0", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_COPR_PMASK_CTRL(0)));
		debugfs_create_x32("led_copr_pmask_ctrl_1", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_COPR_PMASK_CTRL(32)));
		debugfs_create_x32("led_fib_pmask_ctrl_0", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_FIB_PMASK_CTRL(0)));
		debugfs_create_x32("led_fib_pmask_ctrl_1", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_FIB_PMASK_CTRL(32)));
		debugfs_create_x32("led_combo_ctrl_0", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_COMBO_CTRL(0)));
		debugfs_create_x32("led_combo_ctrl_1", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_COMBO_CTRL(32)));
		debugfs_create_x32("led_sw_ctrl", 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_SW_CTRL));
		for (p = 0; p < 5; p++) {
			snprintf(port_led_name, sizeof(port_led_name), "led_sw_p_en_ctrl.%1d", p);
			debugfs_create_x32(port_led_name, 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_SW_P_EN_CTRL(p * 10)));
		}
		for (p = 0; p < 28; p++) {
			snprintf(port_led_name, sizeof(port_led_name), "led_sw_p_ctrl.%02d", p);
			debugfs_create_x32(port_led_name, 0644, led_dir,
				(u32 *)(RTL838X_SW_BASE + RTL8390_LED_SW_P_CTRL(p)));
		}
	}
	return 0;
}

void rtl838x_dbgfs_init(struct rtl838x_switch_priv *priv)
{
	struct dentry *rtl838x_dir;
	struct dentry *port_dir;
	struct dentry *mirror_dir;
	struct debugfs_regset32 *port_ctrl_regset;
	int ret, i;
	char lag_name[10];
	char mirror_name[10];

	pr_info("%s called\n", __func__);
	rtl838x_dir = debugfs_lookup(RTL838X_DRIVER_NAME, NULL);
	if (!rtl838x_dir)
		rtl838x_dir = debugfs_create_dir(RTL838X_DRIVER_NAME, NULL);

	priv->dbgfs_dir = rtl838x_dir;

	debugfs_create_u32("soc", 0444, rtl838x_dir,
			   (u32 *)(RTL838X_SW_BASE + RTL838X_MODEL_NAME_INFO));

	/* Create one directory per port */
	for (i = 0; i < priv->cpu_port; i++) {
		if (priv->ports[i].phy) {
			ret = rtl838x_dbgfs_port_init(rtl838x_dir, priv, i);
			if (ret)
				goto err;
		}
	}

	/* Create directory for CPU-port */
	port_dir = debugfs_create_dir("cpu_port", rtl838x_dir);
	port_ctrl_regset = devm_kzalloc(priv->dev, sizeof(*port_ctrl_regset), GFP_KERNEL);
	if (!port_ctrl_regset) {
		ret = -ENOMEM;
		goto err;
	}

	port_ctrl_regset->regs = port_ctrl_regs;
	port_ctrl_regset->nregs = ARRAY_SIZE(port_ctrl_regs);
	port_ctrl_regset->base = (void *)(RTL838X_SW_BASE + (priv->cpu_port << 2));
	debugfs_create_regset32("port_ctrl", 0400, port_dir, port_ctrl_regset);
	debugfs_create_u8("id", 0444, port_dir, &priv->cpu_port);

	/* Create entries for LAGs */
	for (i = 0; i < priv->n_lags; i++) {
		snprintf(lag_name, sizeof(lag_name), "lag.%02d", i);
		if (priv->family_id == RTL8380_FAMILY_ID)
			debugfs_create_x32(lag_name, 0644, rtl838x_dir,
				(u32 *)(RTL838X_SW_BASE + priv->r->trk_mbr_ctr(i)));
		else
			debugfs_create_x64(lag_name, 0644, rtl838x_dir,
				(u64 *)(RTL838X_SW_BASE + priv->r->trk_mbr_ctr(i)));
	}

	/* Create directories for mirror groups */
	for (i = 0; i < 4; i++) {
		snprintf(mirror_name, sizeof(mirror_name), "mirror.%1d", i);
		mirror_dir = debugfs_create_dir(mirror_name, rtl838x_dir);
		if (priv->family_id == RTL8380_FAMILY_ID) {
			debugfs_create_x32("ctrl", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_MIR_CTRL + i * 4));
			debugfs_create_x32("ingress_pm", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + priv->r->mir_spm + i * 4));
			debugfs_create_x32("egress_pm", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + priv->r->mir_dpm + i * 4));
			debugfs_create_x32("qid", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_MIR_QID_CTRL(i)));
			debugfs_create_x32("rspan_vlan", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_MIR_RSPAN_VLAN_CTRL(i)));
			debugfs_create_x32("rspan_vlan_mac", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_MIR_RSPAN_VLAN_CTRL_MAC(i)));
			debugfs_create_x32("rspan_tx", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_MIR_RSPAN_TX_CTRL));
			debugfs_create_x32("rspan_tx_tag_rm", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_MIR_RSPAN_TX_TAG_RM_CTRL));
			debugfs_create_x32("rspan_tx_tag_en", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_MIR_RSPAN_TX_TAG_EN_CTRL));
		} else {
			debugfs_create_x32("ctrl", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_MIR_CTRL + i * 4));
			debugfs_create_x64("ingress_pm", 0644, mirror_dir,
				(u64 *)(RTL838X_SW_BASE + priv->r->mir_spm + i * 8));
			debugfs_create_x64("egress_pm", 0644, mirror_dir,
				(u64 *)(RTL838X_SW_BASE + priv->r->mir_dpm + i * 8));
			debugfs_create_x32("rspan_vlan", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_MIR_RSPAN_VLAN_CTRL(i)));
			debugfs_create_x32("rspan_tx", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_MIR_RSPAN_TX_CTRL));
			debugfs_create_x32("rspan_tx_tag_rm", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_MIR_RSPAN_TX_TAG_RM_CTRL));
			debugfs_create_x32("rspan_tx_tag_en", 0644, mirror_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_MIR_RSPAN_TX_TAG_EN_CTRL));
			debugfs_create_x64("sample_rate", 0644, mirror_dir,
				(u64 *)(RTL838X_SW_BASE + RTL839X_MIR_SAMPLE_RATE_CTRL));
		}
	}

	if (priv->family_id == RTL8380_FAMILY_ID)
		debugfs_create_x32("bpdu_flood_mask", 0644, rtl838x_dir,
				(u32 *)(RTL838X_SW_BASE + priv->r->rma_bpdu_fld_pmask));
	else
		debugfs_create_x64("bpdu_flood_mask", 0644, rtl838x_dir,
				(u64 *)(RTL838X_SW_BASE + priv->r->rma_bpdu_fld_pmask));

	if (priv->family_id == RTL8380_FAMILY_ID)
		debugfs_create_x32("vlan_ctrl", 0644, rtl838x_dir,
				(u32 *)(RTL838X_SW_BASE + RTL838X_VLAN_CTRL));
	else
		debugfs_create_x32("vlan_ctrl", 0644, rtl838x_dir,
				(u32 *)(RTL838X_SW_BASE + RTL839X_VLAN_CTRL));

	ret = rtl838x_dbgfs_leds(rtl838x_dir, priv);
	if (ret)
		goto err;

	return;
err:
	rtl838x_dbgfs_cleanup(priv);
}
