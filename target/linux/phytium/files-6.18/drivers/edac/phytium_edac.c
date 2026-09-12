// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium Pe220x EDAC (error detection and correction)
 *
 * Copyright (c) 2019-2023, Phytium Technology Co., Ltd.
 */

#include <linux/ctype.h>
#include <linux/edac.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <ras/ras_event.h>
#include "edac_module.h"
#include <linux/of_address.h>
#include <linux/acpi.h>

#define EDAC_MOD_STR			"phytium_edac"

/* RAS register offset */
#define ERR_STATUS(n)			(0x10 + ((n) * 64))
#define ERR_CTLR(n)			(0x08 + ((n) * 64))
#define ERR_MISC0(n)			(0x20 + ((n) * 64))
#define ERR_INJECT			0x7C
#define ERR_DEVID			0xFC8
#define ERR_GSR				0xE00

#define CTLR_ED				BIT(0)
#define CTLR_UI				BIT(2)
#define CTLR_CFI			BIT(8)

#define MISC0_CEC(x)			((u64)(x) << 32)

#define ERR_STATUS_CLEAR		GENMASK(31, 0)

#define CORRECTED_ERROR			0
#define UNCORRECTED_ERROR		1

#define PE220X_IGNORE_ERR_ID_MIN	40
#define PE220X_IGNORE_ERR_ID_MAX	43

#define PE220X_ECC_ERR_ID_MIN		23
#define PE220X_ECC_ERR_ID_MAX		24

#define PD2208_ECC_ERR_ID_MIN		0
#define PD2208_ECC_ERR_ID_MAX		3

/* MC version 1 register */
#define ECCSTAT		0x78
#define ECCCLR		0x7C
#define ECCERRCNT	0x80
#define ECCCADDR0	0x84
#define ECCCADDR1	0x88
#define ECCUADDR0	0xA4
#define ECCUADDR1	0xA8

#define ECC_RANK_MASK		GENMASK(25, 24)
#define ECC_ROW_MASK		GENMASK(17, 0)
#define ECC_CID_MASK		GENMASK(29, 28)
#define ECC_BG_MASK		GENMASK(25, 24)
#define ECC_BANK_MASK		GENMASK(18, 16)
#define ECC_COL_MASK		GENMASK(11, 0)
#define ECC_CORR_ERR_CNT_MASK	GENMASK(15, 0)
#define ECC_UNCORR_ERR_CNT_MASK	GENMASK(31, 16)

#define ECC_CLR_UNCORR_ERR	(BIT_MASK(3) | BIT_MASK(1))
#define ECC_CLR_CORR_ERR	(BIT_MASK(2) | BIT_MASK(0))

/* MC version 2 register */
#define DDRC_PADDR	0x80
#define DDRC_PDATA	0x84
#define ECC_U_ADDR_L	0x7B8
#define ECC_U_ADDR_H	0x7BC
#define ECC_U_DATA_L	0x7C0
#define ECC_U_DATA_H	0x7C4
#define ECC_C_ADDR_L	0x7C8
#define ECC_C_ADDR_H	0x7CC
#define ECC_C_DATA_L	0x7D0
#define ECC_C_DATA_H	0x7D4
#define INT_STATUS_ECC	0x944
#define INT_ACK_ECC	0x964

#define MEM_ADDR_MASK	GENMASK(9, 0)
#define INT_STATUS_MASK	GENMASK(8, 0)
#define ECC_C_ERR_MASK	GENMASK(3, 2)
#define ECC_U_ERR_MASK	GENMASK(1, 0)

#define MC_REG_VERSION_1	1
#define MC_REG_VERSION_2	2

#define EDAC_DRIVER_VERSION "1.1.5"

struct ras_error_info {
	u32 index;
	u32 error_type;
	const char *error_str;
};

struct phytium_edac_mc_ctx {
	struct list_head next;
	char *name;
	struct mem_ctl_info *mci;
	struct phytium_edac *edac;
	void __iomem *reg_base;
	u32 channel_id;
};

struct phytium_edac {
	struct device *dev;
	void __iomem **ras_base;
	struct dentry *dfs;
	struct edac_device_ctl_info *edac_dev;
	int num_err_group;
	const struct ras_error_info **error_info;
	struct list_head mc_list;
	int mc_reg_version;
};

/* error severity definition */
enum {
	SEV_NO = 0x0,
	SEV_CORRECTED = 0x1,
	SEV_RECOVERABLE = 0x2,
	SEV_PANIC = 0x3,
};

/* soc error record */
static const struct ras_error_info pe220x_ras_soc_error[] = {
	{ 0, UNCORRECTED_ERROR, "lsd_nfc_ras_error" },
	{ 1, UNCORRECTED_ERROR, "lsd_lpc_ras_long_wait_to" },
	{ 2, UNCORRECTED_ERROR, "lsd_lpc_ras_short_wait_to" },
	{ 3, UNCORRECTED_ERROR, "lsd_lpc_ras_sync_err" },
	{ 4, UNCORRECTED_ERROR, "lsd_lbc_ras_err" },
	{ 5, UNCORRECTED_ERROR, "usb3_err_0" },
	{ 6, UNCORRECTED_ERROR, "usb3_err_1" },
	{ 7, UNCORRECTED_ERROR, "gsd_gmu_mac0_asf_nonfatal_int" },
	{ 8, UNCORRECTED_ERROR, "gsd_gmu_mac0_asf_fatal_int" },
	{ 9, UNCORRECTED_ERROR, "gsd_gmu_mac0_asf_trans_to_err" },
	{ 10, UNCORRECTED_ERROR, "gsd_gmu_mac0_asf_protocol_err" },
	{ 11, UNCORRECTED_ERROR, "gsd_gmu_mac1_asf_nonfatal_int" },
	{ 12, UNCORRECTED_ERROR, "gsd_gmu_mac1_asf_fatal_int" },
	{ 13, UNCORRECTED_ERROR, "gsd_gmu_mac1_asf_trans_to_err" },
	{ 14, UNCORRECTED_ERROR, "gsd_gmu_mac1_asf_protocol_err" },
	{ 15, UNCORRECTED_ERROR, "gsd_gmu_mac2_asf_nonfatal_int" },
	{ 16, UNCORRECTED_ERROR, "gsd_gmu_mac2_asf_fatal_int" },
	{ 17, UNCORRECTED_ERROR, "gsd_gmu_mac2_asf_trans_to_err" },
	{ 18, UNCORRECTED_ERROR, "gsd_gmu_mac2_asf_protocol_err" },
	{ 19, UNCORRECTED_ERROR, "gsd_gmu_mac3_asf_nonfatal_int" },
	{ 20, UNCORRECTED_ERROR, "gsd_gmu_mac3_asf_fatal_int" },
	{ 21, UNCORRECTED_ERROR, "gsd_gmu_mac3_asf_trans_to_err" },
	{ 22, UNCORRECTED_ERROR, "gsd_gmu_mac3_asf_protocol_err" },
	{ 23, CORRECTED_ERROR, "dmu_ras_ecc_corrected_error" },
	{ 24, UNCORRECTED_ERROR, "dmu_ras_ecc_uncorrected_error" },
	{ 25, UNCORRECTED_ERROR, "cci_ras_nERRIRQ" },
	{ 26, UNCORRECTED_ERROR, "smmu_tcu_ras_irpt" },
	{ 27, UNCORRECTED_ERROR, "smmu_tbu0_ras_irpt" },
	{ 28, UNCORRECTED_ERROR, "smmu_tbu1_ras_irpt" },
	{ 29, UNCORRECTED_ERROR, "smmu_tbu2_ras_irpt" },
	{ 30, UNCORRECTED_ERROR, "ocm_sram_ue" },
	{ 31, CORRECTED_ERROR, "ocm_sram_ce" },
	{ 32, UNCORRECTED_ERROR, "int_axim_err" },
	{ 33, UNCORRECTED_ERROR, "int_fatal_error" },
	{ 34, UNCORRECTED_ERROR, "nEXTERRIRQ_clust0" },
	{ 35, UNCORRECTED_ERROR, "nINTERRIRQ_clust0" },
	{ 36, UNCORRECTED_ERROR, "nEXTERRIRQ_clust1" },
	{ 37, UNCORRECTED_ERROR, "nINTERRIRQ_clust1" },
	{ 38, UNCORRECTED_ERROR, "nEXTERRIRQ_clust2" },
	{ 39, UNCORRECTED_ERROR, "nINTERRIRQ_clust2" },
	{ 40, UNCORRECTED_ERROR, "ras_err_amu0" },
	{ 41, UNCORRECTED_ERROR, "ras_err_amu1" },
	{ 42, UNCORRECTED_ERROR, "ras_err_ame0" },
	{ 43, UNCORRECTED_ERROR, "ras_err_ame1" },
};

/* pcie controller error record */
static const struct ras_error_info pe220x_ras_peu_psu_error[] = {
	{ 0, UNCORRECTED_ERROR, "pio_rd_addr_error" },
	{ 1, UNCORRECTED_ERROR, "pio_wr_addr_error" },
	{ 2, UNCORRECTED_ERROR, "pio_rd_timeout" },
	{ 3, UNCORRECTED_ERROR, "pio_wr_timeout" },
	{ 4, UNCORRECTED_ERROR, "axi_b_rsp_error" },
	{ 5, UNCORRECTED_ERROR, "axi_r_rsp_error" },
};

static const struct ras_error_info pe220x_ras_peu_error[] = {
	{ 0, UNCORRECTED_ERROR, "pio_rd_addr_error" },
	{ 1, UNCORRECTED_ERROR, "pio_wr_addr_error" },
	{ 2, UNCORRECTED_ERROR, "pio_rd_timeout" },
	{ 3, UNCORRECTED_ERROR, "pio_wr_timeout" },
	{ 4, UNCORRECTED_ERROR, "axi_b_rsp_error" },
	{ 5, UNCORRECTED_ERROR, "axi_r_rsp_error" },
};

/* pd2208 error */
static const struct ras_error_info pd2208_ras_err[] = {
	{0, CORRECTED_ERROR, "lmu0_ras_ecc_corrected_err"},
	{1, UNCORRECTED_ERROR, "lmu0_ras_ecc_uncorrected_err"},
	{2, CORRECTED_ERROR, "lmu1_ras_ecc_corrected_err"},
	{3, UNCORRECTED_ERROR, "lmu1_ras_ecc_uncorrected_err"},
	{4, CORRECTED_ERROR, "sram_corrected_err"},
	{5, UNCORRECTED_ERROR, "sram_uncorrected_err"},
	{6, UNCORRECTED_ERROR, "qspi_ras_addr_err"},
	{7, UNCORRECTED_ERROR, "qspi_ras_pstrb_err"},
	{8, UNCORRECTED_ERROR, "intreq_err"},
	{9, UNCORRECTED_ERROR, "gic_axim_err"},
	{10, UNCORRECTED_ERROR, "gic_ecc_fatal"},
	{11, UNCORRECTED_ERROR, "lsd_lbc_ras_err"},
	{12, UNCORRECTED_ERROR, "nEXTERRIRQ_cluster0"},
	{13, UNCORRECTED_ERROR, "nINTERRIRQ_cluster0"},
	{14, UNCORRECTED_ERROR, "nEXTERRIRQ_cluster1"},
	{15, UNCORRECTED_ERROR, "nINTERRIRQ_cluster1"},
	{16, UNCORRECTED_ERROR, "nEXTERRIRQ_cluster2"},
	{17, UNCORRECTED_ERROR, "nINTERRIRQ_cluster2"},
	{18, UNCORRECTED_ERROR, "nEXTERRIRQ_cluster3"},
	{19, UNCORRECTED_ERROR, "nINTERRIRQ_cluster3"},
	{20, CORRECTED_ERROR, "lbc_ecc_corrected_err"},
	{21, UNCORRECTED_ERROR, "lbc_ecc_uncorrected_err"},
};

static const struct ras_error_info pd2208_ras_sram_err[] = {
	{0, CORRECTED_ERROR, "scp_sram_corrected_err"},
	{1, UNCORRECTED_ERROR, "scp_sram_uncorrected_err"},
	{2, CORRECTED_ERROR, "scp_sharemem_corrected_err"},
	{3, UNCORRECTED_ERROR, "scp_sharemem_uncorrected_err"},
	{4, CORRECTED_ERROR, "dmu0_wr_cmd_buf_corrected_err"},
	{5, UNCORRECTED_ERROR, "dmu0_wr_cmd_buf_uncorrected_err"},
	{6, CORRECTED_ERROR, "dmu0_rd_dat_buf_corrected_err"},
	{7, UNCORRECTED_ERROR, "dmu0_rd_dat_buf_uncorrected_err"},
	{8, CORRECTED_ERROR, "dmu1_wr_cmd_buf_corrected_err"},
	{9, UNCORRECTED_ERROR, "dmu1_wr_cmd_buf_uncorrected_err"},
	{10, CORRECTED_ERROR, "dmu1_rd_dat_buf_corrected_err"},
	{11, UNCORRECTED_ERROR, "dmu1_rd_dat_buf_uncorrected_err"},
	{12, CORRECTED_ERROR, "drtrch0_corrected_err"},
	{13, UNCORRECTED_ERROR, "drtrch0_uncorrected_err"},
	{14, CORRECTED_ERROR, "drtrch1_corrected_err"},
	{15, UNCORRECTED_ERROR, "drtrch1_uncorrected_err"},
	{16, CORRECTED_ERROR, "dmac_corrected_err"},
	{17, UNCORRECTED_ERROR, "dmac_uncorrected_err"},
	{18, CORRECTED_ERROR, "rmram0_corrected_err"},
	{19, UNCORRECTED_ERROR, "rmram0_uncorrected_err"},
	{20, CORRECTED_ERROR, "rmram1_corrected_err"},
	{21, UNCORRECTED_ERROR, "rmram1_uncorrected_err"},
	{22, CORRECTED_ERROR, "rmram2_corrected_err"},
	{23, UNCORRECTED_ERROR, "rmram2_uncorrected_err"},
	{24, CORRECTED_ERROR, "rmram3_corrected_err"},
	{25, UNCORRECTED_ERROR, "rmram3_uncorrected_err"},
	{26, CORRECTED_ERROR, "gmactx0_corrected_err"},
	{27, UNCORRECTED_ERROR, "gmactx0_uncorrected_err"},
	{28, CORRECTED_ERROR, "gmactx1_corrected_err"},
	{29, UNCORRECTED_ERROR, "gmactx1_uncorrected_err"},
	{30, CORRECTED_ERROR, "gmactx2_corrected_err"},
	{31, UNCORRECTED_ERROR, "gmactx2_uncorrected_err"},
	{32, CORRECTED_ERROR, "gmactx3_corrected_err"},
	{33, UNCORRECTED_ERROR, "gmactx3_uncorrected_err"},
};

static const struct ras_error_info pd2208_ras_peu_sram0_err[] = {
	{0, CORRECTED_ERROR, "c0p2a_corrected_err"},
	{1, UNCORRECTED_ERROR, "c0p2a_uncorrected_err"},
	{2, CORRECTED_ERROR, "c0a2p_corrected_err"},
	{3, UNCORRECTED_ERROR, "c0a2p_uncorrected_err"},
	{4, CORRECTED_ERROR, "c0rxbuf0_corrected_err"},
	{5, UNCORRECTED_ERROR, "c0rxbuf0_uncorrected_err"},
	{6, CORRECTED_ERROR, "c0rxbuf1_corrected_err"},
	{7, UNCORRECTED_ERROR, "c0rxbuf1_uncorrected_err"},
	{8, CORRECTED_ERROR, "c0rxbuf2_corrected_err"},
	{9, UNCORRECTED_ERROR, "c0rxbuf2_uncorrected_err"},
	{10, CORRECTED_ERROR, "c0rxbuf3_corrected_err"},
	{11, UNCORRECTED_ERROR, "c0rxbuf3_uncorrected_err"},
	{12, CORRECTED_ERROR, "c0txbuf0_corrected_err"},
	{13, UNCORRECTED_ERROR, "c0txbuf0_uncorrected_err"},
	{14, CORRECTED_ERROR, "c0txbuf1_corrected_err"},
	{15, UNCORRECTED_ERROR, "c0txbuf1_uncorrected_err"},
	{16, CORRECTED_ERROR, "c0txbuf2_corrected_err"},
	{17, UNCORRECTED_ERROR, "c0txbuf2_uncorrected_err"},
	{18, CORRECTED_ERROR, "c0txbuf3_corrected_err"},
	{19, UNCORRECTED_ERROR, "c0txbuf3_uncorrected_err"},
	{20, CORRECTED_ERROR, "c1p2a_corrected_err"},
	{21, UNCORRECTED_ERROR, "c1p2a_uncorrected_err"},
	{22, CORRECTED_ERROR, "c1a2p_corrected_err"},
	{23, UNCORRECTED_ERROR, "c1a2p_uncorrected_err"},
	{24, CORRECTED_ERROR, "c1rxbuf0_corrected_err"},
	{25, UNCORRECTED_ERROR, "c1rxbuf0_uncorrected_err"},
	{26, CORRECTED_ERROR, "c1rxbuf1_corrected_err"},
	{27, UNCORRECTED_ERROR, "c1rxbuf1_uncorrected_err"},
	{28, CORRECTED_ERROR, "c1rxbuf2_corrected_err"},
	{29, UNCORRECTED_ERROR, "c1rxbuf2_uncorrected_err"},
	{30, CORRECTED_ERROR, "c1rxbuf3_corrected_err"},
	{31, UNCORRECTED_ERROR, "c1rxbuf3_uncorrected_err"},
	{32, CORRECTED_ERROR, "c1txbuf0_corrected_err"},
	{33, UNCORRECTED_ERROR, "c1txbuf0_uncorrected_err"},
	{34, CORRECTED_ERROR, "c1txbuf1_corrected_err"},
	{35, UNCORRECTED_ERROR, "c1txbuf1_uncorrected_err"},
	{36, CORRECTED_ERROR, "c1txbuf2_corrected_err"},
	{37, UNCORRECTED_ERROR, "c1txbuf2_uncorrected_err"},
	{38, CORRECTED_ERROR, "c1txbuf3_corrected_err"},
	{39, UNCORRECTED_ERROR, "c1txbuf3_uncorrected_err"},
	{40, CORRECTED_ERROR, "c2p2a_corrected_err"},
	{41, UNCORRECTED_ERROR, "c2p2a_uncorrected_err"},
	{42, CORRECTED_ERROR, "c2a2p_corrected_err"},
	{43, UNCORRECTED_ERROR, "c2a2p_uncorrected_err"},
	{44, CORRECTED_ERROR, "c2rxbuf0_corrected_err"},
	{45, UNCORRECTED_ERROR, "c2rxbuf0_uncorrected_err"},
	{46, CORRECTED_ERROR, "c2rxbuf1_corrected_err"},
	{47, UNCORRECTED_ERROR, "c2rxbuf1_uncorrected_err"},
	{48, CORRECTED_ERROR, "c2rxbuf2_corrected_err"},
	{49, UNCORRECTED_ERROR, "c2rxbuf2_uncorrected_err"},
	{50, CORRECTED_ERROR, "c2rxbuf3_corrected_err"},
	{51, UNCORRECTED_ERROR, "c2rxbuf3_uncorrected_err"},
	{52, CORRECTED_ERROR, "c2txbuf0_corrected_err"},
	{53, UNCORRECTED_ERROR, "c2txbuf0_uncorrected_err"},
	{54, CORRECTED_ERROR, "c2txbuf1_corrected_err"},
	{55, UNCORRECTED_ERROR, "c2txbuf1_uncorrected_err"},
};

static const struct ras_error_info pd2208_ras_peu_sram1_err[] = {
	{0, CORRECTED_ERROR, "c2txbuf2_corrected_err"},
	{1, UNCORRECTED_ERROR, "c2txbuf2_uncorrected_err"},
	{2, CORRECTED_ERROR, "c2txbuf3_corrected_err"},
	{3, UNCORRECTED_ERROR, "c2txbuf3_uncorrected_err"},
	{4, CORRECTED_ERROR, "phy0_sram0_corrected_err"},
	{5, UNCORRECTED_ERROR, "phy0_sram0_uncorrected_err"},
	{6, CORRECTED_ERROR, "phy0_sram1_corrected_err"},
	{7, UNCORRECTED_ERROR, "phy0_sram1_uncorrected_err"},
	{8, CORRECTED_ERROR, "phy0_sram2_corrected_err"},
	{9, UNCORRECTED_ERROR, "phy0_sram2_uncorrected_err"},
	{10, CORRECTED_ERROR, "phy0_sram3_corrected_err"},
	{11, UNCORRECTED_ERROR, "phy0_sram3_uncorrected_err"},
	{12, CORRECTED_ERROR, "phy1_sram0_corrected_err"},
	{13, UNCORRECTED_ERROR, "phy1_sram0_uncorrected_err"},
	{14, CORRECTED_ERROR, "mac0_rxdpram_corrected_err"},
	{15, UNCORRECTED_ERROR, "mac0_rxdpram_uncorrected_err"},
	{16, CORRECTED_ERROR, "mac0_txdpram_corrected_err"},
	{17, UNCORRECTED_ERROR, "mac0_txdpram_uncorrected_err"},
	{18, CORRECTED_ERROR, "mac1_rxdpram_corrected_err"},
	{19, UNCORRECTED_ERROR, "mac1_rxdpram_uncorrected_err"},
	{20, CORRECTED_ERROR, "mac1_txdpram_corrected_err"},
	{21, UNCORRECTED_ERROR, "mac1_txdpram_uncorrected_err"},
};

static const struct ras_error_info pd2208_ras_peu_base_err[] = {
	{0, UNCORRECTED_ERROR, "pio_rd_addr_error"},
	{1, UNCORRECTED_ERROR, "pio_rd_timeout"},
	{2, UNCORRECTED_ERROR, "pio_wr_addr_error"},
	{3, UNCORRECTED_ERROR, "pio_wr_timeout"},
	{4, CORRECTED_ERROR, "axi_b_rsp_error"},
	{5, UNCORRECTED_ERROR, "axi_r_rsp_error"},
	{6, UNCORRECTED_ERROR, "mac0_asf_trans_to_err"},
	{7, UNCORRECTED_ERROR, "mac0_asf_protocol_err"},
	{8, UNCORRECTED_ERROR, "mac0_asf_nonfatal_int"},
	{9, UNCORRECTED_ERROR, "mac0_asf_fatal_int"},
	{10, UNCORRECTED_ERROR, "mac1_asf_trans_to_err"},
	{11, UNCORRECTED_ERROR, "mac1_asf_protocol_err"},
	{12, UNCORRECTED_ERROR, "mac1_asf_nonfatal_int"},
	{13, UNCORRECTED_ERROR, "mac1_asf_fatal_int"},
};

static const struct ras_error_info *pe220x_ras_error[] = {
	pe220x_ras_soc_error,
	pe220x_ras_peu_psu_error,
	pe220x_ras_peu_error,
};

static const struct ras_error_info *pd2208_ras_error[] = {
	pd2208_ras_err,
	pd2208_ras_sram_err,
	pd2208_ras_peu_sram0_err,
	pd2208_ras_peu_sram1_err,
	pd2208_ras_peu_base_err,
};

static inline unsigned int get_error_num(const struct phytium_edac *edac,
								int err_group)
{
	unsigned int error_num = 0;

	error_num = readl(edac->ras_base[err_group] + ERR_DEVID);

	return error_num;
}

static inline void phytium_ras_setup(const struct phytium_edac *edac)
{
	u64 val = 0;
	unsigned int i = 0;

	/*
	 * enable error report and generate interrupt for corrected error event
	 */
	for (i = 0; i < edac->num_err_group; i++) {
		val = readq(edac->ras_base[i] + ERR_CTLR(0));
		val |= CTLR_ED | CTLR_UI | CTLR_CFI;
		writeq(val, edac->ras_base[i] + ERR_CTLR(0));
	}
}

static ssize_t phytium_edac_inject_ctrl_write(struct file *filp,
						   const char __user *buf,
						   size_t size, loff_t *ppos)
{
	int ret = 0;
	int res = 0;
	unsigned int error_group = 0;
	unsigned int error_id = 0;
	unsigned int error_num = 0;
	struct phytium_edac *edac = filp->private_data;
	char str[256];
	char *p_str = str;
	char *tmp = NULL;

	if (size > 255) {
		ret = -EFAULT;
		goto out;
	}

	if (copy_from_user(str, buf, size)) {
		ret = -EFAULT;
		goto out;
	} else {
		*ppos += size;
		ret = size;
	}
	str[size] = '\0';

	tmp = strsep(&p_str, ",");
	if (!tmp)
		goto out;

	res = kstrtouint(tmp, 0, &error_group);
	if (res || error_group >= edac->num_err_group) {
		dev_err(edac->dev, "invalid error group parameters");
		goto out;
	}

	res = kstrtouint(p_str, 0, &error_id);
	if (res) {
		dev_err(edac->dev, "invalid error id parameters");
		goto out;
	}

	error_num = get_error_num(edac, error_group);
	if (error_id >= error_num) {
		dev_err(edac->dev, "invalid ras error id.\n");
		goto out;
	}

	dev_dbg(edac->dev, "inject group: %d, error_id: %d\n",
			error_group, error_id);

	if (edac->error_info[error_group][error_id].error_type
			== CORRECTED_ERROR) {
		writeq(MISC0_CEC(0xFF),
			edac->ras_base[error_group] + ERR_MISC0(error_id));
	}

	writel(error_id, edac->ras_base[error_group] + ERR_INJECT);

out:
	return ret;
}

static const struct file_operations phytium_edac_debug_inject_fops[] = {
	{
	.open = simple_open,
	.write = phytium_edac_inject_ctrl_write,
	.llseek = generic_file_llseek, },
	{ }
};

static void phytium_edac_create_debugfs_nodes(struct phytium_edac *edac)
{
	if (!IS_ENABLED(CONFIG_EDAC_DEBUG) || !edac->dfs) {
		dev_info(edac->dev, "edac debug is disable");
		return;
	}

	if (!edac_debugfs_create_file("error_inject_ctrl", 0x0200, edac->dfs, edac,
				      &phytium_edac_debug_inject_fops[0]))
		debugfs_remove_recursive(edac->dfs);

}

static int phytium_edac_device_add(struct phytium_edac *edac)
{
	struct edac_device_ctl_info *edac_dev;
	int res = 0;

	edac_dev = edac_device_alloc_ctl_info(
					sizeof(struct edac_device_ctl_info),
					      "ras", 1, "soc", 1, 0,
					      edac_device_alloc_index());
	if (!edac_dev) {
		res = -ENOMEM;
		goto out;
	}

	edac_dev->dev = edac->dev;
	edac_dev->mod_name = EDAC_MOD_STR;
	edac_dev->ctl_name = "phytium ras";
	edac_dev->dev_name = "phytium_edac_soc";

	phytium_edac_create_debugfs_nodes(edac);

	res = edac_device_add_device(edac_dev);
	if (res > 0) {
		dev_err(edac->dev, "edac_device_add_device failed\n");
		goto err_free;
	}

	edac->edac_dev = edac_dev;
	dev_info(edac->dev, "phytium edac device registered\n");
	return 0;

err_free:
	debugfs_remove_recursive(edac->dfs);
	edac_device_free_ctl_info(edac_dev);

out:
	return res;
}

static int phytium_edac_device_remove(struct phytium_edac *edac)
{
	struct edac_device_ctl_info *edac_dev = edac->edac_dev;

	debugfs_remove_recursive(edac->dfs);
	edac_device_del_device(edac_dev->dev);
	edac_device_free_ctl_info(edac_dev);
	return 0;
}

static int get_error_id(struct phytium_edac *edac, int *error_id,
						int *error_group)
{
	unsigned int error_num = 0;
	u64 error_bit = 0;
	int ret = 0;
	int i = 0;
	int err_id = 0;

	/* Iterate over the ras node to check error status */
	for (i = 0; i < edac->num_err_group; i++) {
		error_num = get_error_num(edac, i);
		error_bit = readq(edac->ras_base[i] + ERR_GSR);
		for (err_id = 0; err_id < error_num; err_id++) {
			if (!(error_bit & BIT(err_id)))
				continue;
			else
				break;
		}
		if (err_id < error_num) {
			*error_id = err_id;
			*error_group = i;
			break;
		}
	}

	if (i >= edac->num_err_group) {
		ret = -1;
		dev_warn(edac->dev, "no error detect.\n");
	}

	return ret;
}

static void phytium_edac_mc_check_ver1(struct mem_ctl_info *mci)
{
	u32 corr_err_cnt = 0;
	u32 uncorr_err_cnt = 0;
	u32 value = 0;
	u32 rank = 0;
	u32 bank = 0;
	u32 row = 0;
	u32 column = 0;
	u32 cid = 0;
	u32 bg = 0;
	struct phytium_edac_mc_ctx *ctx = mci->pvt_info;

	writel(ECCERRCNT, ctx->reg_base + DDRC_PADDR);
	value = readl(ctx->reg_base + DDRC_PDATA);
	corr_err_cnt = value & ECC_CORR_ERR_CNT_MASK;
	uncorr_err_cnt = (value & ECC_UNCORR_ERR_CNT_MASK) >> 16;

	if (corr_err_cnt > 0) {
		writel(ECCCADDR0, ctx->reg_base + DDRC_PADDR);
		value = readl(ctx->reg_base + DDRC_PDATA);
		rank = (value & ECC_RANK_MASK) >> 24;
		row = value & ECC_ROW_MASK;
		writel(ECCCADDR1, ctx->reg_base + DDRC_PADDR);
		value = readl(ctx->reg_base + DDRC_PDATA);
		cid = (value & ECC_CID_MASK) >> 28;
		bg = (value & ECC_BG_MASK) >> 24;
		bank = (value & ECC_BANK_MASK) >> 16;
		column = value & ECC_COL_MASK;
		edac_mc_chipset_printk(mci, KERN_WARNING, "Phytium",
				"ECC correctable error at rank %d bank %d column %d row %d cid %d bg %d count %d\n",
				rank, bank, column, row, bg, cid, corr_err_cnt);
		edac_mc_handle_error(HW_EVENT_ERR_CORRECTED, mci,
				1, 0, 0, 0, 0, 0, -1, mci->ctl_name, "");

		/* clear correct error */
		writel(ECCCLR, ctx->reg_base + DDRC_PADDR);
		writel(ECC_CLR_CORR_ERR, ctx->reg_base + DDRC_PDATA);
	}

	if (uncorr_err_cnt > 0) {
		writel(ECCUADDR0, ctx->reg_base + DDRC_PADDR);
		value = readl(ctx->reg_base + DDRC_PDATA);
		rank = (value & ECC_RANK_MASK) >> 24;
		row = value & ECC_ROW_MASK;
		writel(ECCUADDR1, ctx->reg_base + DDRC_PADDR);
		value = readl(ctx->reg_base + DDRC_PDATA);
		cid = (value & ECC_CID_MASK) >> 28;
		bg = (value & ECC_BG_MASK) >> 24;
		bank = (value & ECC_BANK_MASK) >> 16;
		column = value & ECC_COL_MASK;
		edac_mc_chipset_printk(mci, KERN_ERR, "Phytium",
				"ECC uncorrectable error at rank %d bank %d column %d row %d cid %d bg %d count %d\n",
				rank, bank, column, row, bg, cid, uncorr_err_cnt);
		edac_mc_handle_error(HW_EVENT_ERR_UNCORRECTED, mci,
				1, 0, 0, 0, 0, 0, -1, mci->ctl_name, "");
		/* clear uncorrect error */
		writel(ECCCLR, ctx->reg_base + DDRC_PADDR);
		writel(ECC_CLR_UNCORR_ERR, ctx->reg_base + DDRC_PDATA);
	}
}

static void phytium_edac_mc_check_ver2(struct mem_ctl_info *mci)
{
	u64 temp1 = 0;
	u64 temp2 = 0;
	u64 err_addr = 0;
	u64 err_data = 0;
	u32 ecc_status = 0;
	struct phytium_edac_mc_ctx *ctx = mci->pvt_info;

	writel(INT_STATUS_ECC, ctx->reg_base + DDRC_PADDR);
	ecc_status = readl(ctx->reg_base + DDRC_PDATA) & INT_STATUS_MASK;

	if (ecc_status & ECC_U_ERR_MASK) {
		writel(ECC_U_ADDR_L, ctx->reg_base + DDRC_PADDR);
		temp1 = readl(ctx->reg_base + DDRC_PDATA);
		writel(ECC_U_ADDR_H, ctx->reg_base + DDRC_PADDR);
		temp2 = readl(ctx->reg_base + DDRC_PDATA) & MEM_ADDR_MASK;
		err_addr = temp1 | (temp2 << 32);
		writel(ECC_U_DATA_L, ctx->reg_base + DDRC_PADDR);
		temp1 = readl(ctx->reg_base + DDRC_PDATA);
		writel(ECC_U_DATA_H, ctx->reg_base + DDRC_PADDR);
		temp2 = readl(ctx->reg_base + DDRC_PDATA);
		err_data = temp1 | (temp2 << 32);
		edac_mc_chipset_printk(mci, KERN_ERR, "Phytium",
			"ECC channel %d uncorrectable error at address  %#016llx\n",
			ctx->channel_id, err_addr);
		edac_mc_handle_error(HW_EVENT_ERR_UNCORRECTED, mci,
				1, 0, 0, 0, 0, 0, -1, mci->ctl_name, "");
	}

	if (ecc_status & ECC_C_ERR_MASK) {
		writel(ECC_C_ADDR_L, ctx->reg_base + DDRC_PADDR);
		temp1 = readl(ctx->reg_base + DDRC_PDATA);
		writel(ECC_C_ADDR_H, ctx->reg_base + DDRC_PADDR);
		temp2 = readl(ctx->reg_base + DDRC_PDATA) & MEM_ADDR_MASK;
		err_addr = temp1 | (temp2 << 32);
		writel(ECC_C_DATA_L, ctx->reg_base + DDRC_PADDR);
		temp1 = readl(ctx->reg_base + DDRC_PDATA);
		writel(ECC_C_DATA_H, ctx->reg_base + DDRC_PADDR);
		temp2 = readl(ctx->reg_base + DDRC_PDATA);
		err_data = temp1 | (temp2 << 32);
		edac_mc_chipset_printk(mci, KERN_ERR, "Phytium",
			"ECC channel %d correctable error at address  %#016llx\n",
			ctx->channel_id, err_addr);
		edac_mc_handle_error(HW_EVENT_ERR_CORRECTED, mci,
				1, 0, 0, 0, 0, 0, -1, mci->ctl_name, "");
	}

	/* clear ecc int status */
	if (ecc_status) {
		writel(INT_ACK_ECC, ctx->reg_base + DDRC_PADDR);
		writel(ECC_C_ERR_MASK | ECC_U_ERR_MASK,
			ctx->reg_base + DDRC_PDATA);
	}
}

static void phytium_edac_error_report(struct phytium_edac *edac,
				const int error_id, const int error_group)
{
	struct phytium_edac_mc_ctx *ctx = NULL;
	const struct ras_error_info *err_info =
		edac->error_info[error_group];

	/* ignore pe220x soc_err id 40~43 */
	if ((err_info == pe220x_ras_soc_error) &&
	    (error_id >= PE220X_IGNORE_ERR_ID_MIN) &&
	    (error_id <= PE220X_IGNORE_ERR_ID_MAX))
		return;

	/* ecc error report */
	if ((err_info == pe220x_ras_soc_error) &&
	     (error_id >= PE220X_ECC_ERR_ID_MIN) &&
	     (error_id <= PE220X_ECC_ERR_ID_MAX)) {
		list_for_each_entry(ctx, &edac->mc_list , next)
			phytium_edac_mc_check_ver2(ctx->mci);
	}

	if ((err_info == pd2208_ras_err) &&
	     (error_id >= PD2208_ECC_ERR_ID_MIN) &&
	     (error_id <= PD2208_ECC_ERR_ID_MAX)) {
		list_for_each_entry(ctx, &edac->mc_list , next)
			phytium_edac_mc_check_ver2(ctx->mci);
	}

	if (err_info[error_id].error_type == UNCORRECTED_ERROR) {
		edac_printk(KERN_ERR, EDAC_MOD_STR, "uncorrected error: %s\n",
			err_info[error_id].error_str);
		edac_device_handle_ue(edac->edac_dev, 0, 0,
				err_info[error_id].error_str);
		/* Report the error via the trace interface */
		if (IS_ENABLED(CONFIG_RAS))
			trace_non_standard_event(&NULL_GUID, &NULL_GUID,
					EDAC_MOD_STR, SEV_RECOVERABLE,
					err_info[error_id].error_str,
					strlen(err_info[error_id].error_str));
	} else {
		edac_printk(KERN_WARNING, EDAC_MOD_STR, "corrected error: %s\n",
			err_info[error_id].error_str);
		edac_device_handle_ce(edac->edac_dev, 0, 0,
				err_info[error_id].error_str);
		if (IS_ENABLED(CONFIG_RAS))
			trace_non_standard_event(&NULL_GUID, &NULL_GUID,
					EDAC_MOD_STR, SEV_CORRECTED,
					err_info[error_id].error_str,
					strlen(err_info[error_id].error_str));
	}
}

/*
 * clear error status and set correct error counter to 0xFE for trigger
 * interrupt when next correct error event
 */
static void phytium_edac_clear_error_status(struct phytium_edac *edac,
				const int error_id, const int error_group)
{
	writeq(MISC0_CEC(0XFE), edac->ras_base[error_group] +
			ERR_MISC0(error_id));
	writeq(GENMASK(31, 0), edac->ras_base[error_group] +
			ERR_STATUS(error_id));
}

static irqreturn_t phytium_edac_isr(int irq, void *dev_id)
{
	struct phytium_edac *edac = dev_id;
	int ret = 0;
	int error_group;
	int error_id;

	ret = get_error_id(edac, &error_id, &error_group);
	if (ret < 0)
		goto out;

	phytium_edac_error_report(edac, error_id, error_group);
	phytium_edac_clear_error_status(edac, error_id, error_group);

out:
	return IRQ_HANDLED;
}

static int phytium_edac_mc_add(struct phytium_edac *edac,
			       struct resource *res, int mc_id)
{
	struct mem_ctl_info *mci = NULL;
	struct edac_mc_layer layer;
	struct phytium_edac_mc_ctx ctx;
	struct phytium_edac_mc_ctx *p_ctx;
	int ret = 0;

	if (!devres_open_group(edac->dev, phytium_edac_mc_add, GFP_KERNEL)) {
		ret = -ENOMEM;
		goto out;
	}

	ctx.channel_id = mc_id;
	ctx.reg_base = devm_ioremap_resource(edac->dev, res);
	if (IS_ERR(ctx.reg_base)) {
		dev_err(edac->dev, "unable to map memory controller reg address\n");
		ret = PTR_ERR(ctx.reg_base);
		goto err_group;
	}

	layer.type = EDAC_MC_LAYER_CHANNEL;
	layer.size = 1;
	layer.is_virt_csrow = false;

	mci = edac_mc_alloc(ctx.channel_id, 1, &layer, sizeof(ctx));
	if (!mci) {
		ret = -ENOMEM;
		goto err_group;
	}

	p_ctx = mci->pvt_info;
	*p_ctx = ctx;
	p_ctx->name = "phytium_edac_mc";
	p_ctx->mci = mci;
	mci->pdev = &mci->dev;
	mci->ctl_name = p_ctx->name;
	mci->dev_name = p_ctx->name;

	mci->mtype_cap = MEM_FLAG_DDR4;
	mci->edac_ctl_cap = EDAC_FLAG_SECDED;
	mci->edac_cap = EDAC_FLAG_SECDED;
	mci->mod_name = EDAC_MOD_STR;
	mci->ctl_page_to_phys = NULL;
	mci->scrub_cap = SCRUB_FLAG_HW_SRC;
	mci->scrub_mode = SCRUB_HW_SRC;
	if (edac->num_err_group <= 0) {
		edac_op_state = EDAC_OPSTATE_POLL;
		if (edac->mc_reg_version == MC_REG_VERSION_1)
			mci->edac_check = phytium_edac_mc_check_ver1;
		else if (edac->mc_reg_version == MC_REG_VERSION_2)
			mci->edac_check = phytium_edac_mc_check_ver2;
	}

	ret = edac_mc_add_mc(mci);
	if (ret) {
		dev_err(edac->dev, "edac mc add failed %d\n", ret);
		goto err_free;
	}

	list_add(&p_ctx->next, &edac->mc_list);

	devres_remove_group(edac->dev, phytium_edac_mc_add);
	dev_info(edac->dev, "Phytium EDAC MC %d registered\n", p_ctx->channel_id);
	return 0;

err_free:
	edac_mc_free(mci);

err_group:
	devres_release_group(edac->dev, phytium_edac_mc_add);

out:
	return ret;
}

static int phytium_edac_mc_remove(struct phytium_edac_mc_ctx *ctx)
{
	edac_mc_del_mc(&ctx->mci->dev);
	edac_mc_free(ctx->mci);
	return 0;
}

static int phytium_edac_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct phytium_edac *edac;
	struct resource *res;
	struct device_node *child;
	int ret = 0;
	int irq_cnt = 0;
	int irq = 0;
	int i = 0;

	edac = devm_kzalloc(&pdev->dev, sizeof(*edac), GFP_KERNEL);
	if (!edac) {
		ret = -ENOMEM;
		goto out;
	}

	edac->dev = &pdev->dev;
	platform_set_drvdata(pdev, edac);
	INIT_LIST_HEAD(&edac->mc_list);

	if (has_acpi_companion(&pdev->dev)) {
		edac->num_err_group = 0;
		ret = device_property_read_u32(&pdev->dev, "mc_version",
				&edac->mc_reg_version);
		if (ret) {
			dev_err(&pdev->dev, "failed to read mc_version: %d\n", ret);
			goto out;
		}

		for (i = 0; ; i ++) {
			res = platform_get_resource(pdev, IORESOURCE_MEM, i);
			if (!res)
				break;

			ret = phytium_edac_mc_add(edac, res, i);
			if (ret < 0)
				goto out;
		}
	} else {
		if (of_device_is_compatible(np, "phytium,pd1904-edac"))
			edac->mc_reg_version = MC_REG_VERSION_1;
		else
			edac->mc_reg_version = MC_REG_VERSION_2;

		edac->error_info = (const struct ras_error_info **)
					of_device_get_match_data(&pdev->dev);

		edac->num_err_group = of_address_count(pdev->dev.of_node);

		/* memory controller reg */
		for_each_child_of_node(pdev->dev.of_node, child) {
			struct resource resource_mc;
			int mc_id = 0;
			if (!of_device_is_available(child))
				continue;
			if (of_device_is_compatible(child, "phytium,edac-mc")) {

				ret = of_address_to_resource(child, 0,
						&resource_mc);
				if (ret < 0) {
					dev_err(edac->dev, "no memory controller reg address %d\n", ret);
					goto out;
				}

				ret = of_property_read_u32(child,
							   "memory-controller",
							   &mc_id);
				if (ret < 0) {
					dev_err(edac->dev, "no memory-controller property\n");
					goto out;
				}

				ret = phytium_edac_mc_add(edac, &resource_mc,
							  mc_id);
				if (ret < 0)
					goto out;
			}
		}
	}

	if (edac->num_err_group > 0) {
		edac->ras_base = devm_kcalloc(&pdev->dev, edac->num_err_group,
				sizeof(*edac->ras_base), GFP_KERNEL);
		if (!edac->ras_base) {
			ret = -ENOMEM;
			goto err_free_mc;
		}
	}

	for (i = 0; i < edac->num_err_group; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		edac->ras_base[i] = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(edac->ras_base[i])) {
			dev_err(&pdev->dev, "no resource address\n");
			ret = PTR_ERR(edac->ras_base[i]);
			goto err_free_mc;
		}
	}

	edac->dfs = edac_debugfs_create_dir(EDAC_MOD_STR);

	ret = phytium_edac_device_add(edac);
	if (ret) {
		dev_err(&pdev->dev, "can't add edac device");
		goto err_free_mc;
	}

	phytium_ras_setup(edac);

	irq_cnt = platform_irq_count(pdev);
	if (irq_cnt < 0 && edac->num_err_group > 0) {
		dev_err(&pdev->dev, "no irq resource\n");
		ret = -EINVAL;
		goto err_free_edac;
	}

	for (i = 0; i < irq_cnt; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq < 0) {
			dev_err(&pdev->dev, "invalid irq resource\n");
			ret = -EINVAL;
			goto err_free_edac;
		}
		ret = devm_request_irq(&pdev->dev, irq,
					  phytium_edac_isr, IRQF_SHARED,
					  EDAC_MOD_STR, edac);
		if (ret) {
			dev_err(&pdev->dev,
				"could not request irq %d\n", irq);
			goto err_free_edac;
		}
	}

	return ret;

err_free_edac:
	phytium_edac_device_remove(edac);

err_free_mc:
	{
		struct phytium_edac_mc_ctx *mc;
		struct phytium_edac_mc_ctx *temp_mc;
		list_for_each_entry_safe(mc, temp_mc, &edac->mc_list, next)
			phytium_edac_mc_remove(mc);
	}

out:
	return ret;
}

static void phytium_edac_remove(struct platform_device *pdev)
{
	struct phytium_edac *edac = dev_get_drvdata(&pdev->dev);
	struct phytium_edac_mc_ctx *mc;
	struct phytium_edac_mc_ctx *temp_mc;

	list_for_each_entry_safe(mc, temp_mc, &edac->mc_list, next)
		phytium_edac_mc_remove(mc);

	phytium_edac_device_remove(edac);

}

static const struct of_device_id phytium_edac_of_match[] = {
	{ .compatible = "phytium,pe220x-edac",
	  .data = pe220x_ras_error },
	{ .compatible = "phytium,pd2208-edac",
	  .data = pd2208_ras_error },
	{ .compatible = "phytium,pd2008-edac",
	  .data = NULL},
	{ .compatible = "phytium,pd1904-edac",
	  .data = NULL},
	{}
};
MODULE_DEVICE_TABLE(of, phytium_edac_of_match);

static const struct acpi_device_id phytium_edac_acpi_ids[] = {
	{"PHYT0073", 0},
	{}
};
MODULE_DEVICE_TABLE(acpi, phytium_edac_acpi_ids);

static struct platform_driver phytium_edac_driver = {
	.probe = phytium_edac_probe,
	.remove = phytium_edac_remove,
	.driver = {
		.name = "phytium-edac",
		.of_match_table = phytium_edac_of_match,
		.acpi_match_table = phytium_edac_acpi_ids,
	},
};

module_platform_driver(phytium_edac_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huangjie <huangjie1663@phytium.com.cn>");
MODULE_VERSION(EDAC_DRIVER_VERSION);
