// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Phytium Pe220x EDAC (error detection and correction)
 *
 * Copyright (c) 2023 Phytium Technology Co., Ltd.
 */

#include <linux/ctype.h>
#include <linux/edac.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <ras/ras_event.h>
#include <linux/uaccess.h>
#include "edac_module.h"

#define EDAC_MOD_STR			"phytium_edac"

/* register offset */
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

#define MAX_ERR_GROUP			3

struct phytium_edac {
	struct device		*dev;
	void __iomem		**ras_base;
	struct dentry		*dfs;
	struct edac_device_ctl_info *edac_dev;
};

struct ras_error_info {
	u32 index;
	u32 error_type;
	const char *error_str;
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
	{ 40, UNCORRECTED_ERROR, "ams_ame0_ras_err" },
	{ 41, UNCORRECTED_ERROR, "ams_ame1_ras_err" },
	{ 42, UNCORRECTED_ERROR, "ams_amer_ras_err" },
	{ 43, UNCORRECTED_ERROR, "ras_err_ame1" },
};

/* pcie controller error record */
static const struct ras_error_info pe220x_ras_peu_psu_error[] = {
	{ 0, CORRECTED_ERROR, "pio_rd_addr_error" },
	{ 1, UNCORRECTED_ERROR, "pio_wr_addr_error" },
	{ 2, CORRECTED_ERROR, "pio_rd_timeout" },
	{ 3, CORRECTED_ERROR, "pio_wr_timeout" },
	{ 4, CORRECTED_ERROR, "axi_b_rsp_error" },
	{ 5, CORRECTED_ERROR, "axi_r_rsp_error" },
};

static const struct ras_error_info pe220x_ras_peu_error[] = {
	{ 0, CORRECTED_ERROR, "pio_rd_addr_error" },
	{ 1, UNCORRECTED_ERROR, "pio_wr_addr_error" },
	{ 2, CORRECTED_ERROR, "pio_rd_timeout" },
	{ 3, CORRECTED_ERROR, "pio_wr_timeout" },
	{ 4, CORRECTED_ERROR, "axi_b_rsp_error" },
	{ 5, CORRECTED_ERROR, "axi_r_rsp_error" },
};

static const struct ras_error_info *pe220x_ras_error[] = {
	pe220x_ras_soc_error, pe220x_ras_peu_psu_error, pe220x_ras_peu_error
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
	 * first error record owned by node present the node configuration
	 */
	for (i = 0; i < MAX_ERR_GROUP; i++) {
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
	char str[255];
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
	if (res || error_group >= MAX_ERR_GROUP) {
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

	dev_dbg(edac->dev, "inject group%d, error_id: %d\n",
			error_group, error_id);

	if (pe220x_ras_error[error_group][error_id].error_type ==
			CORRECTED_ERROR) {
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

	edac_debugfs_create_file("error_inject_ctrl", S_IWUSR, edac->dfs, edac,
				 &phytium_edac_debug_inject_fops[0]);
}

static int phytium_edac_device_add(struct phytium_edac *edac)
{
	struct edac_device_ctl_info *edac_dev;
	int res = 0;

	edac_dev = edac_device_alloc_ctl_info(
			sizeof(struct edac_device_ctl_info),
			"ras", 1, "soc", 1, 0, NULL,
			0, edac_device_alloc_index());
	if (!edac_dev)
		res = -ENOMEM;

	edac_dev->dev = edac->dev;
	edac_dev->mod_name = EDAC_MOD_STR;
	edac_dev->ctl_name = "phytium ras";
	edac_dev->dev_name = "soc";

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
	edac_device_free_ctl_info(edac_dev);
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
	for (i = 0; i < MAX_ERR_GROUP; i++) {
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

	if (i >= MAX_ERR_GROUP) {
		ret = -1;
		dev_warn(edac->dev, "no error detect.\n");
	}

	return ret;
}

static void phytium_edac_error_report(struct phytium_edac *edac,
				const int error_id,
				const int error_group)
{
	const struct ras_error_info *err_info =
		pe220x_ras_error[error_group];

	if (err_info[error_id].error_type == UNCORRECTED_ERROR) {
		edac_printk(KERN_CRIT, EDAC_MOD_STR, "uncorrected error: %s\n",
			err_info[error_id].error_str);
		edac_device_handle_ue(edac->edac_dev, 0, 0,
				err_info[error_id].error_str);
		/* Report the error via the trace interface */
		if (IS_ENABLED(CONFIG_RAS))
			trace_non_standard_event(&NULL_UUID_LE, &NULL_UUID_LE,
					EDAC_MOD_STR, SEV_RECOVERABLE,
					err_info[error_id].error_str,
					strlen(err_info[error_id].error_str));
	} else {
		edac_printk(KERN_CRIT, EDAC_MOD_STR, "corrected error: %s\n",
			err_info[error_id].error_str);
		edac_device_handle_ce(edac->edac_dev, 0, 0,
				err_info[error_id].error_str);
		/* Report the error via the trace interface */
		if (IS_ENABLED(CONFIG_RAS))
			trace_non_standard_event(&NULL_UUID_LE, &NULL_UUID_LE,
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
					const int error_id,
					const int error_group)
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

static int phytium_edac_probe(struct platform_device *pdev)
{
	struct phytium_edac *edac;
	struct resource *res;
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

	edac->ras_base = devm_kcalloc(&pdev->dev, 3,
				sizeof(*edac->ras_base), GFP_KERNEL);
	if (!edac->ras_base) {
		return -ENOMEM;
		goto out;
	}

	for (i = 0; i < MAX_ERR_GROUP; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		edac->ras_base[i] = devm_ioremap_resource(&pdev->dev, res);
		if (IS_ERR(edac->ras_base[i])) {
			dev_err(&pdev->dev, "no resource address\n");
			ret = PTR_ERR(edac->ras_base[i]);
			goto out;
		}
	}

	edac->dfs = edac_debugfs_create_dir(EDAC_MOD_STR);

	ret = phytium_edac_device_add(edac);
	if (ret) {
		dev_err(&pdev->dev, "can't add edac device");
		goto out;
	}

	phytium_ras_setup(edac);

	irq_cnt = platform_irq_count(pdev);
	if (irq_cnt < 0) {
		dev_err(&pdev->dev, "no irq resource\n");
		ret = -EINVAL;
		goto out;
	}

	for (i = 0; i < irq_cnt; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq < 0) {
			dev_err(&pdev->dev, "invalid irq resource\n");
			ret = -EINVAL;
			goto out;
		}
		ret = devm_request_irq(&pdev->dev, irq,
					  phytium_edac_isr, IRQF_SHARED,
					  EDAC_MOD_STR, edac);
		if (ret) {
			dev_err(&pdev->dev,
				"could not request irq %d\n", irq);
			goto out;
		}
	}

out:
	return ret;
}

static int phytium_edac_remove(struct platform_device *pdev)
{
	struct phytium_edac *edac = dev_get_drvdata(&pdev->dev);

	phytium_edac_device_remove(edac);

	return 0;
}

static const struct of_device_id phytium_edac_of_match[] = {
	{ .compatible = "phytium,pe220x-edac" },
	{},
};
MODULE_DEVICE_TABLE(of, phytium_edac_of_match);

static struct platform_driver phytium_edac_driver = {
	.probe = phytium_edac_probe,
	.remove = phytium_edac_remove,
	.driver = {
		.name = "phytium-edac",
		.of_match_table = phytium_edac_of_match,
	},
};

module_platform_driver(phytium_edac_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huangjie <huangjie1663@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium Pe220x EDAC driver");
