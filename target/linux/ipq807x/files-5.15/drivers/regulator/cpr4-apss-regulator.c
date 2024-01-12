/*
 * Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/bitops.h>
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm_opp.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>

#include "cpr3-regulator.h"

#define IPQ807x_APSS_FUSE_CORNERS	4
#define IPQ817x_APPS_FUSE_CORNERS	2
#define IPQ6018_APSS_FUSE_CORNERS 	4
#define IPQ9574_APSS_FUSE_CORNERS       4

u32 g_valid_fuse_count = IPQ807x_APSS_FUSE_CORNERS;

/**
 * struct cpr4_ipq807x_apss_fuses - APSS specific fuse data for IPQ807x
 * @ro_sel:		Ring oscillator select fuse parameter value for each
 *			fuse corner
 * @init_voltage:	Initial (i.e. open-loop) voltage fuse parameter value
 *			for each fuse corner (raw, not converted to a voltage)
 * @target_quot:	CPR target quotient fuse parameter value for each fuse
 *			corner
 * @quot_offset:	CPR target quotient offset fuse parameter value for each
 *			fuse corner (raw, not unpacked) used for target quotient
 *			interpolation
 * @speed_bin:		Application processor speed bin fuse parameter value for
 *			the given chip
 * @cpr_fusing_rev:	CPR fusing revision fuse parameter value
 * @boost_cfg:		CPR boost configuration fuse parameter value
 * @boost_voltage:	CPR boost voltage fuse parameter value (raw, not
 *			converted to a voltage)
 *
 * This struct holds the values for all of the fuses read from memory.
 */
struct cpr4_ipq807x_apss_fuses {
	u64	ro_sel[IPQ807x_APSS_FUSE_CORNERS];
	u64	init_voltage[IPQ807x_APSS_FUSE_CORNERS];
	u64	target_quot[IPQ807x_APSS_FUSE_CORNERS];
	u64	quot_offset[IPQ807x_APSS_FUSE_CORNERS];
	u64	speed_bin;
	u64	cpr_fusing_rev;
	u64	boost_cfg;
	u64	boost_voltage;
	u64	misc;
};

/*
 * fuse combo = fusing revision + 8 * (speed bin)
 * where: fusing revision = 0 - 7 and speed bin = 0 - 7
 */
#define CPR4_IPQ807x_APSS_FUSE_COMBO_COUNT	64

/*
 * Constants which define the name of each fuse corner.
 */
enum cpr4_ipq807x_apss_fuse_corner {
	CPR4_IPQ807x_APSS_FUSE_CORNER_SVS	= 0,
	CPR4_IPQ807x_APSS_FUSE_CORNER_NOM	= 1,
	CPR4_IPQ807x_APSS_FUSE_CORNER_TURBO	= 2,
	CPR4_IPQ807x_APSS_FUSE_CORNER_STURBO	= 3,
};

static const char * const cpr4_ipq807x_apss_fuse_corner_name[] = {
	[CPR4_IPQ807x_APSS_FUSE_CORNER_SVS]	= "SVS",
	[CPR4_IPQ807x_APSS_FUSE_CORNER_NOM]	= "NOM",
	[CPR4_IPQ807x_APSS_FUSE_CORNER_TURBO]	= "TURBO",
	[CPR4_IPQ807x_APSS_FUSE_CORNER_STURBO]	= "STURBO",
};

/*
 * IPQ807x APSS fuse parameter locations:
 *
 * Structs are organized with the following dimensions:
 *	Outer: 0 to 3 for fuse corners from lowest to highest corner
 *	Inner: large enough to hold the longest set of parameter segments which
 *		fully defines a fuse parameter, +1 (for NULL termination).
 *		Each segment corresponds to a contiguous group of bits from a
 *		single fuse row.  These segments are concatentated together in
 *		order to form the full fuse parameter value.  The segments for
 *		a given parameter may correspond to different fuse rows.
 */
static struct cpr3_fuse_param
ipq807x_apss_ro_sel_param[IPQ807x_APSS_FUSE_CORNERS][2] = {
	{{73,  8, 11}, {} },
	{{73,  4,  7}, {} },
	{{73,  0,  3}, {} },
	{{73, 12, 15}, {} },
};

static struct cpr3_fuse_param
ipq807x_apss_init_voltage_param[IPQ807x_APSS_FUSE_CORNERS][2] = {
	{{71, 18, 23}, {} },
	{{71, 12, 17}, {} },
	{{71,  6, 11}, {} },
	{{71,  0,  5}, {} },
};

static struct cpr3_fuse_param
ipq807x_apss_target_quot_param[IPQ807x_APSS_FUSE_CORNERS][2] = {
	{{72, 32, 43}, {} },
	{{72, 20, 31}, {} },
	{{72,  8, 19}, {} },
	{{72, 44, 55}, {} },
};

static struct cpr3_fuse_param
ipq807x_apss_quot_offset_param[IPQ807x_APSS_FUSE_CORNERS][2] = {
	{{} },
	{{71, 46, 52}, {} },
	{{71, 39, 45}, {} },
	{{71, 32, 38}, {} },
};

static struct cpr3_fuse_param ipq807x_cpr_fusing_rev_param[] = {
	{71, 53, 55},
	{},
};

static struct cpr3_fuse_param ipq807x_apss_speed_bin_param[] = {
	{36, 40, 42},
	{},
};

static struct cpr3_fuse_param ipq807x_cpr_boost_fuse_cfg_param[] = {
	{36, 43, 45},
	{},
};

static struct cpr3_fuse_param ipq807x_apss_boost_fuse_volt_param[] = {
	{71, 0, 5},
	{},
};

static struct cpr3_fuse_param ipq807x_misc_fuse_volt_adj_param[] = {
	{36, 54, 54},
	{},
};

static struct cpr3_fuse_parameters ipq807x_fuse_params = {
	.apss_ro_sel_param = ipq807x_apss_ro_sel_param,
	.apss_init_voltage_param = ipq807x_apss_init_voltage_param,
	.apss_target_quot_param = ipq807x_apss_target_quot_param,
	.apss_quot_offset_param = ipq807x_apss_quot_offset_param,
	.cpr_fusing_rev_param = ipq807x_cpr_fusing_rev_param,
	.apss_speed_bin_param = ipq807x_apss_speed_bin_param,
	.cpr_boost_fuse_cfg_param = ipq807x_cpr_boost_fuse_cfg_param,
	.apss_boost_fuse_volt_param = ipq807x_apss_boost_fuse_volt_param,
	.misc_fuse_volt_adj_param = ipq807x_misc_fuse_volt_adj_param
};

/*
 * The number of possible values for misc fuse is
 * 2^(#bits defined for misc fuse)
 */
#define IPQ807x_MISC_FUSE_VAL_COUNT		BIT(1)

/*
 * Open loop voltage fuse reference voltages in microvolts for IPQ807x
 */
static int ipq807x_apss_fuse_ref_volt
	[IPQ807x_APSS_FUSE_CORNERS] = {
	720000,
	864000,
	992000,
	1064000,
};

#define IPQ807x_APSS_FUSE_STEP_VOLT		8000
#define IPQ807x_APSS_VOLTAGE_FUSE_SIZE	6
#define IPQ807x_APSS_QUOT_OFFSET_SCALE	5

#define IPQ807x_APSS_CPR_SENSOR_COUNT	6

#define IPQ807x_APSS_CPR_CLOCK_RATE		19200000

#define IPQ807x_APSS_MAX_TEMP_POINTS	3
#define IPQ807x_APSS_TEMP_SENSOR_ID_START	4
#define IPQ807x_APSS_TEMP_SENSOR_ID_END	13
/*
 * Boost voltage fuse reference and ceiling voltages in microvolts for
 * IPQ807x.
 */
#define IPQ807x_APSS_BOOST_FUSE_REF_VOLT	1140000
#define IPQ807x_APSS_BOOST_CEILING_VOLT	1140000
#define IPQ807x_APSS_BOOST_FLOOR_VOLT	900000
#define MAX_BOOST_CONFIG_FUSE_VALUE		8

#define IPQ807x_APSS_CPR_SDELTA_CORE_COUNT	15

#define IPQ807x_APSS_CPR_TCSR_START		8
#define IPQ807x_APSS_CPR_TCSR_END		9

/*
 * Array of integer values mapped to each of the boost config fuse values to
 * indicate boost enable/disable status.
 */
static bool boost_fuse[MAX_BOOST_CONFIG_FUSE_VALUE] = {0, 1, 1, 1, 1, 1, 1, 1};

/*
 * IPQ6018 (Few parameters are changed, remaining are same as IPQ807x)
 */
#define IPQ6018_APSS_FUSE_STEP_VOLT		12500
#define IPQ6018_APSS_CPR_CLOCK_RATE		24000000

static struct cpr3_fuse_param
ipq6018_apss_ro_sel_param[IPQ6018_APSS_FUSE_CORNERS][2] = {
	{{75,  8, 11}, {} },
	{{75,  4,  7}, {} },
	{{75,  0,  3}, {} },
	{{75, 12, 15}, {} },
};

static struct cpr3_fuse_param
ipq6018_apss_init_voltage_param[IPQ6018_APSS_FUSE_CORNERS][2] = {
	{{73, 18, 23}, {} },
	{{73, 12, 17}, {} },
	{{73,  6, 11}, {} },
	{{73,  0,  5}, {} },
};

static struct cpr3_fuse_param
ipq6018_apss_target_quot_param[IPQ6018_APSS_FUSE_CORNERS][2] = {
	{{74, 32, 43}, {} },
	{{74, 20, 31}, {} },
	{{74,  8, 19}, {} },
	{{74, 44, 55}, {} },
};

static struct cpr3_fuse_param
ipq6018_apss_quot_offset_param[IPQ6018_APSS_FUSE_CORNERS][2] = {
	{{} },
	{{73, 48, 55}, {} },
	{{73, 40, 47}, {} },
	{{73, 32, 39}, {} },
};

static struct cpr3_fuse_param ipq6018_cpr_fusing_rev_param[] = {
	{75, 16, 18},
	{},
};

static struct cpr3_fuse_param ipq6018_apss_speed_bin_param[] = {
	{36, 40, 42},
	{},
};

static struct cpr3_fuse_param ipq6018_cpr_boost_fuse_cfg_param[] = {
	{36, 43, 45},
	{},
};

static struct cpr3_fuse_param ipq6018_apss_boost_fuse_volt_param[] = {
	{73, 0, 5},
	{},
};

static struct cpr3_fuse_param ipq6018_misc_fuse_volt_adj_param[] = {
	{36, 54, 54},
	{},
};

static struct cpr3_fuse_parameters ipq6018_fuse_params = {
	.apss_ro_sel_param = ipq6018_apss_ro_sel_param,
	.apss_init_voltage_param = ipq6018_apss_init_voltage_param,
	.apss_target_quot_param = ipq6018_apss_target_quot_param,
	.apss_quot_offset_param = ipq6018_apss_quot_offset_param,
	.cpr_fusing_rev_param = ipq6018_cpr_fusing_rev_param,
	.apss_speed_bin_param = ipq6018_apss_speed_bin_param,
	.cpr_boost_fuse_cfg_param = ipq6018_cpr_boost_fuse_cfg_param,
	.apss_boost_fuse_volt_param = ipq6018_apss_boost_fuse_volt_param,
	.misc_fuse_volt_adj_param = ipq6018_misc_fuse_volt_adj_param
};


/*
 * Boost voltage fuse reference and ceiling voltages in microvolts for
 * IPQ6018.
 */
#define IPQ6018_APSS_BOOST_FUSE_REF_VOLT	1140000
#define IPQ6018_APSS_BOOST_CEILING_VOLT	1140000
#define IPQ6018_APSS_BOOST_FLOOR_VOLT	900000

/*
 * Open loop voltage fuse reference voltages in microvolts for IPQ807x
 */
static int ipq6018_apss_fuse_ref_volt
	[IPQ6018_APSS_FUSE_CORNERS] = {
	725000,
	862500,
	987500,
	1062500,
};

/*
 * IPQ6018 Memory ACC settings on TCSR
 *
 * Turbo_L1: write TCSR_MEM_ACC_SW_OVERRIDE_LEGACY_APC0 0x10
 *           write TCSR_CUSTOM_VDDAPC0_ACC_1            0x1
 * Other modes: write TCSR_MEM_ACC_SW_OVERRIDE_LEGACY_APC0 0x0
 *              write TCSR_CUSTOM_VDDAPC0_ACC_1            0x0
 *
 */
#define IPQ6018_APSS_MEM_ACC_TCSR_COUNT         2
#define TCSR_MEM_ACC_SW_OVERRIDE_LEGACY_APC0    0x1946178
#define TCSR_CUSTOM_VDDAPC0_ACC_1               0x1946124

struct mem_acc_tcsr {
	u32 phy_addr;
	void __iomem *ioremap_addr;
	u32 value;
};

static struct mem_acc_tcsr ipq6018_mem_acc_tcsr[IPQ6018_APSS_MEM_ACC_TCSR_COUNT] = {
	{TCSR_MEM_ACC_SW_OVERRIDE_LEGACY_APC0, NULL, 0x10},
	{TCSR_CUSTOM_VDDAPC0_ACC_1, NULL, 0x1},
};

/*
 * IPQ9574 (Few parameters are changed, remaining are same as IPQ6018)
 */
#define IPQ9574_APSS_FUSE_STEP_VOLT             10000

static struct cpr3_fuse_param
ipq9574_apss_ro_sel_param[IPQ9574_APSS_FUSE_CORNERS][2] = {
	{{107, 4, 7}, {} },
	{{107, 0, 3}, {} },
	{{106, 4, 7}, {} },
	{{106, 0, 3}, {} },
};

static struct cpr3_fuse_param
ipq9574_apss_init_voltage_param[IPQ9574_APSS_FUSE_CORNERS][2] = {
	{{104, 24, 29}, {} },
	{{104, 18, 23}, {} },
	{{104, 12, 17}, {} },
	{{104,  6, 11}, {} },
};

static struct cpr3_fuse_param
ipq9574_apss_target_quot_param[IPQ9574_APSS_FUSE_CORNERS][2] = {
	{{106, 32, 43}, {} },
	{{106, 20, 31}, {} },
	{{106,  8, 19}, {} },
	{{106, 44, 55}, {} },
};

static struct cpr3_fuse_param
ipq9574_apss_quot_offset_param[IPQ9574_APSS_FUSE_CORNERS][2] = {
	{{} },
	{{105, 48, 55}, {} },
	{{105, 40, 47}, {} },
	{{105, 32, 39}, {} },
};

static struct cpr3_fuse_param ipq9574_cpr_fusing_rev_param[] = {
	{107, 8, 10},
	{},
};

static struct cpr3_fuse_param ipq9574_apss_speed_bin_param[] = {
	{0, 40, 42},
	{},
};

static struct cpr3_fuse_param ipq9574_cpr_boost_fuse_cfg_param[] = {
	{0, 43, 45},
	{},
};

static struct cpr3_fuse_param ipq9574_apss_boost_fuse_volt_param[] = {
	{104, 0, 5},
	{},
};

static struct cpr3_fuse_param ipq9574_misc_fuse_volt_adj_param[] = {
	{0, 54, 54},
	{},
};

static struct cpr3_fuse_parameters ipq9574_fuse_params = {
	.apss_ro_sel_param = ipq9574_apss_ro_sel_param,
	.apss_init_voltage_param = ipq9574_apss_init_voltage_param,
	.apss_target_quot_param = ipq9574_apss_target_quot_param,
	.apss_quot_offset_param = ipq9574_apss_quot_offset_param,
	.cpr_fusing_rev_param = ipq9574_cpr_fusing_rev_param,
	.apss_speed_bin_param = ipq9574_apss_speed_bin_param,
	.cpr_boost_fuse_cfg_param = ipq9574_cpr_boost_fuse_cfg_param,
	.apss_boost_fuse_volt_param = ipq9574_apss_boost_fuse_volt_param,
	.misc_fuse_volt_adj_param = ipq9574_misc_fuse_volt_adj_param
};

/*
 * Open loop voltage fuse reference voltages in microvolts for IPQ9574
 */
static int ipq9574_apss_fuse_ref_volt
	[IPQ9574_APSS_FUSE_CORNERS] = {
	725000,
	862500,
	987500,
	1062500,
};

/**
 * cpr4_ipq807x_apss_read_fuse_data() - load APSS specific fuse parameter values
 * @vreg:		Pointer to the CPR3 regulator
 *
 * This function allocates a cpr4_ipq807x_apss_fuses struct, fills it with
 * values read out of hardware fuses, and finally copies common fuse values
 * into the CPR3 regulator struct.
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_ipq807x_apss_read_fuse_data(struct cpr3_regulator *vreg)
{
	void __iomem *base = vreg->thread->ctrl->fuse_base;
	struct cpr4_ipq807x_apss_fuses *fuse;
	int i, rc;

	fuse = devm_kzalloc(vreg->thread->ctrl->dev, sizeof(*fuse), GFP_KERNEL);
	if (!fuse)
		return -ENOMEM;

	rc = cpr3_read_fuse_param(base, vreg->cpr4_regulator_data->cpr3_fuse_params->apss_speed_bin_param,
				  &fuse->speed_bin);
	if (rc) {
		cpr3_err(vreg, "Unable to read speed bin fuse, rc=%d\n", rc);
		return rc;
	}
	cpr3_info(vreg, "speed bin = %llu\n", fuse->speed_bin);

	rc = cpr3_read_fuse_param(base, vreg->cpr4_regulator_data->cpr3_fuse_params->cpr_fusing_rev_param,
				  &fuse->cpr_fusing_rev);
	if (rc) {
		cpr3_err(vreg, "Unable to read CPR fusing revision fuse, rc=%d\n",
			rc);
		return rc;
	}
	cpr3_info(vreg, "CPR fusing revision = %llu\n", fuse->cpr_fusing_rev);

	rc = cpr3_read_fuse_param(base, vreg->cpr4_regulator_data->cpr3_fuse_params->misc_fuse_volt_adj_param,
				  &fuse->misc);
	if (rc) {
		cpr3_err(vreg, "Unable to read misc voltage adjustment fuse, rc=%d\n",
			rc);
		return rc;
	}
	cpr3_info(vreg, "CPR misc fuse value = %llu\n", fuse->misc);
	if (fuse->misc >= IPQ807x_MISC_FUSE_VAL_COUNT) {
		cpr3_err(vreg, "CPR misc fuse value = %llu, should be < %lu\n",
			fuse->misc, IPQ807x_MISC_FUSE_VAL_COUNT);
		return -EINVAL;
	}

	for (i = 0; i < g_valid_fuse_count; i++) {
		rc = cpr3_read_fuse_param(base,
				vreg->cpr4_regulator_data->cpr3_fuse_params->apss_init_voltage_param[i],
				&fuse->init_voltage[i]);
		if (rc) {
			cpr3_err(vreg, "Unable to read fuse-corner %d initial voltage fuse, rc=%d\n",
				i, rc);
			return rc;
		}

		rc = cpr3_read_fuse_param(base,
				vreg->cpr4_regulator_data->cpr3_fuse_params->apss_target_quot_param[i],
				&fuse->target_quot[i]);
		if (rc) {
			cpr3_err(vreg, "Unable to read fuse-corner %d target quotient fuse, rc=%d\n",
				i, rc);
			return rc;
		}

		rc = cpr3_read_fuse_param(base,
				vreg->cpr4_regulator_data->cpr3_fuse_params->apss_ro_sel_param[i],
				&fuse->ro_sel[i]);
		if (rc) {
			cpr3_err(vreg, "Unable to read fuse-corner %d RO select fuse, rc=%d\n",
				i, rc);
			return rc;
		}

		rc = cpr3_read_fuse_param(base,
				vreg->cpr4_regulator_data->cpr3_fuse_params->apss_quot_offset_param[i],
				&fuse->quot_offset[i]);
		if (rc) {
			cpr3_err(vreg, "Unable to read fuse-corner %d quotient offset fuse, rc=%d\n",
				i, rc);
			return rc;
		}
	}

	rc = cpr3_read_fuse_param(base, vreg->cpr4_regulator_data->cpr3_fuse_params->cpr_boost_fuse_cfg_param,
				  &fuse->boost_cfg);
	if (rc) {
		cpr3_err(vreg, "Unable to read CPR boost config fuse, rc=%d\n",
			rc);
		return rc;
	}
	cpr3_info(vreg, "Voltage boost fuse config = %llu boost = %s\n",
			fuse->boost_cfg, boost_fuse[fuse->boost_cfg]
			? "enable" : "disable");

	rc = cpr3_read_fuse_param(base,
				vreg->cpr4_regulator_data->cpr3_fuse_params->apss_boost_fuse_volt_param,
				&fuse->boost_voltage);
	if (rc) {
		cpr3_err(vreg, "failed to read boost fuse voltage, rc=%d\n",
			rc);
		return rc;
	}

	vreg->fuse_combo = fuse->cpr_fusing_rev + 8 * fuse->speed_bin;
	if (vreg->fuse_combo >= CPR4_IPQ807x_APSS_FUSE_COMBO_COUNT) {
		cpr3_err(vreg, "invalid CPR fuse combo = %d found\n",
			vreg->fuse_combo);
		return -EINVAL;
	}

	vreg->speed_bin_fuse	= fuse->speed_bin;
	vreg->cpr_rev_fuse	= fuse->cpr_fusing_rev;
	vreg->fuse_corner_count	= g_valid_fuse_count;
	vreg->platform_fuses	= fuse;

	return 0;
}

/**
 * cpr4_apss_parse_corner_data() - parse APSS corner data from device tree
 *		properties of the CPR3 regulator's device node
 * @vreg:		Pointer to the CPR3 regulator
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_apss_parse_corner_data(struct cpr3_regulator *vreg)
{
	struct device_node *node = vreg->of_node;
	struct cpr4_ipq807x_apss_fuses *fuse = vreg->platform_fuses;
	u32 *temp = NULL;
	int i, rc;

	rc = cpr3_parse_common_corner_data(vreg);
	if (rc) {
		cpr3_err(vreg, "error reading corner data, rc=%d\n", rc);
		return rc;
	}

	/* If fuse has incorrect RO Select values and dtsi has "qcom,cpr-ro-sel"
	 * entry with RO select values other than zero, then dtsi values will
	 * be used.
	 */
	if (of_find_property(node, "qcom,cpr-ro-sel", NULL)) {
		temp = kcalloc(vreg->fuse_corner_count, sizeof(*temp),
				GFP_KERNEL);
		if (!temp)
			return -ENOMEM;

		rc = cpr3_parse_array_property(vreg, "qcom,cpr-ro-sel",
				vreg->fuse_corner_count, temp);
		if (rc)
			goto done;

		for (i = 0; i < vreg->fuse_corner_count; i++) {
			if (temp[i] != 0)
				fuse->ro_sel[i] = temp[i];
		}
	}
done:
	kfree(temp);
	return rc;
}

/**
 * cpr4_apss_parse_misc_fuse_voltage_adjustments() - fill an array from a
 *		portion of the voltage adjustments specified based on
 *		miscellaneous fuse bits.
 * @vreg:		Pointer to the CPR3 regulator
 * @volt_adjust:	Voltage adjustment output data array which must be
 *			of size vreg->corner_count
 *
 * cpr3_parse_common_corner_data() must be called for vreg before this function
 * is called so that speed bin size elements are initialized.
 *
 * Two formats are supported for the device tree property:
 * 1. Length == tuple_list_size * vreg->corner_count
 *	(reading begins at index 0)
 * 2. Length == tuple_list_size * vreg->speed_bin_corner_sum
 *	(reading begins at index tuple_list_size * vreg->speed_bin_offset)
 *
 * Here, tuple_list_size is the number of possible values for misc fuse.
 * All other property lengths are treated as errors.
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_apss_parse_misc_fuse_voltage_adjustments(
	struct cpr3_regulator *vreg, u32 *volt_adjust)
{
	struct device_node *node = vreg->of_node;
	struct cpr4_ipq807x_apss_fuses *fuse = vreg->platform_fuses;
	int tuple_list_size = IPQ807x_MISC_FUSE_VAL_COUNT;
	int i, offset, rc, len = 0;
	const char *prop_name = "qcom,cpr-misc-fuse-voltage-adjustment";

	if (!of_find_property(node, prop_name, &len)) {
		cpr3_err(vreg, "property %s is missing\n", prop_name);
		return -EINVAL;
	}

	if (len == tuple_list_size * vreg->corner_count * sizeof(u32)) {
		offset = 0;
	} else if (vreg->speed_bin_corner_sum > 0 &&
			len == tuple_list_size * vreg->speed_bin_corner_sum
			* sizeof(u32)) {
		offset = tuple_list_size * vreg->speed_bin_offset
			+ fuse->misc * vreg->corner_count;
	} else {
		if (vreg->speed_bin_corner_sum > 0)
			cpr3_err(vreg, "property %s has invalid length=%d, should be %zu or %zu\n",
				prop_name, len,
				tuple_list_size * vreg->corner_count
					* sizeof(u32),
				tuple_list_size * vreg->speed_bin_corner_sum
					* sizeof(u32));
		else
			cpr3_err(vreg, "property %s has invalid length=%d, should be %zu\n",
				prop_name, len,
				tuple_list_size * vreg->corner_count
				* sizeof(u32));
		return -EINVAL;
	}

	for (i = 0; i < vreg->corner_count; i++) {
		rc = of_property_read_u32_index(node, prop_name, offset + i,
						&volt_adjust[i]);
		if (rc) {
			cpr3_err(vreg, "error reading property %s, rc=%d\n",
				prop_name, rc);
			return rc;
		}
	}

	return 0;
}

/**
 * cpr4_ipq807x_apss_calculate_open_loop_voltages() - calculate the open-loop
 *		voltage for each corner of a CPR3 regulator
 * @vreg:		Pointer to the CPR3 regulator
 *
 * If open-loop voltage interpolation is allowed in device tree, then
 * this function calculates the open-loop voltage for a given corner using
 * linear interpolation.  This interpolation is performed using the processor
 * frequencies of the lower and higher Fmax corners along with their fused
 * open-loop voltages.
 *
 * If open-loop voltage interpolation is not allowed, then this function uses
 * the Fmax fused open-loop voltage for all of the corners associated with a
 * given fuse corner.
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_ipq807x_apss_calculate_open_loop_voltages(
			struct cpr3_regulator *vreg)
{
	struct device_node *node = vreg->of_node;
	struct cpr4_ipq807x_apss_fuses *fuse = vreg->platform_fuses;
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	int i, j, rc = 0;
	bool allow_interpolation;
	u64 freq_low, volt_low, freq_high, volt_high;
	int *fuse_volt, *misc_adj_volt;
	int *fmax_corner;

	fuse_volt = kcalloc(vreg->fuse_corner_count, sizeof(*fuse_volt),
				GFP_KERNEL);
	fmax_corner = kcalloc(vreg->fuse_corner_count, sizeof(*fmax_corner),
				GFP_KERNEL);
	if (!fuse_volt || !fmax_corner) {
		rc = -ENOMEM;
		goto done;
	}

	for (i = 0; i < vreg->fuse_corner_count; i++) {
		if (ctrl->cpr_global_setting == CPR_DISABLED)
			fuse_volt[i] = vreg->cpr4_regulator_data->fuse_ref_volt[i];
		else
			fuse_volt[i] = cpr3_convert_open_loop_voltage_fuse(
				vreg->cpr4_regulator_data->fuse_ref_volt[i],
				vreg->cpr4_regulator_data->fuse_step_volt,
				fuse->init_voltage[i],
				IPQ807x_APSS_VOLTAGE_FUSE_SIZE);

		/* Log fused open-loop voltage values for debugging purposes. */
		cpr3_info(vreg, "fused %8s: open-loop=%7d uV\n",
			  cpr4_ipq807x_apss_fuse_corner_name[i],
			  fuse_volt[i]);
	}

	rc = cpr3_determine_part_type(vreg,
			  fuse_volt[vreg->fuse_corner_count - 1]);
	if (rc) {
		cpr3_err(vreg, "fused part type detection failed failed, rc=%d\n",
			rc);
		goto done;
	}

	rc = cpr3_adjust_fused_open_loop_voltages(vreg, fuse_volt);
	if (rc) {
		cpr3_err(vreg, "fused open-loop voltage adjustment failed, rc=%d\n",
			rc);
		goto done;
	}

	allow_interpolation = of_property_read_bool(node,
				"qcom,allow-voltage-interpolation");

	for (i = 1; i < vreg->fuse_corner_count; i++) {
		if (fuse_volt[i] < fuse_volt[i - 1]) {
			cpr3_info(vreg, "fuse corner %d voltage=%d uV < fuse corner %d voltage=%d uV; overriding: fuse corner %d voltage=%d\n",
				i, fuse_volt[i], i - 1, fuse_volt[i - 1],
				i, fuse_volt[i - 1]);
			fuse_volt[i] = fuse_volt[i - 1];
		}
	}

	if (!allow_interpolation) {
		/* Use fused open-loop voltage for lower frequencies. */
		for (i = 0; i < vreg->corner_count; i++)
			vreg->corner[i].open_loop_volt
				= fuse_volt[vreg->corner[i].cpr_fuse_corner];
		goto done;
	}

	/* Determine highest corner mapped to each fuse corner */
	j = vreg->fuse_corner_count - 1;
	for (i = vreg->corner_count - 1; i >= 0; i--) {
		if (vreg->corner[i].cpr_fuse_corner == j) {
			fmax_corner[j] = i;
			j--;
		}
	}
	if (j >= 0) {
		cpr3_err(vreg, "invalid fuse corner mapping\n");
		rc = -EINVAL;
		goto done;
	}

	/*
	 * Interpolation is not possible for corners mapped to the lowest fuse
	 * corner so use the fuse corner value directly.
	 */
	for (i = 0; i <= fmax_corner[0]; i++)
		vreg->corner[i].open_loop_volt = fuse_volt[0];

	/* Interpolate voltages for the higher fuse corners. */
	for (i = 1; i < vreg->fuse_corner_count; i++) {
		freq_low = vreg->corner[fmax_corner[i - 1]].proc_freq;
		volt_low = fuse_volt[i - 1];
		freq_high = vreg->corner[fmax_corner[i]].proc_freq;
		volt_high = fuse_volt[i];

		for (j = fmax_corner[i - 1] + 1; j <= fmax_corner[i]; j++)
			vreg->corner[j].open_loop_volt = cpr3_interpolate(
				freq_low, volt_low, freq_high, volt_high,
				vreg->corner[j].proc_freq);
	}

done:
	if (rc == 0) {
		cpr3_debug(vreg, "unadjusted per-corner open-loop voltages:\n");
		for (i = 0; i < vreg->corner_count; i++)
			cpr3_debug(vreg, "open-loop[%2d] = %d uV\n", i,
				vreg->corner[i].open_loop_volt);

		rc = cpr3_adjust_open_loop_voltages(vreg);
		if (rc)
			cpr3_err(vreg, "open-loop voltage adjustment failed, rc=%d\n",
				rc);

		if (of_find_property(node,
			"qcom,cpr-misc-fuse-voltage-adjustment",
			NULL)) {
			misc_adj_volt = kcalloc(vreg->corner_count,
					sizeof(*misc_adj_volt), GFP_KERNEL);
			if (!misc_adj_volt) {
				rc = -ENOMEM;
				goto _exit;
			}

			rc = cpr4_apss_parse_misc_fuse_voltage_adjustments(vreg,
				misc_adj_volt);
			if (rc) {
				cpr3_err(vreg, "qcom,cpr-misc-fuse-voltage-adjustment reading failed, rc=%d\n",
					rc);
				kfree(misc_adj_volt);
				goto _exit;
			}

			for (i = 0; i < vreg->corner_count; i++)
				vreg->corner[i].open_loop_volt
						+= misc_adj_volt[i];
			kfree(misc_adj_volt);
		}
	}

_exit:
	kfree(fuse_volt);
	kfree(fmax_corner);
	return rc;
}

/**
 * cpr4_ipq807x_apss_set_no_interpolation_quotients() - use the fused target
 *		quotient values for lower frequencies.
 * @vreg:		Pointer to the CPR3 regulator
 * @volt_adjust:	Pointer to array of per-corner closed-loop adjustment
 *			voltages
 * @volt_adjust_fuse:	Pointer to array of per-fuse-corner closed-loop
 *			adjustment voltages
 * @ro_scale:		Pointer to array of per-fuse-corner RO scaling factor
 *			values with units of QUOT/V
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_ipq807x_apss_set_no_interpolation_quotients(
			struct cpr3_regulator *vreg, int *volt_adjust,
			int *volt_adjust_fuse, int *ro_scale)
{
	struct cpr4_ipq807x_apss_fuses *fuse = vreg->platform_fuses;
	u32 quot, ro;
	int quot_adjust;
	int i, fuse_corner;

	for (i = 0; i < vreg->corner_count; i++) {
		fuse_corner = vreg->corner[i].cpr_fuse_corner;
		quot = fuse->target_quot[fuse_corner];
		quot_adjust = cpr3_quot_adjustment(ro_scale[fuse_corner],
					   volt_adjust_fuse[fuse_corner] +
					   volt_adjust[i]);
		ro = fuse->ro_sel[fuse_corner];
		vreg->corner[i].target_quot[ro] = quot + quot_adjust;
		cpr3_debug(vreg, "corner=%d RO=%u target quot=%u\n",
			  i, ro, quot);

		if (quot_adjust)
			cpr3_debug(vreg, "adjusted corner %d RO%u target quot: %u --> %u (%d uV)\n",
				  i, ro, quot, vreg->corner[i].target_quot[ro],
				  volt_adjust_fuse[fuse_corner] +
				  volt_adjust[i]);
	}

	return 0;
}

/**
 * cpr4_ipq807x_apss_calculate_target_quotients() - calculate the CPR target
 *		quotient for each corner of a CPR3 regulator
 * @vreg:		Pointer to the CPR3 regulator
 *
 * If target quotient interpolation is allowed in device tree, then this
 * function calculates the target quotient for a given corner using linear
 * interpolation.  This interpolation is performed using the processor
 * frequencies of the lower and higher Fmax corners along with the fused
 * target quotient and quotient offset of the higher Fmax corner.
 *
 * If target quotient interpolation is not allowed, then this function uses
 * the Fmax fused target quotient for all of the corners associated with a
 * given fuse corner.
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_ipq807x_apss_calculate_target_quotients(
			struct cpr3_regulator *vreg)
{
	struct cpr4_ipq807x_apss_fuses *fuse = vreg->platform_fuses;
	int rc;
	bool allow_interpolation;
	u64 freq_low, freq_high, prev_quot;
	u64 *quot_low;
	u64 *quot_high;
	u32 quot, ro;
	int i, j, fuse_corner, quot_adjust;
	int *fmax_corner;
	int *volt_adjust, *volt_adjust_fuse, *ro_scale;
	int *voltage_adj_misc;

	/* Log fused quotient values for debugging purposes. */
	for (i = CPR4_IPQ807x_APSS_FUSE_CORNER_SVS;
		i < vreg->fuse_corner_count; i++)
		cpr3_info(vreg, "fused %8s: quot[%2llu]=%4llu, quot_offset[%2llu]=%4llu\n",
			cpr4_ipq807x_apss_fuse_corner_name[i],
			fuse->ro_sel[i], fuse->target_quot[i],
			fuse->ro_sel[i], fuse->quot_offset[i] *
			IPQ807x_APSS_QUOT_OFFSET_SCALE);

	allow_interpolation = of_property_read_bool(vreg->of_node,
					"qcom,allow-quotient-interpolation");

	volt_adjust = kcalloc(vreg->corner_count, sizeof(*volt_adjust),
					GFP_KERNEL);
	volt_adjust_fuse = kcalloc(vreg->fuse_corner_count,
					sizeof(*volt_adjust_fuse), GFP_KERNEL);
	ro_scale = kcalloc(vreg->fuse_corner_count, sizeof(*ro_scale),
					GFP_KERNEL);
	fmax_corner = kcalloc(vreg->fuse_corner_count, sizeof(*fmax_corner),
					GFP_KERNEL);
	quot_low = kcalloc(vreg->fuse_corner_count, sizeof(*quot_low),
					GFP_KERNEL);
	quot_high = kcalloc(vreg->fuse_corner_count, sizeof(*quot_high),
					GFP_KERNEL);
	if (!volt_adjust || !volt_adjust_fuse || !ro_scale ||
	    !fmax_corner || !quot_low || !quot_high) {
		rc = -ENOMEM;
		goto done;
	}

	rc = cpr3_parse_closed_loop_voltage_adjustments(vreg, &fuse->ro_sel[0],
				volt_adjust, volt_adjust_fuse, ro_scale);
	if (rc) {
		cpr3_err(vreg, "could not load closed-loop voltage adjustments, rc=%d\n",
			rc);
		goto done;
	}

	if (of_find_property(vreg->of_node,
		"qcom,cpr-misc-fuse-voltage-adjustment", NULL)) {
		voltage_adj_misc = kcalloc(vreg->corner_count,
				sizeof(*voltage_adj_misc), GFP_KERNEL);
		if (!voltage_adj_misc) {
			rc = -ENOMEM;
			goto done;
		}

		rc = cpr4_apss_parse_misc_fuse_voltage_adjustments(vreg,
			voltage_adj_misc);
		if (rc) {
			cpr3_err(vreg, "qcom,cpr-misc-fuse-voltage-adjustment reading failed, rc=%d\n",
				rc);
			kfree(voltage_adj_misc);
			goto done;
		}

		for (i = 0; i < vreg->corner_count; i++)
			volt_adjust[i] += voltage_adj_misc[i];

		kfree(voltage_adj_misc);
	}

	if (!allow_interpolation) {
		/* Use fused target quotients for lower frequencies. */
		return cpr4_ipq807x_apss_set_no_interpolation_quotients(
				vreg, volt_adjust, volt_adjust_fuse, ro_scale);
	}

	/* Determine highest corner mapped to each fuse corner */
	j = vreg->fuse_corner_count - 1;
	for (i = vreg->corner_count - 1; i >= 0; i--) {
		if (vreg->corner[i].cpr_fuse_corner == j) {
			fmax_corner[j] = i;
			j--;
		}
	}
	if (j >= 0) {
		cpr3_err(vreg, "invalid fuse corner mapping\n");
		rc = -EINVAL;
		goto done;
	}

	/*
	 * Interpolation is not possible for corners mapped to the lowest fuse
	 * corner so use the fuse corner value directly.
	 */
	i = CPR4_IPQ807x_APSS_FUSE_CORNER_SVS;
	quot_adjust = cpr3_quot_adjustment(ro_scale[i], volt_adjust_fuse[i]);
	quot = fuse->target_quot[i] + quot_adjust;
	quot_high[i] = quot_low[i] = quot;
	ro = fuse->ro_sel[i];
	if (quot_adjust)
		cpr3_debug(vreg, "adjusted fuse corner %d RO%u target quot: %llu --> %u (%d uV)\n",
			i, ro, fuse->target_quot[i], quot, volt_adjust_fuse[i]);

	for (i = 0; i <= fmax_corner[CPR4_IPQ807x_APSS_FUSE_CORNER_SVS];
		i++)
		vreg->corner[i].target_quot[ro] = quot;

	for (i = CPR4_IPQ807x_APSS_FUSE_CORNER_NOM;
	     i < vreg->fuse_corner_count; i++) {
		quot_high[i] = fuse->target_quot[i];
		if (fuse->ro_sel[i] == fuse->ro_sel[i - 1])
			quot_low[i] = quot_high[i - 1];
		else
			quot_low[i] = quot_high[i]
					- fuse->quot_offset[i]
					  * IPQ807x_APSS_QUOT_OFFSET_SCALE;
		if (quot_high[i] < quot_low[i]) {
			cpr3_debug(vreg, "quot_high[%d]=%llu < quot_low[%d]=%llu; overriding: quot_high[%d]=%llu\n",
				i, quot_high[i], i, quot_low[i],
				i, quot_low[i]);
			quot_high[i] = quot_low[i];
		}
	}

	/* Perform per-fuse-corner target quotient adjustment */
	for (i = 1; i < vreg->fuse_corner_count; i++) {
		quot_adjust = cpr3_quot_adjustment(ro_scale[i],
						   volt_adjust_fuse[i]);
		if (quot_adjust) {
			prev_quot = quot_high[i];
			quot_high[i] += quot_adjust;
			cpr3_debug(vreg, "adjusted fuse corner %d RO%llu target quot: %llu --> %llu (%d uV)\n",
				i, fuse->ro_sel[i], prev_quot, quot_high[i],
				volt_adjust_fuse[i]);
		}

		if (fuse->ro_sel[i] == fuse->ro_sel[i - 1])
			quot_low[i] = quot_high[i - 1];
		else
			quot_low[i] += cpr3_quot_adjustment(ro_scale[i],
						    volt_adjust_fuse[i - 1]);

		if (quot_high[i] < quot_low[i]) {
			cpr3_debug(vreg, "quot_high[%d]=%llu < quot_low[%d]=%llu after adjustment; overriding: quot_high[%d]=%llu\n",
				i, quot_high[i], i, quot_low[i],
				i, quot_low[i]);
			quot_high[i] = quot_low[i];
		}
	}

	/* Interpolate voltages for the higher fuse corners. */
	for (i = 1; i < vreg->fuse_corner_count; i++) {
		freq_low = vreg->corner[fmax_corner[i - 1]].proc_freq;
		freq_high = vreg->corner[fmax_corner[i]].proc_freq;

		ro = fuse->ro_sel[i];
		for (j = fmax_corner[i - 1] + 1; j <= fmax_corner[i]; j++)
			vreg->corner[j].target_quot[ro] = cpr3_interpolate(
				freq_low, quot_low[i], freq_high, quot_high[i],
				vreg->corner[j].proc_freq);
	}

	/* Perform per-corner target quotient adjustment */
	for (i = 0; i < vreg->corner_count; i++) {
		fuse_corner = vreg->corner[i].cpr_fuse_corner;
		ro = fuse->ro_sel[fuse_corner];
		quot_adjust = cpr3_quot_adjustment(ro_scale[fuse_corner],
						   volt_adjust[i]);
		if (quot_adjust) {
			prev_quot = vreg->corner[i].target_quot[ro];
			vreg->corner[i].target_quot[ro] += quot_adjust;
			cpr3_debug(vreg, "adjusted corner %d RO%u target quot: %llu --> %u (%d uV)\n",
				i, ro, prev_quot,
				vreg->corner[i].target_quot[ro],
				volt_adjust[i]);
		}
	}

	/* Ensure that target quotients increase monotonically */
	for (i = 1; i < vreg->corner_count; i++) {
		ro = fuse->ro_sel[vreg->corner[i].cpr_fuse_corner];
		if (fuse->ro_sel[vreg->corner[i - 1].cpr_fuse_corner] == ro
		    && vreg->corner[i].target_quot[ro]
				< vreg->corner[i - 1].target_quot[ro]) {
			cpr3_debug(vreg, "adjusted corner %d RO%u target quot=%u < adjusted corner %d RO%u target quot=%u; overriding: corner %d RO%u target quot=%u\n",
				i, ro, vreg->corner[i].target_quot[ro],
				i - 1, ro, vreg->corner[i - 1].target_quot[ro],
				i, ro, vreg->corner[i - 1].target_quot[ro]);
			vreg->corner[i].target_quot[ro]
				= vreg->corner[i - 1].target_quot[ro];
		}
	}

done:
	kfree(volt_adjust);
	kfree(volt_adjust_fuse);
	kfree(ro_scale);
	kfree(fmax_corner);
	kfree(quot_low);
	kfree(quot_high);
	return rc;
}

/**
 * cpr4_apss_print_settings() - print out APSS CPR configuration settings into
 *		the kernel log for debugging purposes
 * @vreg:		Pointer to the CPR3 regulator
 */
static void cpr4_apss_print_settings(struct cpr3_regulator *vreg)
{
	struct cpr3_corner *corner;
	int i;

	cpr3_debug(vreg, "Corner: Frequency (Hz), Fuse Corner, Floor (uV), Open-Loop (uV), Ceiling (uV)\n");
	for (i = 0; i < vreg->corner_count; i++) {
		corner = &vreg->corner[i];
		cpr3_debug(vreg, "%3d: %10u, %2d, %7d, %7d, %7d\n",
			i, corner->proc_freq, corner->cpr_fuse_corner,
			corner->floor_volt, corner->open_loop_volt,
			corner->ceiling_volt);
	}

	if (vreg->thread->ctrl->apm)
		cpr3_debug(vreg, "APM threshold = %d uV, APM adjust = %d uV\n",
			vreg->thread->ctrl->apm_threshold_volt,
			vreg->thread->ctrl->apm_adj_volt);
}

/**
 * cpr4_apss_init_thread() - perform steps necessary to initialize the
 *		configuration data for a CPR3 thread
 * @thread:		Pointer to the CPR3 thread
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_apss_init_thread(struct cpr3_thread *thread)
{
	int rc;

	rc = cpr3_parse_common_thread_data(thread);
	if (rc) {
		cpr3_err(thread->ctrl, "thread %u unable to read CPR thread data from device tree, rc=%d\n",
			thread->thread_id, rc);
		return rc;
	}

	return 0;
}

/**
 * cpr4_apss_parse_temp_adj_properties() - parse temperature based
 *		adjustment properties from device tree.
 * @ctrl:	Pointer to the CPR3 controller
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_apss_parse_temp_adj_properties(struct cpr3_controller *ctrl)
{
	struct device_node *of_node = ctrl->dev->of_node;
	int rc, i, len, temp_point_count;

	if (!of_find_property(of_node, "qcom,cpr-temp-point-map", &len)) {
		/*
		 * Temperature based adjustments are not defined. Single
		 * temperature band is still valid for per-online-core
		 * adjustments.
		 */
		ctrl->temp_band_count = 1;
		return 0;
	}

	temp_point_count = len / sizeof(u32);
	if (temp_point_count <= 0 ||
	    temp_point_count > IPQ807x_APSS_MAX_TEMP_POINTS) {
		cpr3_err(ctrl, "invalid number of temperature points %d > %d (max)\n",
			 temp_point_count, IPQ807x_APSS_MAX_TEMP_POINTS);
		return -EINVAL;
	}

	ctrl->temp_points = devm_kcalloc(ctrl->dev, temp_point_count,
					sizeof(*ctrl->temp_points), GFP_KERNEL);
	if (!ctrl->temp_points)
		return -ENOMEM;

	rc = of_property_read_u32_array(of_node, "qcom,cpr-temp-point-map",
					ctrl->temp_points, temp_point_count);
	if (rc) {
		cpr3_err(ctrl, "error reading property qcom,cpr-temp-point-map, rc=%d\n",
			 rc);
		return rc;
	}

	for (i = 0; i < temp_point_count; i++)
		cpr3_debug(ctrl, "Temperature Point %d=%d\n", i,
				   ctrl->temp_points[i]);

	/*
	 * If t1, t2, and t3 are the temperature points, then the temperature
	 * bands are: (-inf, t1], (t1, t2], (t2, t3], and (t3, inf).
	 */
	ctrl->temp_band_count = temp_point_count + 1;
	cpr3_debug(ctrl, "Number of temp bands =%d\n", ctrl->temp_band_count);

	rc = of_property_read_u32(of_node, "qcom,cpr-initial-temp-band",
				  &ctrl->initial_temp_band);
	if (rc) {
		cpr3_err(ctrl, "error reading qcom,cpr-initial-temp-band, rc=%d\n",
			rc);
		return rc;
	}

	if (ctrl->initial_temp_band >= ctrl->temp_band_count) {
		cpr3_err(ctrl, "Initial temperature band value %d should be in range [0 - %d]\n",
			ctrl->initial_temp_band, ctrl->temp_band_count - 1);
		return -EINVAL;
	}

	ctrl->temp_sensor_id_start = IPQ807x_APSS_TEMP_SENSOR_ID_START;
	ctrl->temp_sensor_id_end = IPQ807x_APSS_TEMP_SENSOR_ID_END;
	ctrl->allow_temp_adj = true;
	return rc;
}

/**
 * cpr4_apss_parse_boost_properties() - parse configuration data for boost
 *		voltage adjustment for CPR3 regulator from device tree.
 * @vreg:	Pointer to the CPR3 regulator
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_apss_parse_boost_properties(struct cpr3_regulator *vreg)
{
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	struct cpr4_ipq807x_apss_fuses *fuse = vreg->platform_fuses;
	struct cpr3_corner *corner;
	int i, boost_voltage, final_boost_volt, rc = 0;
	int *boost_table = NULL, *boost_temp_adj = NULL;
	int boost_voltage_adjust = 0, boost_num_cores = 0;
	u32 boost_allowed = 0;

	if (!boost_fuse[fuse->boost_cfg])
		/* Voltage boost is disabled in fuse */
		return 0;

	if (of_find_property(vreg->of_node, "qcom,allow-boost", NULL)) {
		rc = cpr3_parse_array_property(vreg, "qcom,allow-boost", 1,
				&boost_allowed);
		if (rc)
			return rc;
	}

	if (!boost_allowed) {
		/* Voltage boost is not enabled for this regulator */
		return 0;
	}

	boost_voltage = cpr3_convert_open_loop_voltage_fuse(
				vreg->cpr4_regulator_data->boost_fuse_ref_volt,
				vreg->cpr4_regulator_data->fuse_step_volt,
				fuse->boost_voltage,
				IPQ807x_APSS_VOLTAGE_FUSE_SIZE);

	/* Log boost voltage value for debugging purposes. */
	cpr3_info(vreg, "Boost open-loop=%7d uV\n", boost_voltage);

	if (of_find_property(vreg->of_node,
			"qcom,cpr-boost-voltage-fuse-adjustment", NULL)) {
		rc = cpr3_parse_array_property(vreg,
			"qcom,cpr-boost-voltage-fuse-adjustment",
			1, &boost_voltage_adjust);
		if (rc) {
			cpr3_err(vreg, "qcom,cpr-boost-voltage-fuse-adjustment reading failed, rc=%d\n",
				rc);
			return rc;
		}

		boost_voltage += boost_voltage_adjust;
		/* Log boost voltage value for debugging purposes. */
		cpr3_info(vreg, "Adjusted boost open-loop=%7d uV\n",
			boost_voltage);
	}

	/* Limit boost voltage value between ceiling and floor voltage limits */
	boost_voltage = min(boost_voltage, vreg->cpr4_regulator_data->boost_ceiling_volt);
	boost_voltage = max(boost_voltage, vreg->cpr4_regulator_data->boost_floor_volt);

	/*
	 * The boost feature can only be used for the highest voltage corner.
	 * Also, keep core-count adjustments disabled when the boost feature
	 * is enabled.
	 */
	corner = &vreg->corner[vreg->corner_count - 1];
	if (!corner->sdelta) {
		/*
		 * If core-count/temp adjustments are not defined, the cpr4
		 * sdelta for this corner will not be allocated. Allocate it
		 * here for boost configuration.
		 */
		corner->sdelta = devm_kzalloc(ctrl->dev,
					sizeof(*corner->sdelta), GFP_KERNEL);
		if (!corner->sdelta)
			return -ENOMEM;
	}
	corner->sdelta->temp_band_count = ctrl->temp_band_count;

	rc = of_property_read_u32(vreg->of_node, "qcom,cpr-num-boost-cores",
				&boost_num_cores);
	if (rc) {
		cpr3_err(vreg, "qcom,cpr-num-boost-cores reading failed, rc=%d\n",
			rc);
		return rc;
	}

	if (boost_num_cores <= 0 ||
	    boost_num_cores > IPQ807x_APSS_CPR_SDELTA_CORE_COUNT) {
		cpr3_err(vreg, "Invalid boost number of cores = %d\n",
			boost_num_cores);
		return -EINVAL;
	}
	corner->sdelta->boost_num_cores = boost_num_cores;

	boost_table = devm_kcalloc(ctrl->dev, corner->sdelta->temp_band_count,
					sizeof(*boost_table), GFP_KERNEL);
	if (!boost_table)
		return -ENOMEM;

	if (of_find_property(vreg->of_node,
				"qcom,cpr-boost-temp-adjustment", NULL)) {
		boost_temp_adj = kcalloc(corner->sdelta->temp_band_count,
					sizeof(*boost_temp_adj), GFP_KERNEL);
		if (!boost_temp_adj)
			return -ENOMEM;

		rc = cpr3_parse_array_property(vreg,
				"qcom,cpr-boost-temp-adjustment",
				corner->sdelta->temp_band_count,
				boost_temp_adj);
		if (rc) {
			cpr3_err(vreg, "qcom,cpr-boost-temp-adjustment reading failed, rc=%d\n",
				rc);
			goto done;
		}
	}

	for (i = 0; i < corner->sdelta->temp_band_count; i++) {
		/* Apply static adjustments to boost voltage */
		final_boost_volt = boost_voltage + (boost_temp_adj == NULL
						? 0 : boost_temp_adj[i]);
		/*
		 * Limit final adjusted boost voltage value between ceiling
		 * and floor voltage limits
		 */
		final_boost_volt = min(final_boost_volt,
					vreg->cpr4_regulator_data->boost_ceiling_volt);
		final_boost_volt = max(final_boost_volt,
					vreg->cpr4_regulator_data->boost_floor_volt);

		boost_table[i] = (corner->open_loop_volt - final_boost_volt)
					/ ctrl->step_volt;
		cpr3_debug(vreg, "Adjusted boost voltage margin for temp band %d = %d steps\n",
			i, boost_table[i]);
	}

	corner->ceiling_volt = vreg->cpr4_regulator_data->boost_ceiling_volt;
	corner->sdelta->boost_table = boost_table;
	corner->sdelta->allow_boost = true;
	corner->sdelta->allow_core_count_adj = false;
	vreg->allow_boost = true;
	ctrl->allow_boost = true;
done:
	kfree(boost_temp_adj);
	return rc;
}

/**
 * cpr4_apss_init_regulator() - perform all steps necessary to initialize the
 *		configuration data for a CPR3 regulator
 * @vreg:		Pointer to the CPR3 regulator
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_apss_init_regulator(struct cpr3_regulator *vreg)
{
	struct cpr4_ipq807x_apss_fuses *fuse;
	int rc;

	rc = cpr4_ipq807x_apss_read_fuse_data(vreg);
	if (rc) {
		cpr3_err(vreg, "unable to read CPR fuse data, rc=%d\n", rc);
		return rc;
	}

	fuse = vreg->platform_fuses;

	rc = cpr4_apss_parse_corner_data(vreg);
	if (rc) {
		cpr3_err(vreg, "unable to read CPR corner data from device tree, rc=%d\n",
			rc);
		return rc;
	}

	rc = cpr3_mem_acc_init(vreg);
	if (rc) {
		if (rc != -EPROBE_DEFER)
			cpr3_err(vreg, "unable to initialize mem-acc regulator settings, rc=%d\n",
				 rc);
		return rc;
	}

	rc = cpr4_ipq807x_apss_calculate_open_loop_voltages(vreg);
	if (rc) {
		cpr3_err(vreg, "unable to calculate open-loop voltages, rc=%d\n",
			rc);
		return rc;
	}

	rc = cpr3_limit_open_loop_voltages(vreg);
	if (rc) {
		cpr3_err(vreg, "unable to limit open-loop voltages, rc=%d\n",
			rc);
		return rc;
	}

	cpr3_open_loop_voltage_as_ceiling(vreg);

	rc = cpr3_limit_floor_voltages(vreg);
	if (rc) {
		cpr3_err(vreg, "unable to limit floor voltages, rc=%d\n", rc);
		return rc;
	}

	rc = cpr4_ipq807x_apss_calculate_target_quotients(vreg);
	if (rc) {
		cpr3_err(vreg, "unable to calculate target quotients, rc=%d\n",
			rc);
		return rc;
	}

	rc = cpr4_parse_core_count_temp_voltage_adj(vreg, false);
	if (rc) {
		cpr3_err(vreg, "unable to parse temperature and core count voltage adjustments, rc=%d\n",
			 rc);
		return rc;
	}

	if (vreg->allow_core_count_adj && (vreg->max_core_count <= 0
				   || vreg->max_core_count >
				   IPQ807x_APSS_CPR_SDELTA_CORE_COUNT)) {
		cpr3_err(vreg, "qcom,max-core-count has invalid value = %d\n",
			 vreg->max_core_count);
		return -EINVAL;
	}

	rc = cpr4_apss_parse_boost_properties(vreg);
	if (rc) {
		cpr3_err(vreg, "unable to parse boost adjustments, rc=%d\n",
			 rc);
		return rc;
	}

	cpr4_apss_print_settings(vreg);

	return rc;
}

/**
 * cpr4_apss_init_controller() - perform APSS CPR4 controller specific
 *		initializations
 * @ctrl:		Pointer to the CPR3 controller
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_apss_init_controller(struct cpr3_controller *ctrl)
{
	int rc;

	rc = cpr3_parse_common_ctrl_data(ctrl);
	if (rc) {
		if (rc != -EPROBE_DEFER)
			cpr3_err(ctrl, "unable to parse common controller data, rc=%d\n",
				rc);
		return rc;
	}

	rc = of_property_read_u32(ctrl->dev->of_node,
				  "qcom,cpr-down-error-step-limit",
				  &ctrl->down_error_step_limit);
	if (rc) {
		cpr3_err(ctrl, "error reading qcom,cpr-down-error-step-limit, rc=%d\n",
			rc);
		return rc;
	}

	rc = of_property_read_u32(ctrl->dev->of_node,
				  "qcom,cpr-up-error-step-limit",
				  &ctrl->up_error_step_limit);
	if (rc) {
		cpr3_err(ctrl, "error reading qcom,cpr-up-error-step-limit, rc=%d\n",
			rc);
		return rc;
	}

	/*
	 * Use fixed step quotient if specified otherwise use dynamic
	 * calculated per RO step quotient
	 */
	of_property_read_u32(ctrl->dev->of_node, "qcom,cpr-step-quot-fixed",
			&ctrl->step_quot_fixed);
	ctrl->use_dynamic_step_quot = ctrl->step_quot_fixed ? false : true;

	ctrl->saw_use_unit_mV = of_property_read_bool(ctrl->dev->of_node,
					"qcom,cpr-saw-use-unit-mV");

	of_property_read_u32(ctrl->dev->of_node,
			"qcom,cpr-voltage-settling-time",
			&ctrl->voltage_settling_time);

	if (of_find_property(ctrl->dev->of_node, "vdd-limit-supply", NULL)) {
		ctrl->vdd_limit_regulator =
			devm_regulator_get(ctrl->dev, "vdd-limit");
		if (IS_ERR(ctrl->vdd_limit_regulator)) {
			rc = PTR_ERR(ctrl->vdd_limit_regulator);
			if (rc != -EPROBE_DEFER)
				cpr3_err(ctrl, "unable to request vdd-limit regulator, rc=%d\n",
					 rc);
			return rc;
		}
	}

	rc = cpr3_apm_init(ctrl);
	if (rc) {
		if (rc != -EPROBE_DEFER)
			cpr3_err(ctrl, "unable to initialize APM settings, rc=%d\n",
				rc);
		return rc;
	}

	rc = cpr4_apss_parse_temp_adj_properties(ctrl);
	if (rc) {
		cpr3_err(ctrl, "unable to parse temperature adjustment properties, rc=%d\n",
			 rc);
		return rc;
	}

	ctrl->sensor_count = IPQ807x_APSS_CPR_SENSOR_COUNT;

	/*
	 * APSS only has one thread (0) per controller so the zeroed
	 * array does not need further modification.
	 */
	ctrl->sensor_owner = devm_kcalloc(ctrl->dev, ctrl->sensor_count,
		sizeof(*ctrl->sensor_owner), GFP_KERNEL);
	if (!ctrl->sensor_owner)
		return -ENOMEM;

	ctrl->ctrl_type = CPR_CTRL_TYPE_CPR4;
	ctrl->supports_hw_closed_loop = false;
	ctrl->use_hw_closed_loop = of_property_read_bool(ctrl->dev->of_node,
						"qcom,cpr-hw-closed-loop");
	return 0;
}

static int cpr4_apss_regulator_suspend(struct platform_device *pdev,
				pm_message_t state)
{
	struct cpr3_controller *ctrl = platform_get_drvdata(pdev);

	return cpr3_regulator_suspend(ctrl);
}

static int cpr4_apss_regulator_resume(struct platform_device *pdev)
{
	struct cpr3_controller *ctrl = platform_get_drvdata(pdev);

	return cpr3_regulator_resume(ctrl);
}

static void ipq6018_set_mem_acc(struct regulator_dev *rdev)
{
	struct cpr3_regulator *vreg = rdev_get_drvdata(rdev);

	ipq6018_mem_acc_tcsr[0].ioremap_addr =
		ioremap(ipq6018_mem_acc_tcsr[0].phy_addr, 0x4);
	ipq6018_mem_acc_tcsr[1].ioremap_addr =
		ioremap(ipq6018_mem_acc_tcsr[1].phy_addr, 0x4);

	if ((ipq6018_mem_acc_tcsr[0].ioremap_addr != NULL) &&
			(ipq6018_mem_acc_tcsr[1].ioremap_addr != NULL) &&
			(vreg->current_corner == (vreg->corner_count - CPR3_CORNER_OFFSET))) {

		writel_relaxed(ipq6018_mem_acc_tcsr[0].value,
				ipq6018_mem_acc_tcsr[0].ioremap_addr);
		writel_relaxed(ipq6018_mem_acc_tcsr[1].value,
				ipq6018_mem_acc_tcsr[1].ioremap_addr);
	}
}

static void ipq6018_clr_mem_acc(struct regulator_dev *rdev)
{
	struct cpr3_regulator *vreg = rdev_get_drvdata(rdev);

	if ((ipq6018_mem_acc_tcsr[0].ioremap_addr != NULL) &&
			(ipq6018_mem_acc_tcsr[1].ioremap_addr != NULL) &&
			(vreg->current_corner != vreg->corner_count - CPR3_CORNER_OFFSET)) {
		writel_relaxed(0x0, ipq6018_mem_acc_tcsr[0].ioremap_addr);
		writel_relaxed(0x0, ipq6018_mem_acc_tcsr[1].ioremap_addr);
	}

	iounmap(ipq6018_mem_acc_tcsr[0].ioremap_addr);
	iounmap(ipq6018_mem_acc_tcsr[1].ioremap_addr);
}

static struct cpr4_mem_acc_func ipq6018_mem_acc_funcs = {
	.set_mem_acc = ipq6018_set_mem_acc,
	.clear_mem_acc = ipq6018_clr_mem_acc
};

static const struct cpr4_reg_data ipq807x_cpr_apss = {
	.cpr_valid_fuse_count = IPQ807x_APSS_FUSE_CORNERS,
	.fuse_ref_volt = ipq807x_apss_fuse_ref_volt,
	.fuse_step_volt = IPQ807x_APSS_FUSE_STEP_VOLT,
	.cpr_clk_rate = IPQ807x_APSS_CPR_CLOCK_RATE,
	.boost_fuse_ref_volt= IPQ807x_APSS_BOOST_FUSE_REF_VOLT,
	.boost_ceiling_volt= IPQ807x_APSS_BOOST_CEILING_VOLT,
	.boost_floor_volt= IPQ807x_APSS_BOOST_FLOOR_VOLT,
	.cpr3_fuse_params = &ipq807x_fuse_params,
	.mem_acc_funcs = NULL,
};

static const struct cpr4_reg_data ipq817x_cpr_apss = {
	.cpr_valid_fuse_count = IPQ817x_APPS_FUSE_CORNERS,
	.fuse_ref_volt = ipq807x_apss_fuse_ref_volt,
	.fuse_step_volt = IPQ807x_APSS_FUSE_STEP_VOLT,
	.cpr_clk_rate = IPQ807x_APSS_CPR_CLOCK_RATE,
	.boost_fuse_ref_volt= IPQ807x_APSS_BOOST_FUSE_REF_VOLT,
	.boost_ceiling_volt= IPQ807x_APSS_BOOST_CEILING_VOLT,
	.boost_floor_volt= IPQ807x_APSS_BOOST_FLOOR_VOLT,
	.cpr3_fuse_params = &ipq807x_fuse_params,
	.mem_acc_funcs = NULL,
};

static const struct cpr4_reg_data ipq6018_cpr_apss = {
	.cpr_valid_fuse_count = IPQ6018_APSS_FUSE_CORNERS,
	.fuse_ref_volt = ipq6018_apss_fuse_ref_volt,
	.fuse_step_volt = IPQ6018_APSS_FUSE_STEP_VOLT,
	.cpr_clk_rate = IPQ6018_APSS_CPR_CLOCK_RATE,
	.boost_fuse_ref_volt = IPQ6018_APSS_BOOST_FUSE_REF_VOLT,
	.boost_ceiling_volt = IPQ6018_APSS_BOOST_CEILING_VOLT,
	.boost_floor_volt = IPQ6018_APSS_BOOST_FLOOR_VOLT,
	.cpr3_fuse_params = &ipq6018_fuse_params,
	.mem_acc_funcs = &ipq6018_mem_acc_funcs,
};

static const struct cpr4_reg_data ipq9574_cpr_apss = {
	.cpr_valid_fuse_count = IPQ9574_APSS_FUSE_CORNERS,
	.fuse_ref_volt = ipq9574_apss_fuse_ref_volt,
	.fuse_step_volt = IPQ9574_APSS_FUSE_STEP_VOLT,
	.cpr_clk_rate = IPQ6018_APSS_CPR_CLOCK_RATE,
	.boost_fuse_ref_volt = IPQ6018_APSS_BOOST_FUSE_REF_VOLT,
	.boost_ceiling_volt = IPQ6018_APSS_BOOST_CEILING_VOLT,
	.boost_floor_volt = IPQ6018_APSS_BOOST_FLOOR_VOLT,
	.cpr3_fuse_params = &ipq9574_fuse_params,
	.mem_acc_funcs = NULL,
};

static struct of_device_id cpr4_regulator_match_table[] = {
	{
		.compatible = "qcom,cpr4-ipq807x-apss-regulator",
		.data = &ipq807x_cpr_apss
	},
	{
		.compatible = "qcom,cpr4-ipq817x-apss-regulator",
		.data = &ipq817x_cpr_apss
	},
	{
		.compatible = "qcom,cpr4-ipq6018-apss-regulator",
		.data = &ipq6018_cpr_apss
	},
	{
		.compatible = "qcom,cpr4-ipq9574-apss-regulator",
		.data = &ipq9574_cpr_apss
	},
	{}
};

static int cpr4_apss_regulator_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cpr3_controller *ctrl;
	const struct of_device_id *match;
	struct cpr4_reg_data *cpr_data;
	int i, rc;

	if (!dev->of_node) {
		dev_err(dev, "Device tree node is missing\n");
		return -EINVAL;
	}

	ctrl = devm_kzalloc(dev, sizeof(*ctrl), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;

	match = of_match_device(cpr4_regulator_match_table, &pdev->dev);
	if (!match)
		return -ENODEV;

	cpr_data = (struct cpr4_reg_data *)match->data;
	g_valid_fuse_count = cpr_data->cpr_valid_fuse_count;
	dev_info(dev, "CPR valid fuse count: %d\n", g_valid_fuse_count);
	ctrl->cpr_clock_rate = cpr_data->cpr_clk_rate;

	ctrl->dev = dev;
	/* Set to false later if anything precludes CPR operation. */
	ctrl->cpr_allowed_hw = true;

	rc = of_property_read_string(dev->of_node, "qcom,cpr-ctrl-name",
					&ctrl->name);
	if (rc) {
		cpr3_err(ctrl, "unable to read qcom,cpr-ctrl-name, rc=%d\n",
			rc);
		return rc;
	}

	rc = cpr3_map_fuse_base(ctrl, pdev);
	if (rc) {
		cpr3_err(ctrl, "could not map fuse base address\n");
		return rc;
	}

	rc = cpr3_read_tcsr_setting(ctrl, pdev, IPQ807x_APSS_CPR_TCSR_START,
				    IPQ807x_APSS_CPR_TCSR_END);
	if (rc) {
		cpr3_err(ctrl, "could not read CPR tcsr setting\n");
		return rc;
	}

	rc = cpr3_allocate_threads(ctrl, 0, 0);
	if (rc) {
		cpr3_err(ctrl, "failed to allocate CPR thread array, rc=%d\n",
			rc);
		return rc;
	}

	if (ctrl->thread_count != 1) {
		cpr3_err(ctrl, "expected 1 thread but found %d\n",
			ctrl->thread_count);
		return -EINVAL;
	}

	rc = cpr4_apss_init_controller(ctrl);
	if (rc) {
		if (rc != -EPROBE_DEFER)
			cpr3_err(ctrl, "failed to initialize CPR controller parameters, rc=%d\n",
				rc);
		return rc;
	}

	rc = cpr4_apss_init_thread(&ctrl->thread[0]);
	if (rc) {
		cpr3_err(ctrl, "thread initialization failed, rc=%d\n", rc);
		return rc;
	}

	for (i = 0; i < ctrl->thread[0].vreg_count; i++) {
		ctrl->thread[0].vreg[i].cpr4_regulator_data = cpr_data;
		rc = cpr4_apss_init_regulator(&ctrl->thread[0].vreg[i]);
		if (rc) {
			cpr3_err(&ctrl->thread[0].vreg[i], "regulator initialization failed, rc=%d\n",
				 rc);
			return rc;
		}
	}

	platform_set_drvdata(pdev, ctrl);

	return cpr3_regulator_register(pdev, ctrl);
}

static int cpr4_apss_regulator_remove(struct platform_device *pdev)
{
	struct cpr3_controller *ctrl = platform_get_drvdata(pdev);

	return cpr3_regulator_unregister(ctrl);
}

static struct platform_driver cpr4_apss_regulator_driver = {
	.driver		= {
		.name		= "qcom,cpr4-apss-regulator",
		.of_match_table	= cpr4_regulator_match_table,
		.owner		= THIS_MODULE,
	},
	.probe		= cpr4_apss_regulator_probe,
	.remove		= cpr4_apss_regulator_remove,
	.suspend	= cpr4_apss_regulator_suspend,
	.resume		= cpr4_apss_regulator_resume,
};

static int cpr4_regulator_init(void)
{
	return platform_driver_register(&cpr4_apss_regulator_driver);
}

static void cpr4_regulator_exit(void)
{
	platform_driver_unregister(&cpr4_apss_regulator_driver);
}

MODULE_DESCRIPTION("CPR4 APSS regulator driver");
MODULE_LICENSE("GPL v2");

arch_initcall(cpr4_regulator_init);
module_exit(cpr4_regulator_exit);
