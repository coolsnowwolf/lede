/*
 * Copyright (c) 2015-2017, The Linux Foundation. All rights reserved.
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

/*
 * This file contains utility functions to be used by platform specific CPR3
 * regulator drivers.
 */

#define pr_fmt(fmt) "%s: " fmt, __func__

#include <linux/cpumask.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <soc/qcom/socinfo.h>

#include "cpr3-regulator.h"

#define BYTES_PER_FUSE_ROW		8
#define MAX_FUSE_ROW_BIT		63

#define CPR3_CONSECUTIVE_UP_DOWN_MIN	0
#define CPR3_CONSECUTIVE_UP_DOWN_MAX	15
#define CPR3_UP_DOWN_THRESHOLD_MIN	0
#define CPR3_UP_DOWN_THRESHOLD_MAX	31
#define CPR3_STEP_QUOT_MIN		0
#define CPR3_STEP_QUOT_MAX		63
#define CPR3_IDLE_CLOCKS_MIN		0
#define CPR3_IDLE_CLOCKS_MAX		31

/* This constant has units of uV/mV so 1000 corresponds to 100%. */
#define CPR3_AGING_DERATE_UNITY		1000

/**
 * cpr3_allocate_regulators() - allocate and initialize CPR3 regulators for a
 *		given thread based upon device tree data
 * @thread:		Pointer to the CPR3 thread
 *
 * This function allocates the thread->vreg array based upon the number of
 * device tree regulator subnodes.  It also initializes generic elements of each
 * regulator struct such as name, of_node, and thread.
 *
 * Return: 0 on success, errno on failure
 */
static int cpr3_allocate_regulators(struct cpr3_thread *thread)
{
	struct device_node *node;
	int i, rc;

	thread->vreg_count = 0;

	for_each_available_child_of_node(thread->of_node, node) {
		thread->vreg_count++;
	}

	thread->vreg = devm_kcalloc(thread->ctrl->dev, thread->vreg_count,
			sizeof(*thread->vreg), GFP_KERNEL);
	if (!thread->vreg)
		return -ENOMEM;

	i = 0;
	for_each_available_child_of_node(thread->of_node, node) {
		thread->vreg[i].of_node = node;
		thread->vreg[i].thread = thread;

		rc = of_property_read_string(node, "regulator-name",
						&thread->vreg[i].name);
		if (rc) {
			dev_err(thread->ctrl->dev, "could not find regulator name, rc=%d\n",
				rc);
			return rc;
		}

		i++;
	}

	return 0;
}

/**
 * cpr3_allocate_threads() - allocate and initialize CPR3 threads for a given
 *			     controller based upon device tree data
 * @ctrl:		Pointer to the CPR3 controller
 * @min_thread_id:	Minimum allowed hardware thread ID for this controller
 * @max_thread_id:	Maximum allowed hardware thread ID for this controller
 *
 * This function allocates the ctrl->thread array based upon the number of
 * device tree thread subnodes.  It also initializes generic elements of each
 * thread struct such as thread_id, of_node, ctrl, and vreg array.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_allocate_threads(struct cpr3_controller *ctrl, u32 min_thread_id,
			u32 max_thread_id)
{
	struct device *dev = ctrl->dev;
	struct device_node *thread_node;
	int i, j, rc;

	ctrl->thread_count = 0;

	for_each_available_child_of_node(dev->of_node, thread_node) {
		ctrl->thread_count++;
	}

	ctrl->thread = devm_kcalloc(dev, ctrl->thread_count,
			sizeof(*ctrl->thread), GFP_KERNEL);
	if (!ctrl->thread)
		return -ENOMEM;

	i = 0;
	for_each_available_child_of_node(dev->of_node, thread_node) {
		ctrl->thread[i].of_node = thread_node;
		ctrl->thread[i].ctrl = ctrl;

		rc = of_property_read_u32(thread_node, "qcom,cpr-thread-id",
					  &ctrl->thread[i].thread_id);
		if (rc) {
			dev_err(dev, "could not read DT property qcom,cpr-thread-id, rc=%d\n",
				rc);
			return rc;
		}

		if (ctrl->thread[i].thread_id < min_thread_id ||
				ctrl->thread[i].thread_id > max_thread_id) {
			dev_err(dev, "invalid thread id = %u; not within [%u, %u]\n",
				ctrl->thread[i].thread_id, min_thread_id,
				max_thread_id);
			return -EINVAL;
		}

		/* Verify that the thread ID is unique for all child nodes. */
		for (j = 0; j < i; j++) {
			if (ctrl->thread[j].thread_id
					== ctrl->thread[i].thread_id) {
				dev_err(dev, "duplicate thread id = %u found\n",
					ctrl->thread[i].thread_id);
				return -EINVAL;
			}
		}

		rc = cpr3_allocate_regulators(&ctrl->thread[i]);
		if (rc)
			return rc;

		i++;
	}

	return 0;
}

/**
 * cpr3_map_fuse_base() - ioremap the base address of the fuse region
 * @ctrl:	Pointer to the CPR3 controller
 * @pdev:	Platform device pointer for the CPR3 controller
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_map_fuse_base(struct cpr3_controller *ctrl,
			struct platform_device *pdev)
{
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "fuse_base");
	if (!res || !res->start) {
		dev_err(&pdev->dev, "fuse base address is missing\n");
		return -ENXIO;
	}

	ctrl->fuse_base = devm_ioremap(&pdev->dev, res->start,
						resource_size(res));

	return 0;
}

/**
 * cpr3_read_tcsr_setting - reads the CPR setting bits from TCSR register
 * @ctrl:	Pointer to the CPR3 controller
 * @pdev:	Platform device pointer for the CPR3 controller
 * @start:	start bit in TCSR register
 * @end:	end bit in TCSR register
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_read_tcsr_setting(struct cpr3_controller *ctrl,
			   struct platform_device *pdev, u8 start, u8 end)
{
	struct resource *res;
	void __iomem *tcsr_reg;
	u32 val;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
					   "cpr_tcsr_reg");
	if (!res || !res->start)
		return 0;

	tcsr_reg = ioremap(res->start, resource_size(res));
	if (!tcsr_reg) {
		dev_err(&pdev->dev, "tcsr ioremap failed\n");
		return 0;
	}

	val = readl_relaxed(tcsr_reg);
	val &= GENMASK(end, start);
	val >>= start;

	switch (val) {
	case 1:
		ctrl->cpr_global_setting = CPR_DISABLED;
		break;
	case 2:
		ctrl->cpr_global_setting = CPR_OPEN_LOOP_EN;
		break;
	case 3:
		ctrl->cpr_global_setting = CPR_CLOSED_LOOP_EN;
		break;
	default:
		ctrl->cpr_global_setting = CPR_DEFAULT;
	}

	iounmap(tcsr_reg);

	return 0;
}

/**
 * cpr3_read_fuse_param() - reads a CPR3 fuse parameter out of eFuses
 * @fuse_base_addr:	Virtual memory address of the eFuse base address
 * @param:		Null terminated array of fuse param segments to read
 *			from
 * @param_value:	Output with value read from the eFuses
 *
 * This function reads from each of the parameter segments listed in the param
 * array and concatenates their values together.  Reading stops when an element
 * is reached which has all 0 struct values.  The total number of bits specified
 * for the fuse parameter across all segments must be less than or equal to 64.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_read_fuse_param(void __iomem *fuse_base_addr,
		const struct cpr3_fuse_param *param, u64 *param_value)
{
	u64 fuse_val, val;
	int bits;
	int bits_total = 0;

	*param_value = 0;

	while (param->row || param->bit_start || param->bit_end) {
		if (param->bit_start > param->bit_end
		    || param->bit_end > MAX_FUSE_ROW_BIT) {
			pr_err("Invalid fuse parameter segment: row=%u, start=%u, end=%u\n",
				param->row, param->bit_start, param->bit_end);
			return -EINVAL;
		}

		bits = param->bit_end - param->bit_start + 1;
		if (bits_total + bits > 64) {
			pr_err("Invalid fuse parameter segments; total bits = %d\n",
				bits_total + bits);
			return -EINVAL;
		}

		fuse_val = readq_relaxed(fuse_base_addr
					 + param->row * BYTES_PER_FUSE_ROW);
		val = (fuse_val >> param->bit_start) & ((1ULL << bits) - 1);
		*param_value |= val << bits_total;
		bits_total += bits;

		param++;
	}

	return 0;
}

/**
 * cpr3_convert_open_loop_voltage_fuse() - converts an open loop voltage fuse
 *		value into an absolute voltage with units of microvolts
 * @ref_volt:		Reference voltage in microvolts
 * @step_volt:		The step size in microvolts of the fuse LSB
 * @fuse:		Open loop voltage fuse value
 * @fuse_len:		The bit length of the fuse value
 *
 * The MSB of the fuse parameter corresponds to a sign bit.  If it is set, then
 * the lower bits correspond to the number of steps to go down from the
 * reference voltage.  If it is not set, then the lower bits correspond to the
 * number of steps to go up from the reference voltage.
 */
int cpr3_convert_open_loop_voltage_fuse(int ref_volt, int step_volt, u32 fuse,
					int fuse_len)
{
	int sign, steps;

	sign = (fuse & (1 << (fuse_len - 1))) ? -1 : 1;
	steps = fuse & ((1 << (fuse_len - 1)) - 1);

	return ref_volt + sign * steps * step_volt;
}

/**
 * cpr3_interpolate() - performs linear interpolation
 * @x1		Lower known x value
 * @y1		Lower known y value
 * @x2		Upper known x value
 * @y2		Upper known y value
 * @x		Intermediate x value
 *
 * Returns y where (x, y) falls on the line between (x1, y1) and (x2, y2).
 * It is required that x1 < x2, y1 <= y2, and x1 <= x <= x2.  If these
 * conditions are not met, then y2 will be returned.
 */
u64 cpr3_interpolate(u64 x1, u64 y1, u64 x2, u64 y2, u64 x)
{
	u64 temp;

	if (x1 >= x2 || y1 > y2 || x1 > x || x > x2)
		return y2;

	temp = (x2 - x) * (y2 - y1);
	do_div(temp, (u32)(x2 - x1));

	return y2 - temp;
}

/**
 * cpr3_parse_array_property() - fill an array from a portion of the values
 *		specified for a device tree property
 * @vreg:		Pointer to the CPR3 regulator
 * @prop_name:		The name of the device tree property to read from
 * @tuple_size:		The number of elements in each tuple
 * @out:		Output data array which must be of size tuple_size
 *
 * cpr3_parse_common_corner_data() must be called for vreg before this function
 * is called so that fuse combo and speed bin size elements are initialized.
 *
 * Three formats are supported for the device tree property:
 * 1. Length == tuple_size
 *	(reading begins at index 0)
 * 2. Length == tuple_size * vreg->fuse_combos_supported
 *	(reading begins at index tuple_size * vreg->fuse_combo)
 * 3. Length == tuple_size * vreg->speed_bins_supported
 *	(reading begins at index tuple_size * vreg->speed_bin_fuse)
 *
 * All other property lengths are treated as errors.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_array_property(struct cpr3_regulator *vreg,
		const char *prop_name, int tuple_size, u32 *out)
{
	struct device_node *node = vreg->of_node;
	int len = 0;
	int i, offset, rc;

	if (!of_find_property(node, prop_name, &len)) {
		cpr3_err(vreg, "property %s is missing\n", prop_name);
		return -EINVAL;
	}

	if (len == tuple_size * sizeof(u32)) {
		offset = 0;
	} else if (len == tuple_size * vreg->fuse_combos_supported
				     * sizeof(u32)) {
		offset = tuple_size * vreg->fuse_combo;
	} else if (vreg->speed_bins_supported > 0 &&
		 len == tuple_size * vreg->speed_bins_supported * sizeof(u32)) {
		offset = tuple_size * vreg->speed_bin_fuse;
	} else {
		if (vreg->speed_bins_supported > 0)
			cpr3_err(vreg, "property %s has invalid length=%d, should be %zu, %zu, or %zu\n",
				prop_name, len,
				tuple_size * sizeof(u32),
				tuple_size * vreg->speed_bins_supported
					   * sizeof(u32),
				tuple_size * vreg->fuse_combos_supported
					   * sizeof(u32));
		else
			cpr3_err(vreg, "property %s has invalid length=%d, should be %zu or %zu\n",
				prop_name, len,
				tuple_size * sizeof(u32),
				tuple_size * vreg->fuse_combos_supported
					   * sizeof(u32));
		return -EINVAL;
	}

	for (i = 0; i < tuple_size; i++) {
		rc = of_property_read_u32_index(node, prop_name, offset + i,
						&out[i]);
		if (rc) {
			cpr3_err(vreg, "error reading property %s, rc=%d\n",
				prop_name, rc);
			return rc;
		}
	}

	return 0;
}

/**
 * cpr3_parse_corner_array_property() - fill a per-corner array from a portion
 *		of the values specified for a device tree property
 * @vreg:		Pointer to the CPR3 regulator
 * @prop_name:		The name of the device tree property to read from
 * @tuple_size:		The number of elements in each per-corner tuple
 * @out:		Output data array which must be of size:
 *			tuple_size * vreg->corner_count
 *
 * cpr3_parse_common_corner_data() must be called for vreg before this function
 * is called so that fuse combo and speed bin size elements are initialized.
 *
 * Three formats are supported for the device tree property:
 * 1. Length == tuple_size * vreg->corner_count
 *	(reading begins at index 0)
 * 2. Length == tuple_size * vreg->fuse_combo_corner_sum
 *	(reading begins at index tuple_size * vreg->fuse_combo_offset)
 * 3. Length == tuple_size * vreg->speed_bin_corner_sum
 *	(reading begins at index tuple_size * vreg->speed_bin_offset)
 *
 * All other property lengths are treated as errors.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_corner_array_property(struct cpr3_regulator *vreg,
		const char *prop_name, int tuple_size, u32 *out)
{
	struct device_node *node = vreg->of_node;
	int len = 0;
	int i, offset, rc;

	if (!of_find_property(node, prop_name, &len)) {
		cpr3_err(vreg, "property %s is missing\n", prop_name);
		return -EINVAL;
	}

	if (len == tuple_size * vreg->corner_count * sizeof(u32)) {
		offset = 0;
	} else if (len == tuple_size * vreg->fuse_combo_corner_sum
				     * sizeof(u32)) {
		offset = tuple_size * vreg->fuse_combo_offset;
	} else if (vreg->speed_bin_corner_sum > 0 &&
		 len == tuple_size * vreg->speed_bin_corner_sum * sizeof(u32)) {
		offset = tuple_size * vreg->speed_bin_offset;
	} else {
		if (vreg->speed_bin_corner_sum > 0)
			cpr3_err(vreg, "property %s has invalid length=%d, should be %zu, %zu, or %zu\n",
				prop_name, len,
				tuple_size * vreg->corner_count * sizeof(u32),
				tuple_size * vreg->speed_bin_corner_sum
					   * sizeof(u32),
				tuple_size * vreg->fuse_combo_corner_sum
					   * sizeof(u32));
		else
			cpr3_err(vreg, "property %s has invalid length=%d, should be %zu or %zu\n",
				prop_name, len,
				tuple_size * vreg->corner_count * sizeof(u32),
				tuple_size * vreg->fuse_combo_corner_sum
					   * sizeof(u32));
		return -EINVAL;
	}

	for (i = 0; i < tuple_size * vreg->corner_count; i++) {
		rc = of_property_read_u32_index(node, prop_name, offset + i,
						&out[i]);
		if (rc) {
			cpr3_err(vreg, "error reading property %s, rc=%d\n",
				prop_name, rc);
			return rc;
		}
	}

	return 0;
}

/**
 * cpr3_parse_corner_band_array_property() - fill a per-corner band array
 *		from a portion of the values specified for a device tree
 *		property
 * @vreg:		Pointer to the CPR3 regulator
 * @prop_name:		The name of the device tree property to read from
 * @tuple_size:		The number of elements in each per-corner band tuple
 * @out:		Output data array which must be of size:
 *			tuple_size * vreg->corner_band_count
 *
 * cpr3_parse_common_corner_data() must be called for vreg before this function
 * is called so that fuse combo and speed bin size elements are initialized.
 * In addition, corner band fuse combo and speed bin sum and offset elements
 * must be initialized prior to executing this function.
 *
 * Three formats are supported for the device tree property:
 * 1. Length == tuple_size * vreg->corner_band_count
 *	(reading begins at index 0)
 * 2. Length == tuple_size * vreg->fuse_combo_corner_band_sum
 *	(reading begins at index tuple_size *
 *		vreg->fuse_combo_corner_band_offset)
 * 3. Length == tuple_size * vreg->speed_bin_corner_band_sum
 *	(reading begins at index tuple_size *
 *		vreg->speed_bin_corner_band_offset)
 *
 * All other property lengths are treated as errors.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_corner_band_array_property(struct cpr3_regulator *vreg,
		const char *prop_name, int tuple_size, u32 *out)
{
	struct device_node *node = vreg->of_node;
	int len = 0;
	int i, offset, rc;

	if (!of_find_property(node, prop_name, &len)) {
		cpr3_err(vreg, "property %s is missing\n", prop_name);
		return -EINVAL;
	}

	if (len == tuple_size * vreg->corner_band_count * sizeof(u32)) {
		offset = 0;
	} else if (len == tuple_size * vreg->fuse_combo_corner_band_sum
				     * sizeof(u32)) {
		offset = tuple_size * vreg->fuse_combo_corner_band_offset;
	} else if (vreg->speed_bin_corner_band_sum > 0 &&
		 len == tuple_size * vreg->speed_bin_corner_band_sum *
		   sizeof(u32)) {
		offset = tuple_size * vreg->speed_bin_corner_band_offset;
	} else {
		if (vreg->speed_bin_corner_band_sum > 0)
			cpr3_err(vreg, "property %s has invalid length=%d, should be %zu, %zu, or %zu\n",
				prop_name, len,
				tuple_size * vreg->corner_band_count *
				 sizeof(u32),
				tuple_size * vreg->speed_bin_corner_band_sum
					   * sizeof(u32),
				tuple_size * vreg->fuse_combo_corner_band_sum
					   * sizeof(u32));
		else
			cpr3_err(vreg, "property %s has invalid length=%d, should be %zu or %zu\n",
				prop_name, len,
				tuple_size * vreg->corner_band_count *
				 sizeof(u32),
				tuple_size * vreg->fuse_combo_corner_band_sum
					   * sizeof(u32));
		return -EINVAL;
	}

	for (i = 0; i < tuple_size * vreg->corner_band_count; i++) {
		rc = of_property_read_u32_index(node, prop_name, offset + i,
						&out[i]);
		if (rc) {
			cpr3_err(vreg, "error reading property %s, rc=%d\n",
				prop_name, rc);
			return rc;
		}
	}

	return 0;
}

/**
 * cpr3_parse_common_corner_data() - parse common CPR3 properties relating to
 *		the corners supported by a CPR3 regulator from device tree
 * @vreg:		Pointer to the CPR3 regulator
 *
 * This function reads, validates, and utilizes the following device tree
 * properties: qcom,cpr-fuse-corners, qcom,cpr-fuse-combos, qcom,cpr-speed-bins,
 * qcom,cpr-speed-bin-corners, qcom,cpr-corners, qcom,cpr-voltage-ceiling,
 * qcom,cpr-voltage-floor, qcom,corner-frequencies,
 * and qcom,cpr-corner-fmax-map.
 *
 * It initializes these CPR3 regulator elements: corner, corner_count,
 * fuse_combos_supported, fuse_corner_map, and speed_bins_supported.  It
 * initializes these elements for each corner: ceiling_volt, floor_volt,
 * proc_freq, and cpr_fuse_corner.
 *
 * It requires that the following CPR3 regulator elements be initialized before
 * being called: fuse_corner_count, fuse_combo, and speed_bin_fuse.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_common_corner_data(struct cpr3_regulator *vreg)
{
	struct device_node *node = vreg->of_node;
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	u32 max_fuse_combos, fuse_corners, aging_allowed = 0;
	u32 max_speed_bins = 0;
	u32 *combo_corners;
	u32 *speed_bin_corners;
	u32 *temp;
	int i, j, rc;

	rc = of_property_read_u32(node, "qcom,cpr-fuse-corners", &fuse_corners);
	if (rc) {
		cpr3_err(vreg, "error reading property qcom,cpr-fuse-corners, rc=%d\n",
			rc);
		return rc;
	}

	if (vreg->fuse_corner_count != fuse_corners) {
		cpr3_err(vreg, "device tree config supports %d fuse corners but the hardware has %d fuse corners\n",
			fuse_corners, vreg->fuse_corner_count);
		return -EINVAL;
	}

	rc = of_property_read_u32(node, "qcom,cpr-fuse-combos",
				&max_fuse_combos);
	if (rc) {
		cpr3_err(vreg, "error reading property qcom,cpr-fuse-combos, rc=%d\n",
			rc);
		return rc;
	}

	/*
	 * Sanity check against arbitrarily large value to avoid excessive
	 * memory allocation.
	 */
	if (max_fuse_combos > 100 || max_fuse_combos == 0) {
		cpr3_err(vreg, "qcom,cpr-fuse-combos is invalid: %u\n",
			max_fuse_combos);
		return -EINVAL;
	}

	if (vreg->fuse_combo >= max_fuse_combos) {
		cpr3_err(vreg, "device tree config supports fuse combos 0-%u but the hardware has combo %d\n",
			max_fuse_combos - 1, vreg->fuse_combo);
		BUG_ON(1);
		return -EINVAL;
	}

	vreg->fuse_combos_supported = max_fuse_combos;

	of_property_read_u32(node, "qcom,cpr-speed-bins", &max_speed_bins);

	/*
	 * Sanity check against arbitrarily large value to avoid excessive
	 * memory allocation.
	 */
	if (max_speed_bins > 100) {
		cpr3_err(vreg, "qcom,cpr-speed-bins is invalid: %u\n",
			max_speed_bins);
		return -EINVAL;
	}

	if (max_speed_bins && vreg->speed_bin_fuse >= max_speed_bins) {
		cpr3_err(vreg, "device tree config supports speed bins 0-%u but the hardware has speed bin %d\n",
			max_speed_bins - 1, vreg->speed_bin_fuse);
		BUG();
		return -EINVAL;
	}

	vreg->speed_bins_supported = max_speed_bins;

	combo_corners = kcalloc(vreg->fuse_combos_supported,
				sizeof(*combo_corners), GFP_KERNEL);
	if (!combo_corners)
		return -ENOMEM;

	rc = of_property_read_u32_array(node, "qcom,cpr-corners", combo_corners,
					vreg->fuse_combos_supported);
	if (rc == -EOVERFLOW) {
		/* Single value case */
		rc = of_property_read_u32(node, "qcom,cpr-corners",
					combo_corners);
		for (i = 1; i < vreg->fuse_combos_supported; i++)
			combo_corners[i] = combo_corners[0];
	}
	if (rc) {
		cpr3_err(vreg, "error reading property qcom,cpr-corners, rc=%d\n",
			rc);
		kfree(combo_corners);
		return rc;
	}

	vreg->fuse_combo_offset = 0;
	vreg->fuse_combo_corner_sum = 0;
	for (i = 0; i < vreg->fuse_combos_supported; i++) {
		vreg->fuse_combo_corner_sum += combo_corners[i];
		if (i < vreg->fuse_combo)
			vreg->fuse_combo_offset += combo_corners[i];
	}

	vreg->corner_count = combo_corners[vreg->fuse_combo];

	kfree(combo_corners);

	vreg->speed_bin_offset = 0;
	vreg->speed_bin_corner_sum = 0;
	if (vreg->speed_bins_supported > 0) {
		speed_bin_corners = kcalloc(vreg->speed_bins_supported,
					sizeof(*speed_bin_corners), GFP_KERNEL);
		if (!speed_bin_corners)
			return -ENOMEM;

		rc = of_property_read_u32_array(node,
				"qcom,cpr-speed-bin-corners", speed_bin_corners,
				vreg->speed_bins_supported);
		if (rc) {
			cpr3_err(vreg, "error reading property qcom,cpr-speed-bin-corners, rc=%d\n",
				rc);
			kfree(speed_bin_corners);
			return rc;
		}

		for (i = 0; i < vreg->speed_bins_supported; i++) {
			vreg->speed_bin_corner_sum += speed_bin_corners[i];
			if (i < vreg->speed_bin_fuse)
				vreg->speed_bin_offset += speed_bin_corners[i];
		}

		if (speed_bin_corners[vreg->speed_bin_fuse]
		    != vreg->corner_count) {
			cpr3_err(vreg, "qcom,cpr-corners and qcom,cpr-speed-bin-corners conflict on number of corners: %d vs %u\n",
				vreg->corner_count,
				speed_bin_corners[vreg->speed_bin_fuse]);
			kfree(speed_bin_corners);
			return -EINVAL;
		}

		kfree(speed_bin_corners);
	}

	vreg->corner = devm_kcalloc(ctrl->dev, vreg->corner_count,
				    sizeof(*vreg->corner), GFP_KERNEL);
	temp = kcalloc(vreg->corner_count, sizeof(*temp), GFP_KERNEL);
	if (!vreg->corner || !temp)
		return -ENOMEM;

	rc = cpr3_parse_corner_array_property(vreg, "qcom,cpr-voltage-ceiling",
			1, temp);
	if (rc)
		goto free_temp;
	for (i = 0; i < vreg->corner_count; i++) {
		vreg->corner[i].ceiling_volt
			= CPR3_ROUND(temp[i], ctrl->step_volt);
		vreg->corner[i].abs_ceiling_volt = vreg->corner[i].ceiling_volt;
	}

	rc = cpr3_parse_corner_array_property(vreg, "qcom,cpr-voltage-floor",
			1, temp);
	if (rc)
		goto free_temp;
	for (i = 0; i < vreg->corner_count; i++)
		vreg->corner[i].floor_volt
			= CPR3_ROUND(temp[i], ctrl->step_volt);

	/* Validate ceiling and floor values */
	for (i = 0; i < vreg->corner_count; i++) {
		if (vreg->corner[i].floor_volt
		    > vreg->corner[i].ceiling_volt) {
			cpr3_err(vreg, "CPR floor[%d]=%d > ceiling[%d]=%d uV\n",
				i, vreg->corner[i].floor_volt,
				i, vreg->corner[i].ceiling_volt);
			rc = -EINVAL;
			goto free_temp;
		}
	}

	/* Load optional system-supply voltages */
	if (of_find_property(vreg->of_node, "qcom,system-voltage", NULL)) {
		rc = cpr3_parse_corner_array_property(vreg,
			"qcom,system-voltage", 1, temp);
		if (rc)
			goto free_temp;
		for (i = 0; i < vreg->corner_count; i++)
			vreg->corner[i].system_volt = temp[i];
	}

	rc = cpr3_parse_corner_array_property(vreg, "qcom,corner-frequencies",
			1, temp);
	if (rc)
		goto free_temp;
	for (i = 0; i < vreg->corner_count; i++)
		vreg->corner[i].proc_freq = temp[i];

	/* Validate frequencies */
	for (i = 1; i < vreg->corner_count; i++) {
		if (vreg->corner[i].proc_freq
		    < vreg->corner[i - 1].proc_freq) {
			cpr3_err(vreg, "invalid frequency: freq[%d]=%u < freq[%d]=%u\n",
				i, vreg->corner[i].proc_freq, i - 1,
				vreg->corner[i - 1].proc_freq);
			rc = -EINVAL;
			goto free_temp;
		}
	}

	vreg->fuse_corner_map = devm_kcalloc(ctrl->dev, vreg->fuse_corner_count,
				    sizeof(*vreg->fuse_corner_map), GFP_KERNEL);
	if (!vreg->fuse_corner_map) {
		rc = -ENOMEM;
		goto free_temp;
	}

	rc = cpr3_parse_array_property(vreg, "qcom,cpr-corner-fmax-map",
		vreg->fuse_corner_count, temp);
	if (rc)
		goto free_temp;
	for (i = 0; i < vreg->fuse_corner_count; i++) {
		vreg->fuse_corner_map[i] = temp[i] - CPR3_CORNER_OFFSET;
		if (temp[i] < CPR3_CORNER_OFFSET
		    || temp[i] > vreg->corner_count + CPR3_CORNER_OFFSET) {
			cpr3_err(vreg, "invalid corner value specified in qcom,cpr-corner-fmax-map: %u\n",
				temp[i]);
			rc = -EINVAL;
			goto free_temp;
		} else if (i > 0 && temp[i - 1] >= temp[i]) {
			cpr3_err(vreg, "invalid corner %u less than or equal to previous corner %u\n",
				temp[i], temp[i - 1]);
			rc = -EINVAL;
			goto free_temp;
		}
	}
	if (temp[vreg->fuse_corner_count - 1] != vreg->corner_count)
		cpr3_debug(vreg, "Note: highest Fmax corner %u in qcom,cpr-corner-fmax-map does not match highest supported corner %d\n",
			temp[vreg->fuse_corner_count - 1],
			vreg->corner_count);

	for (i = 0; i < vreg->corner_count; i++) {
		for (j = 0; j < vreg->fuse_corner_count; j++) {
			if (i + CPR3_CORNER_OFFSET <= temp[j]) {
				vreg->corner[i].cpr_fuse_corner = j;
				break;
			}
		}
		if (j == vreg->fuse_corner_count) {
			/*
			 * Handle the case where the highest fuse corner maps
			 * to a corner below the highest corner.
			 */
			vreg->corner[i].cpr_fuse_corner
				= vreg->fuse_corner_count - 1;
		}
	}

	if (of_find_property(vreg->of_node,
				"qcom,allow-aging-voltage-adjustment", NULL)) {
		rc = cpr3_parse_array_property(vreg,
			"qcom,allow-aging-voltage-adjustment",
			1, &aging_allowed);
		if (rc)
			goto free_temp;

		vreg->aging_allowed = aging_allowed;
	}

	if (of_find_property(vreg->of_node,
		       "qcom,allow-aging-open-loop-voltage-adjustment", NULL)) {
		rc = cpr3_parse_array_property(vreg,
			"qcom,allow-aging-open-loop-voltage-adjustment",
			1, &aging_allowed);
		if (rc)
			goto free_temp;

		vreg->aging_allow_open_loop_adj = aging_allowed;
	}

	if (vreg->aging_allowed) {
		if (ctrl->aging_ref_volt <= 0) {
			cpr3_err(ctrl, "qcom,cpr-aging-ref-voltage must be specified\n");
			rc = -EINVAL;
			goto free_temp;
		}

		rc = cpr3_parse_array_property(vreg,
			"qcom,cpr-aging-max-voltage-adjustment",
			1, &vreg->aging_max_adjust_volt);
		if (rc)
			goto free_temp;

		rc = cpr3_parse_array_property(vreg,
			"qcom,cpr-aging-ref-corner", 1, &vreg->aging_corner);
		if (rc) {
			goto free_temp;
		} else if (vreg->aging_corner < CPR3_CORNER_OFFSET
			   || vreg->aging_corner > vreg->corner_count - 1
							+ CPR3_CORNER_OFFSET) {
			cpr3_err(vreg, "aging reference corner=%d not in range [%d, %d]\n",
				vreg->aging_corner, CPR3_CORNER_OFFSET,
				vreg->corner_count - 1 + CPR3_CORNER_OFFSET);
			rc = -EINVAL;
			goto free_temp;
		}
		vreg->aging_corner -= CPR3_CORNER_OFFSET;

		if (of_find_property(vreg->of_node, "qcom,cpr-aging-derate",
					NULL)) {
			rc = cpr3_parse_corner_array_property(vreg,
				"qcom,cpr-aging-derate", 1, temp);
			if (rc)
				goto free_temp;

			for (i = 0; i < vreg->corner_count; i++)
				vreg->corner[i].aging_derate = temp[i];
		} else {
			for (i = 0; i < vreg->corner_count; i++)
				vreg->corner[i].aging_derate
					= CPR3_AGING_DERATE_UNITY;
		}
	}

free_temp:
	kfree(temp);
	return rc;
}

/**
 * cpr3_parse_thread_u32() - parse the specified property from the CPR3 thread's
 *		device tree node and verify that it is within the allowed limits
 * @thread:		Pointer to the CPR3 thread
 * @propname:		The name of the device tree property to read
 * @out_value:		The output pointer to fill with the value read
 * @value_min:		The minimum allowed property value
 * @value_max:		The maximum allowed property value
 *
 * This function prints a verbose error message if the property is missing or
 * has a value which is not within the specified range.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_thread_u32(struct cpr3_thread *thread, const char *propname,
		       u32 *out_value, u32 value_min, u32 value_max)
{
	int rc;

	rc = of_property_read_u32(thread->of_node, propname, out_value);
	if (rc) {
		cpr3_err(thread->ctrl, "thread %u error reading property %s, rc=%d\n",
			thread->thread_id, propname, rc);
		return rc;
	}

	if (*out_value < value_min || *out_value > value_max) {
		cpr3_err(thread->ctrl, "thread %u %s=%u is invalid; allowed range: [%u, %u]\n",
			thread->thread_id, propname, *out_value, value_min,
			value_max);
		return -EINVAL;
	}

	return 0;
}

/**
 * cpr3_parse_ctrl_u32() - parse the specified property from the CPR3
 *		controller's device tree node and verify that it is within the
 *		allowed limits
 * @ctrl:		Pointer to the CPR3 controller
 * @propname:		The name of the device tree property to read
 * @out_value:		The output pointer to fill with the value read
 * @value_min:		The minimum allowed property value
 * @value_max:		The maximum allowed property value
 *
 * This function prints a verbose error message if the property is missing or
 * has a value which is not within the specified range.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_ctrl_u32(struct cpr3_controller *ctrl, const char *propname,
		       u32 *out_value, u32 value_min, u32 value_max)
{
	int rc;

	rc = of_property_read_u32(ctrl->dev->of_node, propname, out_value);
	if (rc) {
		cpr3_err(ctrl, "error reading property %s, rc=%d\n",
			propname, rc);
		return rc;
	}

	if (*out_value < value_min || *out_value > value_max) {
		cpr3_err(ctrl, "%s=%u is invalid; allowed range: [%u, %u]\n",
			propname, *out_value, value_min, value_max);
		return -EINVAL;
	}

	return 0;
}

/**
 * cpr3_parse_common_thread_data() - parse common CPR3 thread properties from
 *		device tree
 * @thread:		Pointer to the CPR3 thread
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_common_thread_data(struct cpr3_thread *thread)
{
	int rc;

	rc = cpr3_parse_thread_u32(thread, "qcom,cpr-consecutive-up",
			&thread->consecutive_up, CPR3_CONSECUTIVE_UP_DOWN_MIN,
			CPR3_CONSECUTIVE_UP_DOWN_MAX);
	if (rc)
		return rc;

	rc = cpr3_parse_thread_u32(thread, "qcom,cpr-consecutive-down",
			&thread->consecutive_down, CPR3_CONSECUTIVE_UP_DOWN_MIN,
			CPR3_CONSECUTIVE_UP_DOWN_MAX);
	if (rc)
		return rc;

	rc = cpr3_parse_thread_u32(thread, "qcom,cpr-up-threshold",
			&thread->up_threshold, CPR3_UP_DOWN_THRESHOLD_MIN,
			CPR3_UP_DOWN_THRESHOLD_MAX);
	if (rc)
		return rc;

	rc = cpr3_parse_thread_u32(thread, "qcom,cpr-down-threshold",
			&thread->down_threshold, CPR3_UP_DOWN_THRESHOLD_MIN,
			CPR3_UP_DOWN_THRESHOLD_MAX);
	if (rc)
		return rc;

	return rc;
}

/**
 * cpr3_parse_irq_affinity() - parse CPR IRQ affinity information
 * @ctrl:		Pointer to the CPR3 controller
 *
 * Return: 0 on success, errno on failure
 */
static int cpr3_parse_irq_affinity(struct cpr3_controller *ctrl)
{
	struct device_node *cpu_node;
	int i, cpu;
	int len = 0;

	if (!of_find_property(ctrl->dev->of_node, "qcom,cpr-interrupt-affinity",
				&len)) {
		/* No IRQ affinity required */
		return 0;
	}

	len /= sizeof(u32);

	for (i = 0; i < len; i++) {
		cpu_node = of_parse_phandle(ctrl->dev->of_node,
					    "qcom,cpr-interrupt-affinity", i);
		if (!cpu_node) {
			cpr3_err(ctrl, "could not find CPU node %d\n", i);
			return -EINVAL;
		}

		for_each_possible_cpu(cpu) {
			if (of_get_cpu_node(cpu, NULL) == cpu_node) {
				cpumask_set_cpu(cpu, &ctrl->irq_affinity_mask);
				break;
			}
		}
		of_node_put(cpu_node);
	}

	return 0;
}

static int cpr3_panic_notifier_init(struct cpr3_controller *ctrl)
{
	struct device_node *node = ctrl->dev->of_node;
	struct cpr3_panic_regs_info *panic_regs_info;
	struct cpr3_reg_info *regs;
	int i, reg_count, len, rc = 0;

	if (!of_find_property(node, "qcom,cpr-panic-reg-addr-list", &len)) {
		/* panic register address list not specified */
		return rc;
	}

	reg_count = len / sizeof(u32);
	if (!reg_count) {
		cpr3_err(ctrl, "qcom,cpr-panic-reg-addr-list has invalid len = %d\n",
			len);
		return -EINVAL;
	}

	if (!of_find_property(node, "qcom,cpr-panic-reg-name-list", NULL)) {
		cpr3_err(ctrl, "property qcom,cpr-panic-reg-name-list not specified\n");
		return -EINVAL;
	}

	len = of_property_count_strings(node, "qcom,cpr-panic-reg-name-list");
	if (reg_count != len) {
		cpr3_err(ctrl, "qcom,cpr-panic-reg-name-list should have %d strings\n",
			reg_count);
		return -EINVAL;
	}

	panic_regs_info = devm_kzalloc(ctrl->dev, sizeof(*panic_regs_info),
					GFP_KERNEL);
	if (!panic_regs_info)
		return -ENOMEM;

	regs = devm_kcalloc(ctrl->dev, reg_count, sizeof(*regs), GFP_KERNEL);
	if (!regs)
		return -ENOMEM;

	for (i = 0; i < reg_count; i++) {
		rc = of_property_read_string_index(node,
				"qcom,cpr-panic-reg-name-list", i,
				&(regs[i].name));
		if (rc) {
			cpr3_err(ctrl, "error reading property qcom,cpr-panic-reg-name-list, rc=%d\n",
				rc);
			return rc;
		}

		rc = of_property_read_u32_index(node,
				"qcom,cpr-panic-reg-addr-list", i,
				&(regs[i].addr));
		if (rc) {
			cpr3_err(ctrl, "error reading property qcom,cpr-panic-reg-addr-list, rc=%d\n",
				rc);
			return rc;
		}
		regs[i].virt_addr = devm_ioremap(ctrl->dev, regs[i].addr, 0x4);
		if (!regs[i].virt_addr) {
			pr_err("Unable to map panic register addr 0x%08x\n",
				regs[i].addr);
			return -EINVAL;
		}
		regs[i].value = 0xFFFFFFFF;
	}

	panic_regs_info->reg_count = reg_count;
	panic_regs_info->regs = regs;
	ctrl->panic_regs_info = panic_regs_info;

	return rc;
}

/**
 * cpr3_parse_common_ctrl_data() - parse common CPR3 controller properties from
 *		device tree
 * @ctrl:		Pointer to the CPR3 controller
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_common_ctrl_data(struct cpr3_controller *ctrl)
{
	int rc;

	rc = cpr3_parse_ctrl_u32(ctrl, "qcom,cpr-sensor-time",
			&ctrl->sensor_time, 0, UINT_MAX);
	if (rc)
		return rc;

	rc = cpr3_parse_ctrl_u32(ctrl, "qcom,cpr-loop-time",
			&ctrl->loop_time, 0, UINT_MAX);
	if (rc)
		return rc;

	rc = cpr3_parse_ctrl_u32(ctrl, "qcom,cpr-idle-cycles",
			&ctrl->idle_clocks, CPR3_IDLE_CLOCKS_MIN,
			CPR3_IDLE_CLOCKS_MAX);
	if (rc)
		return rc;

	rc = cpr3_parse_ctrl_u32(ctrl, "qcom,cpr-step-quot-init-min",
			&ctrl->step_quot_init_min, CPR3_STEP_QUOT_MIN,
			CPR3_STEP_QUOT_MAX);
	if (rc)
		return rc;

	rc = cpr3_parse_ctrl_u32(ctrl, "qcom,cpr-step-quot-init-max",
			&ctrl->step_quot_init_max, CPR3_STEP_QUOT_MIN,
			CPR3_STEP_QUOT_MAX);
	if (rc)
		return rc;

	rc = of_property_read_u32(ctrl->dev->of_node, "qcom,voltage-step",
				&ctrl->step_volt);
	if (rc) {
		cpr3_err(ctrl, "error reading property qcom,voltage-step, rc=%d\n",
			rc);
		return rc;
	}
	if (ctrl->step_volt <= 0) {
		cpr3_err(ctrl, "qcom,voltage-step=%d is invalid\n",
			ctrl->step_volt);
		return -EINVAL;
	}

	rc = cpr3_parse_ctrl_u32(ctrl, "qcom,cpr-count-mode",
			&ctrl->count_mode, CPR3_COUNT_MODE_ALL_AT_ONCE_MIN,
			CPR3_COUNT_MODE_STAGGERED);
	if (rc)
		return rc;

	/* Count repeat is optional */
	ctrl->count_repeat = 0;
	of_property_read_u32(ctrl->dev->of_node, "qcom,cpr-count-repeat",
			&ctrl->count_repeat);

	ctrl->cpr_allowed_sw =
		of_property_read_bool(ctrl->dev->of_node, "qcom,cpr-enable") ||
		ctrl->cpr_global_setting == CPR_CLOSED_LOOP_EN;

	rc = cpr3_parse_irq_affinity(ctrl);
	if (rc)
		return rc;

	/* Aging reference voltage is optional */
	ctrl->aging_ref_volt = 0;
	of_property_read_u32(ctrl->dev->of_node, "qcom,cpr-aging-ref-voltage",
			&ctrl->aging_ref_volt);

	/* Aging possible bitmask is optional */
	ctrl->aging_possible_mask = 0;
	of_property_read_u32(ctrl->dev->of_node,
			"qcom,cpr-aging-allowed-reg-mask",
			&ctrl->aging_possible_mask);

	if (ctrl->aging_possible_mask) {
		/*
		 * Aging possible register value required if bitmask is
		 * specified
		 */
		rc = cpr3_parse_ctrl_u32(ctrl,
				"qcom,cpr-aging-allowed-reg-value",
				&ctrl->aging_possible_val, 0, UINT_MAX);
		if (rc)
			return rc;
	}

	if (of_find_property(ctrl->dev->of_node, "clock-names", NULL)) {
		ctrl->core_clk = devm_clk_get(ctrl->dev, "core_clk");
		if (IS_ERR(ctrl->core_clk)) {
			rc = PTR_ERR(ctrl->core_clk);
			if (rc != -EPROBE_DEFER)
				cpr3_err(ctrl, "unable request core clock, rc=%d\n",
				rc);
			return rc;
		}
	}

	rc = cpr3_panic_notifier_init(ctrl);
	if (rc)
		return rc;

	if (of_find_property(ctrl->dev->of_node, "vdd-supply", NULL)) {
		ctrl->vdd_regulator = devm_regulator_get(ctrl->dev, "vdd");
		if (IS_ERR(ctrl->vdd_regulator)) {
			rc = PTR_ERR(ctrl->vdd_regulator);
			if (rc != -EPROBE_DEFER)
				cpr3_err(ctrl, "unable to request vdd regulator, rc=%d\n",
					 rc);
			return rc;
		}
	} else {
		cpr3_err(ctrl, "vdd supply is not defined\n");
		return -ENODEV;
	}

	ctrl->system_regulator = devm_regulator_get_optional(ctrl->dev,
								"system");
	if (IS_ERR(ctrl->system_regulator)) {
		rc = PTR_ERR(ctrl->system_regulator);
		if (rc != -EPROBE_DEFER) {
			rc = 0;
			ctrl->system_regulator = NULL;
		} else {
			return rc;
		}
	}

	ctrl->mem_acc_regulator = devm_regulator_get_optional(ctrl->dev,
							      "mem-acc");
	if (IS_ERR(ctrl->mem_acc_regulator)) {
		rc = PTR_ERR(ctrl->mem_acc_regulator);
		if (rc != -EPROBE_DEFER) {
			rc = 0;
			ctrl->mem_acc_regulator = NULL;
		} else {
			return rc;
		}
	}

	return rc;
}

/**
 * cpr3_parse_open_loop_common_ctrl_data() - parse common open loop CPR3
 *			controller properties from device tree
 * @ctrl:		Pointer to the CPR3 controller
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_open_loop_common_ctrl_data(struct cpr3_controller *ctrl)
{
	int rc;

	rc = of_property_read_u32(ctrl->dev->of_node, "qcom,voltage-step",
				  &ctrl->step_volt);
	if (rc) {
		cpr3_err(ctrl, "error reading property qcom,voltage-step, rc=%d\n",
			 rc);
		return rc;
	}

	if (ctrl->step_volt <= 0) {
		cpr3_err(ctrl, "qcom,voltage-step=%d is invalid\n",
			 ctrl->step_volt);
		return -EINVAL;
	}

	if (of_find_property(ctrl->dev->of_node, "vdd-supply", NULL)) {
		ctrl->vdd_regulator = devm_regulator_get(ctrl->dev, "vdd");
		if (IS_ERR(ctrl->vdd_regulator)) {
			rc = PTR_ERR(ctrl->vdd_regulator);
			if (rc != -EPROBE_DEFER)
				cpr3_err(ctrl, "unable to request vdd regulator, rc=%d\n",
					 rc);
			return rc;
		}
	} else {
		cpr3_err(ctrl, "vdd supply is not defined\n");
		return -ENODEV;
	}

	ctrl->system_regulator = devm_regulator_get_optional(ctrl->dev,
								"system");
	if (IS_ERR(ctrl->system_regulator)) {
		rc = PTR_ERR(ctrl->system_regulator);
		if (rc != -EPROBE_DEFER) {
			rc = 0;
			ctrl->system_regulator = NULL;
		} else {
			return rc;
		}
	} else {
		rc = regulator_enable(ctrl->system_regulator);
	}

	ctrl->mem_acc_regulator = devm_regulator_get_optional(ctrl->dev,
							      "mem-acc");
	if (IS_ERR(ctrl->mem_acc_regulator)) {
		rc = PTR_ERR(ctrl->mem_acc_regulator);
		if (rc != -EPROBE_DEFER) {
			rc = 0;
			ctrl->mem_acc_regulator = NULL;
		} else {
			return rc;
		}
	}

	return rc;
}

/**
 * cpr3_limit_open_loop_voltages() - modify the open-loop voltage of each corner
 *				so that it fits within the floor to ceiling
 *				voltage range of the corner
 * @vreg:		Pointer to the CPR3 regulator
 *
 * This function clips the open-loop voltage for each corner so that it is
 * limited to the floor to ceiling range.  It also rounds each open-loop voltage
 * so that it corresponds to a set point available to the underlying regulator.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_limit_open_loop_voltages(struct cpr3_regulator *vreg)
{
	int i, volt;

	cpr3_debug(vreg, "open-loop voltages after trimming and rounding:\n");
	for (i = 0; i < vreg->corner_count; i++) {
		volt = CPR3_ROUND(vreg->corner[i].open_loop_volt,
					vreg->thread->ctrl->step_volt);
		if (volt < vreg->corner[i].floor_volt)
			volt = vreg->corner[i].floor_volt;
		else if (volt > vreg->corner[i].ceiling_volt)
			volt = vreg->corner[i].ceiling_volt;
		vreg->corner[i].open_loop_volt = volt;
		cpr3_debug(vreg, "corner[%2d]: open-loop=%d uV\n", i, volt);
	}

	return 0;
}

/**
 * cpr3_open_loop_voltage_as_ceiling() - configures the ceiling voltage for each
 *		corner to equal the open-loop voltage if the relevant device
 *		tree property is found for the CPR3 regulator
 * @vreg:		Pointer to the CPR3 regulator
 *
 * This function assumes that the the open-loop voltage for each corner has
 * already been rounded to the nearest allowed set point and that it falls
 * within the floor to ceiling range.
 *
 * Return: none
 */
void cpr3_open_loop_voltage_as_ceiling(struct cpr3_regulator *vreg)
{
	int i;

	if (!of_property_read_bool(vreg->of_node,
				"qcom,cpr-scaled-open-loop-voltage-as-ceiling"))
		return;

	for (i = 0; i < vreg->corner_count; i++)
		vreg->corner[i].ceiling_volt
			= vreg->corner[i].open_loop_volt;
}

/**
 * cpr3_limit_floor_voltages() - raise the floor voltage of each corner so that
 *		the optional maximum floor to ceiling voltage range specified in
 *		device tree is satisfied
 * @vreg:		Pointer to the CPR3 regulator
 *
 * This function also ensures that the open-loop voltage for each corner falls
 * within the final floor to ceiling voltage range and that floor voltages
 * increase monotonically.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_limit_floor_voltages(struct cpr3_regulator *vreg)
{
	char *prop = "qcom,cpr-floor-to-ceiling-max-range";
	int i, floor_new;
	u32 *floor_range;
	int rc = 0;

	if (!of_find_property(vreg->of_node, prop, NULL))
		goto enforce_monotonicity;

	floor_range = kcalloc(vreg->corner_count, sizeof(*floor_range),
				GFP_KERNEL);
	if (!floor_range)
		return -ENOMEM;

	rc = cpr3_parse_corner_array_property(vreg, prop, 1, floor_range);
	if (rc)
		goto free_floor_adjust;

	for (i = 0; i < vreg->corner_count; i++) {
		if ((s32)floor_range[i] >= 0) {
			floor_new = CPR3_ROUND(vreg->corner[i].ceiling_volt
							- floor_range[i],
						vreg->thread->ctrl->step_volt);

			vreg->corner[i].floor_volt = max(floor_new,
						vreg->corner[i].floor_volt);
			if (vreg->corner[i].open_loop_volt
			    < vreg->corner[i].floor_volt)
				vreg->corner[i].open_loop_volt
					= vreg->corner[i].floor_volt;
		}
	}

free_floor_adjust:
	kfree(floor_range);

enforce_monotonicity:
	/* Ensure that floor voltages increase monotonically. */
	for (i = 1; i < vreg->corner_count; i++) {
		if (vreg->corner[i].floor_volt
		    < vreg->corner[i - 1].floor_volt) {
			cpr3_debug(vreg, "corner %d floor voltage=%d uV < corner %d voltage=%d uV; overriding: corner %d voltage=%d\n",
				i, vreg->corner[i].floor_volt,
				i - 1, vreg->corner[i - 1].floor_volt,
				i, vreg->corner[i - 1].floor_volt);
			vreg->corner[i].floor_volt
				= vreg->corner[i - 1].floor_volt;

			if (vreg->corner[i].open_loop_volt
			    < vreg->corner[i].floor_volt)
				vreg->corner[i].open_loop_volt
					= vreg->corner[i].floor_volt;
			if (vreg->corner[i].ceiling_volt
			    < vreg->corner[i].floor_volt)
				vreg->corner[i].ceiling_volt
					= vreg->corner[i].floor_volt;
		}
	}

	return rc;
}

/**
 * cpr3_print_quots() - print CPR target quotients into the kernel log for
 *		debugging purposes
 * @vreg:		Pointer to the CPR3 regulator
 *
 * Return: none
 */
void cpr3_print_quots(struct cpr3_regulator *vreg)
{
	int i, j, pos;
	size_t buflen;
	char *buf;

	buflen = sizeof(*buf) * CPR3_RO_COUNT * (MAX_CHARS_PER_INT + 2);
	buf = kzalloc(buflen, GFP_KERNEL);
	if (!buf)
		return;

	for (i = 0; i < vreg->corner_count; i++) {
		for (j = 0, pos = 0; j < CPR3_RO_COUNT; j++)
			pos += scnprintf(buf + pos, buflen - pos, " %u",
				vreg->corner[i].target_quot[j]);
		cpr3_debug(vreg, "target quots[%2d]:%s\n", i, buf);
	}

	kfree(buf);
}

/**
 * cpr3_determine_part_type() - determine the part type (SS/TT/FF).
 *
 * qcom,cpr-part-types prop tells the number of part types for which correction
 * voltages are different. Another prop qcom,cpr-parts-voltage will contain the
 * open loop fuse voltage which will be compared with this part voltage
 * and accordingly part type will de determined.
 *
 * if qcom,cpr-part-types has value n, then qcom,cpr-parts-voltage will be
 * array of n - 1 elements which will contain the voltage in increasing order.
 * This function compares the fused volatge with all these voltage and returns
 * the first index for which the fused volatge is greater.
 *
 * @vreg:		Pointer to the CPR3 regulator
 * @fuse_volt:		fused open loop voltage which will be compared with
 *                      qcom,cpr-parts-voltage array
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_determine_part_type(struct cpr3_regulator *vreg, int fuse_volt)
{
	int i, rc, len;
	u32 volt;
	int soc_version_major;
	char prop_name[100];
	const char prop_name_def[] = "qcom,cpr-parts-voltage";
	const char prop_name_v2[] = "qcom,cpr-parts-voltage-v2";

	soc_version_major = read_ipq_soc_version_major();
        BUG_ON(soc_version_major <= 0);

	if (of_property_read_u32(vreg->of_node, "qcom,cpr-part-types",
				  &vreg->part_type_supported))
		return 0;

	if (soc_version_major > 1)
		strlcpy(prop_name, prop_name_v2, sizeof(prop_name_v2));
	else
		strlcpy(prop_name, prop_name_def, sizeof(prop_name_def));

	if (!of_find_property(vreg->of_node, prop_name, &len)) {
		cpr3_err(vreg, "property %s is missing\n", prop_name);
		return -EINVAL;
	}

	if (len != (vreg->part_type_supported - 1) * sizeof(u32)) {
		cpr3_err(vreg, "wrong len in qcom,cpr-parts-voltage\n");
		return -EINVAL;
	}

	for (i = 0; i < vreg->part_type_supported - 1; i++) {
		rc = of_property_read_u32_index(vreg->of_node,
					prop_name, i, &volt);
		if (rc) {
			cpr3_err(vreg, "error reading property %s, rc=%d\n",
				 prop_name, rc);
			return rc;
		}

		if (fuse_volt < volt)
			break;
	}

	vreg->part_type = i;
	return 0;
}

int cpr3_determine_temp_base_open_loop_correction(struct cpr3_regulator *vreg,
		int *fuse_volt)
{
	int i, rc, prev_volt;
	int *volt_adjust;
	char prop_str[75];
	int soc_version_major = read_ipq_soc_version_major();

	BUG_ON(soc_version_major <= 0);

	if (vreg->part_type_supported) {
		if (soc_version_major > 1)
			snprintf(prop_str, sizeof(prop_str),
			"qcom,cpr-cold-temp-voltage-adjustment-v2-%d",
			vreg->part_type);
		else
			snprintf(prop_str, sizeof(prop_str),
			"qcom,cpr-cold-temp-voltage-adjustment-%d",
			vreg->part_type);
	} else {
		strlcpy(prop_str, "qcom,cpr-cold-temp-voltage-adjustment",
			sizeof(prop_str));
	}

	if (!of_find_property(vreg->of_node, prop_str, NULL)) {
		/* No adjustment required. */
		cpr3_info(vreg, "No cold temperature adjustment required.\n");
		return 0;
	}

	volt_adjust = kcalloc(vreg->fuse_corner_count, sizeof(*volt_adjust),
	GFP_KERNEL);
	if (!volt_adjust)
		return -ENOMEM;

	rc = cpr3_parse_array_property(vreg, prop_str,
			vreg->fuse_corner_count, volt_adjust);
	if (rc) {
		cpr3_err(vreg, "could not load cold temp voltage adjustments, rc=%d\n",
			rc);
		goto done;
	}

	for (i = 0; i < vreg->fuse_corner_count; i++) {
		if (volt_adjust[i]) {
			prev_volt = fuse_volt[i];
			fuse_volt[i] += volt_adjust[i];
			cpr3_debug(vreg,
				"adjusted fuse corner %d open-loop voltage: %d -> %d uV\n",
				i, prev_volt, fuse_volt[i]);
		}
	}

done:
	kfree(volt_adjust);
	return rc;
}

/**
 * cpr3_can_adjust_cold_temp() - Is cold temperature adjustment available
 *
 * @vreg:		Pointer to the CPR3 regulator
 *
 * This function checks the cold temperature threshold is available
 *
 * Return: true on cold temperature threshold is available, else false
 */
bool cpr3_can_adjust_cold_temp(struct cpr3_regulator *vreg)
{
	char prop_str[75];
	int soc_version_major = read_ipq_soc_version_major();

	BUG_ON(soc_version_major <= 0);

	if (soc_version_major > 1)
		strlcpy(prop_str, "qcom,cpr-cold-temp-threshold-v2",
			sizeof(prop_str));
	else
		strlcpy(prop_str, "qcom,cpr-cold-temp-threshold",
			sizeof(prop_str));

	if (!of_find_property(vreg->of_node, prop_str, NULL)) {
		/* No adjustment required. */
		return false;
	} else
		return true;
}

/**
 * cpr3_get_cold_temp_threshold() - get cold temperature threshold
 *
 * @vreg:		Pointer to the CPR3 regulator
 * @cold_temp:		cold temperature read.
 *
 * This function reads the cold temperature threshold below which
 * cold temperature adjustment margins will be applied.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_get_cold_temp_threshold(struct cpr3_regulator *vreg, int *cold_temp)
{
	int rc;
	u32 temp;
	char req_prop_str[75], prop_str[75];
	int soc_version_major = read_ipq_soc_version_major();

	BUG_ON(soc_version_major <= 0);

	if (vreg->part_type_supported) {
		if (soc_version_major > 1)
			snprintf(req_prop_str, sizeof(req_prop_str),
			"qcom,cpr-cold-temp-voltage-adjustment-v2-%d",
			vreg->part_type);
		else
			snprintf(req_prop_str, sizeof(req_prop_str),
			"qcom,cpr-cold-temp-voltage-adjustment-%d",
			vreg->part_type);
	} else {
		strlcpy(req_prop_str, "qcom,cpr-cold-temp-voltage-adjustment",
			sizeof(req_prop_str));
	}

	if (soc_version_major > 1)
		strlcpy(prop_str, "qcom,cpr-cold-temp-threshold-v2",
			sizeof(prop_str));
	else
		strlcpy(prop_str, "qcom,cpr-cold-temp-threshold",
			sizeof(prop_str));

	if (!of_find_property(vreg->of_node, req_prop_str, NULL)) {
		/* No adjustment required. */
		cpr3_info(vreg, "Cold temperature adjustment not required.\n");
		return 0;
	}

	if (!of_find_property(vreg->of_node, prop_str, NULL)) {
		/* No adjustment required. */
                cpr3_err(vreg, "Missing %s required for %s\n",
			prop_str, req_prop_str);
		return -EINVAL;
        }

	rc = of_property_read_u32(vreg->of_node, prop_str, &temp);
	if (rc) {
		cpr3_err(vreg, "error reading property %s, rc=%d\n",
			prop_str, rc);
		return rc;
	}

	*cold_temp = temp;
	return 0;
}

/**
 * cpr3_adjust_fused_open_loop_voltages() - adjust the fused open-loop voltages
 *		for each fuse corner according to device tree values
 * @vreg:		Pointer to the CPR3 regulator
 * @fuse_volt:		Pointer to an array of the fused open-loop voltage
 *			values
 *
 * Voltage values in fuse_volt are modified in place.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_adjust_fused_open_loop_voltages(struct cpr3_regulator *vreg,
		int *fuse_volt)
{
	int i, rc, prev_volt;
	int *volt_adjust;
	char prop_str[75];
	int soc_version_major = read_ipq_soc_version_major();

	BUG_ON(soc_version_major <= 0);

	if (vreg->part_type_supported) {
		if (soc_version_major > 1)
			snprintf(prop_str, sizeof(prop_str),
			"qcom,cpr-open-loop-voltage-fuse-adjustment-v2-%d",
			vreg->part_type);
		else
		snprintf(prop_str, sizeof(prop_str),
			 "qcom,cpr-open-loop-voltage-fuse-adjustment-%d",
			 vreg->part_type);
	} else {
		strlcpy(prop_str, "qcom,cpr-open-loop-voltage-fuse-adjustment",
			sizeof(prop_str));
	}

	if (!of_find_property(vreg->of_node, prop_str, NULL)) {
		/* No adjustment required. */
		return 0;
	}

	volt_adjust = kcalloc(vreg->fuse_corner_count, sizeof(*volt_adjust),
				GFP_KERNEL);
	if (!volt_adjust)
		return -ENOMEM;

	rc = cpr3_parse_array_property(vreg,
		prop_str, vreg->fuse_corner_count, volt_adjust);
	if (rc) {
		cpr3_err(vreg, "could not load open-loop fused voltage adjustments, rc=%d\n",
			rc);
		goto done;
	}

	for (i = 0; i < vreg->fuse_corner_count; i++) {
		if (volt_adjust[i]) {
			prev_volt = fuse_volt[i];
			fuse_volt[i] += volt_adjust[i];
			cpr3_debug(vreg, "adjusted fuse corner %d open-loop voltage: %d --> %d uV\n",
				i, prev_volt, fuse_volt[i]);
		}
	}

done:
	kfree(volt_adjust);
	return rc;
}

/**
 * cpr3_adjust_open_loop_voltages() - adjust the open-loop voltages for each
 *		corner according to device tree values
 * @vreg:		Pointer to the CPR3 regulator
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_adjust_open_loop_voltages(struct cpr3_regulator *vreg)
{
	int i, rc, prev_volt, min_volt;
	int *volt_adjust, *volt_diff;

	if (!of_find_property(vreg->of_node,
			"qcom,cpr-open-loop-voltage-adjustment", NULL)) {
		/* No adjustment required. */
		return 0;
	}

	volt_adjust = kcalloc(vreg->corner_count, sizeof(*volt_adjust),
				GFP_KERNEL);
	volt_diff = kcalloc(vreg->corner_count, sizeof(*volt_diff), GFP_KERNEL);
	if (!volt_adjust || !volt_diff) {
		rc = -ENOMEM;
		goto done;
	}

	rc = cpr3_parse_corner_array_property(vreg,
		"qcom,cpr-open-loop-voltage-adjustment", 1, volt_adjust);
	if (rc) {
		cpr3_err(vreg, "could not load open-loop voltage adjustments, rc=%d\n",
			rc);
		goto done;
	}

	for (i = 0; i < vreg->corner_count; i++) {
		if (volt_adjust[i]) {
			prev_volt = vreg->corner[i].open_loop_volt;
			vreg->corner[i].open_loop_volt += volt_adjust[i];
			cpr3_debug(vreg, "adjusted corner %d open-loop voltage: %d --> %d uV\n",
				i, prev_volt, vreg->corner[i].open_loop_volt);
		}
	}

	if (of_find_property(vreg->of_node,
			"qcom,cpr-open-loop-voltage-min-diff", NULL)) {
		rc = cpr3_parse_corner_array_property(vreg,
			"qcom,cpr-open-loop-voltage-min-diff", 1, volt_diff);
		if (rc) {
			cpr3_err(vreg, "could not load minimum open-loop voltage differences, rc=%d\n",
				rc);
			goto done;
		}
	}

	/*
	 * Ensure that open-loop voltages increase monotonically with respect
	 * to configurable minimum allowed differences.
	 */
	for (i = 1; i < vreg->corner_count; i++) {
		min_volt = vreg->corner[i - 1].open_loop_volt + volt_diff[i];
		if (vreg->corner[i].open_loop_volt < min_volt) {
			cpr3_debug(vreg, "adjusted corner %d open-loop voltage=%d uV < corner %d voltage=%d uV + min diff=%d uV; overriding: corner %d voltage=%d\n",
				i, vreg->corner[i].open_loop_volt,
				i - 1, vreg->corner[i - 1].open_loop_volt,
				volt_diff[i], i, min_volt);
			vreg->corner[i].open_loop_volt = min_volt;
		}
	}

done:
	kfree(volt_diff);
	kfree(volt_adjust);
	return rc;
}

/**
 * cpr3_quot_adjustment() - returns the quotient adjustment value resulting from
 *		the specified voltage adjustment and RO scaling factor
 * @ro_scale:		The CPR ring oscillator (RO) scaling factor with units
 *			of QUOT/V
 * @volt_adjust:	The amount to adjust the voltage by in units of
 *			microvolts.  This value may be positive or negative.
 */
int cpr3_quot_adjustment(int ro_scale, int volt_adjust)
{
	unsigned long long temp;
	int quot_adjust;
	int sign = 1;

	if (ro_scale < 0) {
		sign = -sign;
		ro_scale = -ro_scale;
	}

	if (volt_adjust < 0) {
		sign = -sign;
		volt_adjust = -volt_adjust;
	}

	temp = (unsigned long long)ro_scale * (unsigned long long)volt_adjust;
	do_div(temp, 1000000);

	quot_adjust = temp;
	quot_adjust *= sign;

	return quot_adjust;
}

/**
 * cpr3_voltage_adjustment() - returns the voltage adjustment value resulting
 *		from the specified quotient adjustment and RO scaling factor
 * @ro_scale:		The CPR ring oscillator (RO) scaling factor with units
 *			of QUOT/V
 * @quot_adjust:	The amount to adjust the quotient by in units of
 *			QUOT.  This value may be positive or negative.
 */
int cpr3_voltage_adjustment(int ro_scale, int quot_adjust)
{
	unsigned long long temp;
	int volt_adjust;
	int sign = 1;

	if (ro_scale < 0) {
		sign = -sign;
		ro_scale = -ro_scale;
	}

	if (quot_adjust < 0) {
		sign = -sign;
		quot_adjust = -quot_adjust;
	}

	if (ro_scale == 0)
		return 0;

	temp = (unsigned long long)quot_adjust * 1000000;
	do_div(temp, ro_scale);

	volt_adjust = temp;
	volt_adjust *= sign;

	return volt_adjust;
}

/**
 * cpr3_parse_closed_loop_voltage_adjustments() - load per-fuse-corner and
 *		per-corner closed-loop adjustment values from device tree
 * @vreg:		Pointer to the CPR3 regulator
 * @ro_sel:		Array of ring oscillator values selected for each
 *			fuse corner
 * @volt_adjust:	Pointer to array which will be filled with the
 *			per-corner closed-loop adjustment voltages
 * @volt_adjust_fuse:	Pointer to array which will be filled with the
 *			per-fuse-corner closed-loop adjustment voltages
 * @ro_scale:		Pointer to array which will be filled with the
 *			per-fuse-corner RO scaling factor values with units of
 *			QUOT/V
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_parse_closed_loop_voltage_adjustments(
			struct cpr3_regulator *vreg, u64 *ro_sel,
			int *volt_adjust, int *volt_adjust_fuse, int *ro_scale)
{
	int i, rc;
	u32 *ro_all_scale;

	char volt_adj[] = "qcom,cpr-closed-loop-voltage-adjustment";
	char volt_fuse_adj[] = "qcom,cpr-closed-loop-voltage-fuse-adjustment";
	char ro_scaling[] = "qcom,cpr-ro-scaling-factor";

	if (!of_find_property(vreg->of_node, volt_adj, NULL)
	    && !of_find_property(vreg->of_node, volt_fuse_adj, NULL)
	    && !vreg->aging_allowed) {
		/* No adjustment required. */
		return 0;
	} else if (!of_find_property(vreg->of_node, ro_scaling, NULL)) {
		cpr3_err(vreg, "Missing %s required for closed-loop voltage adjustment.\n",
				ro_scaling);
		return -EINVAL;
	}

	ro_all_scale = kcalloc(vreg->fuse_corner_count * CPR3_RO_COUNT,
				sizeof(*ro_all_scale), GFP_KERNEL);
	if (!ro_all_scale)
		return -ENOMEM;

	rc = cpr3_parse_array_property(vreg, ro_scaling,
		vreg->fuse_corner_count * CPR3_RO_COUNT, ro_all_scale);
	if (rc) {
		cpr3_err(vreg, "could not load RO scaling factors, rc=%d\n",
			rc);
		goto done;
	}

	for (i = 0; i < vreg->fuse_corner_count; i++)
		ro_scale[i] = ro_all_scale[i * CPR3_RO_COUNT + ro_sel[i]];

	for (i = 0; i < vreg->corner_count; i++)
		memcpy(vreg->corner[i].ro_scale,
		 &ro_all_scale[vreg->corner[i].cpr_fuse_corner * CPR3_RO_COUNT],
		 sizeof(*ro_all_scale) * CPR3_RO_COUNT);

	if (of_find_property(vreg->of_node, volt_fuse_adj, NULL)) {
		rc = cpr3_parse_array_property(vreg, volt_fuse_adj,
			vreg->fuse_corner_count, volt_adjust_fuse);
		if (rc) {
			cpr3_err(vreg, "could not load closed-loop fused voltage adjustments, rc=%d\n",
				rc);
			goto done;
		}
	}

	if (of_find_property(vreg->of_node, volt_adj, NULL)) {
		rc = cpr3_parse_corner_array_property(vreg, volt_adj,
			1, volt_adjust);
		if (rc) {
			cpr3_err(vreg, "could not load closed-loop voltage adjustments, rc=%d\n",
				rc);
			goto done;
		}
	}

done:
	kfree(ro_all_scale);
	return rc;
}

/**
 * cpr3_apm_init() - initialize APM data for a CPR3 controller
 * @ctrl:		Pointer to the CPR3 controller
 *
 * This function loads memory array power mux (APM) data from device tree
 * if it is present and requests a handle to the appropriate APM controller
 * device.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_apm_init(struct cpr3_controller *ctrl)
{
	struct device_node *node = ctrl->dev->of_node;
	int rc;

	if (!of_find_property(node, "qcom,apm-ctrl", NULL)) {
		/* No APM used */
		return 0;
	}

	ctrl->apm = msm_apm_ctrl_dev_get(ctrl->dev);
	if (IS_ERR(ctrl->apm)) {
		rc = PTR_ERR(ctrl->apm);
		if (rc != -EPROBE_DEFER)
			cpr3_err(ctrl, "APM get failed, rc=%d\n", rc);
		return rc;
	}

	rc = of_property_read_u32(node, "qcom,apm-threshold-voltage",
				&ctrl->apm_threshold_volt);
	if (rc) {
		cpr3_err(ctrl, "error reading qcom,apm-threshold-voltage, rc=%d\n",
			rc);
		return rc;
	}
	ctrl->apm_threshold_volt
		= CPR3_ROUND(ctrl->apm_threshold_volt, ctrl->step_volt);

	/* No error check since this is an optional property. */
	of_property_read_u32(node, "qcom,apm-hysteresis-voltage",
				&ctrl->apm_adj_volt);
	ctrl->apm_adj_volt = CPR3_ROUND(ctrl->apm_adj_volt, ctrl->step_volt);

	ctrl->apm_high_supply = MSM_APM_SUPPLY_APCC;
	ctrl->apm_low_supply = MSM_APM_SUPPLY_MX;

	return 0;
}

/**
 * cpr3_mem_acc_init() - initialize mem-acc regulator data for
 *		a CPR3 regulator
 * @ctrl:		Pointer to the CPR3 controller
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_mem_acc_init(struct cpr3_regulator *vreg)
{
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	u32 *temp;
	int i, rc;

	if (!ctrl->mem_acc_regulator) {
		cpr3_info(ctrl, "not using memory accelerator regulator\n");
		return 0;
	}

	temp = kcalloc(vreg->corner_count, sizeof(*temp), GFP_KERNEL);
	if (!temp)
		return -ENOMEM;

	rc = cpr3_parse_corner_array_property(vreg, "qcom,mem-acc-voltage",
					      1, temp);
	if (rc) {
		cpr3_err(ctrl, "could not load mem-acc corners, rc=%d\n", rc);
	} else {
		for (i = 0; i < vreg->corner_count; i++)
			vreg->corner[i].mem_acc_volt = temp[i];
	}

	kfree(temp);
	return rc;
}

/**
 * cpr4_load_core_and_temp_adj() - parse amount of voltage adjustment for
 *		per-online-core and per-temperature voltage adjustment for a
 *		given corner or corner band from device tree.
 * @vreg:	Pointer to the CPR3 regulator
 * @num:	Corner number or corner band number
 * @use_corner_band:	Boolean indicating if the CPR3 regulator supports
 *			adjustments per corner band
 *
 * Return: 0 on success, errno on failure
 */
static int cpr4_load_core_and_temp_adj(struct cpr3_regulator *vreg,
					int num, bool use_corner_band)
{
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	struct cpr4_sdelta *sdelta;
	int sdelta_size, i, j, pos, rc = 0;
	char str[75];
	size_t buflen;
	char *buf;

	sdelta = use_corner_band ? vreg->corner_band[num].sdelta :
		vreg->corner[num].sdelta;

	if (!sdelta->allow_core_count_adj && !sdelta->allow_temp_adj) {
		/* corner doesn't need sdelta table */
		sdelta->max_core_count = 0;
		sdelta->temp_band_count = 0;
		return rc;
	}

	sdelta_size = sdelta->max_core_count * sdelta->temp_band_count;
	if (use_corner_band)
		snprintf(str, sizeof(str),
			 "corner_band=%d core_config_count=%d temp_band_count=%d sdelta_size=%d\n",
			 num, sdelta->max_core_count,
			 sdelta->temp_band_count, sdelta_size);
	else
		snprintf(str, sizeof(str),
			 "corner=%d core_config_count=%d temp_band_count=%d sdelta_size=%d\n",
			 num, sdelta->max_core_count,
			 sdelta->temp_band_count, sdelta_size);

	cpr3_debug(vreg, "%s", str);

	sdelta->table = devm_kcalloc(ctrl->dev, sdelta_size,
				sizeof(*sdelta->table), GFP_KERNEL);
	if (!sdelta->table)
		return -ENOMEM;

	if (use_corner_band)
		snprintf(str, sizeof(str),
			 "qcom,cpr-corner-band%d-temp-core-voltage-adjustment",
			 num + CPR3_CORNER_OFFSET);
	else
		snprintf(str, sizeof(str),
			 "qcom,cpr-corner%d-temp-core-voltage-adjustment",
			 num + CPR3_CORNER_OFFSET);

	rc = cpr3_parse_array_property(vreg, str, sdelta_size,
				sdelta->table);
	if (rc) {
		cpr3_err(vreg, "could not load %s, rc=%d\n", str, rc);
		return rc;
	}

	/*
	 * Convert sdelta margins from uV to PMIC steps and apply negation to
	 * follow the SDELTA register semantics.
	 */
	for (i = 0; i < sdelta_size; i++)
		sdelta->table[i] = -(sdelta->table[i] / ctrl->step_volt);

	buflen = sizeof(*buf) * sdelta_size * (MAX_CHARS_PER_INT + 2);
	buf = kzalloc(buflen, GFP_KERNEL);
	if (!buf)
		return rc;

	for (i = 0; i < sdelta->max_core_count; i++) {
		for (j = 0, pos = 0; j < sdelta->temp_band_count; j++)
			pos += scnprintf(buf + pos, buflen - pos, " %u",
			 sdelta->table[i * sdelta->temp_band_count + j]);
		cpr3_debug(vreg, "sdelta[%d]:%s\n", i, buf);
	}

	kfree(buf);
	return rc;
}

/**
 * cpr4_parse_core_count_temp_voltage_adj() - parse configuration data for
 *		per-online-core and per-temperature voltage adjustment for
 *		a CPR3 regulator from device tree.
 * @vreg:	Pointer to the CPR3 regulator
 * @use_corner_band:	Boolean indicating if the CPR3 regulator supports
 *			adjustments per corner band
 *
 * This function supports parsing of per-online-core and per-temperature
 * adjustments per corner or per corner band. CPR controllers which support
 * corner bands apply the same adjustments to all corners within a corner band.
 *
 * Return: 0 on success, errno on failure
 */
int cpr4_parse_core_count_temp_voltage_adj(
			struct cpr3_regulator *vreg, bool use_corner_band)
{
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	struct device_node *node = vreg->of_node;
	struct cpr3_corner *corner;
	struct cpr4_sdelta *sdelta;
	int i, sdelta_table_count, rc = 0;
	int *allow_core_count_adj = NULL, *allow_temp_adj = NULL;
	char prop_str[75];

	if (of_find_property(node, use_corner_band ?
			     "qcom,corner-band-allow-temp-adjustment"
			     : "qcom,corner-allow-temp-adjustment", NULL)) {
		if (!ctrl->allow_temp_adj) {
			cpr3_err(ctrl, "Temperature adjustment configurations missing\n");
			return -EINVAL;
		}

		vreg->allow_temp_adj = true;
	}

	if (of_find_property(node, use_corner_band ?
			     "qcom,corner-band-allow-core-count-adjustment"
			     : "qcom,corner-allow-core-count-adjustment",
			     NULL)) {
		rc = of_property_read_u32(node, "qcom,max-core-count",
				&vreg->max_core_count);
		if (rc) {
			cpr3_err(vreg, "error reading qcom,max-core-count, rc=%d\n",
				rc);
			return -EINVAL;
		}

		vreg->allow_core_count_adj = true;
		ctrl->allow_core_count_adj = true;
	}

	if (!vreg->allow_temp_adj && !vreg->allow_core_count_adj) {
		/*
		 * Both per-online-core and temperature based adjustments are
		 * disabled for this regulator.
		 */
		return 0;
	} else if (!vreg->allow_core_count_adj) {
		/*
		 * Only per-temperature voltage adjusments are allowed.
		 * Keep max core count value as 1 to allocate SDELTA.
		 */
		vreg->max_core_count = 1;
	}

	if (vreg->allow_core_count_adj) {
		allow_core_count_adj = kcalloc(vreg->corner_count,
					sizeof(*allow_core_count_adj),
					GFP_KERNEL);
		if (!allow_core_count_adj)
			return -ENOMEM;

		snprintf(prop_str, sizeof(prop_str), "%s", use_corner_band ?
			 "qcom,corner-band-allow-core-count-adjustment" :
			 "qcom,corner-allow-core-count-adjustment");

		rc = use_corner_band ?
			cpr3_parse_corner_band_array_property(vreg, prop_str,
					      1, allow_core_count_adj) :
			cpr3_parse_corner_array_property(vreg, prop_str,
						 1, allow_core_count_adj);
		if (rc) {
			cpr3_err(vreg, "error reading %s, rc=%d\n", prop_str,
				 rc);
			goto done;
		}
	}

	if (vreg->allow_temp_adj) {
		allow_temp_adj = kcalloc(vreg->corner_count,
					sizeof(*allow_temp_adj), GFP_KERNEL);
		if (!allow_temp_adj) {
			rc = -ENOMEM;
			goto done;
		}

		snprintf(prop_str, sizeof(prop_str), "%s", use_corner_band ?
			 "qcom,corner-band-allow-temp-adjustment" :
			 "qcom,corner-allow-temp-adjustment");

		rc = use_corner_band ?
			cpr3_parse_corner_band_array_property(vreg, prop_str,
						      1, allow_temp_adj) :
			cpr3_parse_corner_array_property(vreg, prop_str,
						 1, allow_temp_adj);
		if (rc) {
			cpr3_err(vreg, "error reading %s, rc=%d\n", prop_str,
				 rc);
			goto done;
		}
	}

	sdelta_table_count = use_corner_band ? vreg->corner_band_count :
		vreg->corner_count;

	for (i = 0; i < sdelta_table_count; i++) {
		sdelta = devm_kzalloc(ctrl->dev, sizeof(*corner->sdelta),
				      GFP_KERNEL);
		if (!sdelta) {
			rc = -ENOMEM;
			goto done;
		}

		if (allow_core_count_adj)
			sdelta->allow_core_count_adj = allow_core_count_adj[i];
		if (allow_temp_adj)
			sdelta->allow_temp_adj = allow_temp_adj[i];
		sdelta->max_core_count = vreg->max_core_count;
		sdelta->temp_band_count = ctrl->temp_band_count;

		if (use_corner_band)
			vreg->corner_band[i].sdelta = sdelta;
		else
			vreg->corner[i].sdelta = sdelta;

		rc = cpr4_load_core_and_temp_adj(vreg, i, use_corner_band);
		if (rc) {
			cpr3_err(vreg, "corner/band %d core and temp adjustment loading failed, rc=%d\n",
				 i, rc);
			goto done;
		}
	}

done:
	kfree(allow_core_count_adj);
	kfree(allow_temp_adj);

	return rc;
}

/**
 * cprh_adjust_voltages_for_apm() - adjust per-corner floor and ceiling voltages
 *		so that they do not overlap the APM threshold voltage.
 * @vreg:		Pointer to the CPR3 regulator
 *
 * The memory array power mux (APM) must be configured for a specific supply
 * based upon where the VDD voltage lies with respect to the APM threshold
 * voltage.  When using CPR hardware closed-loop, the voltage may vary anywhere
 * between the floor and ceiling voltage without software notification.
 * Therefore, it is required that the floor to ceiling range for every corner
 * not intersect the APM threshold voltage.  This function adjusts the floor to
 * ceiling range for each corner which violates this requirement.
 *
 * The following algorithm is applied:
 *	if floor < threshold <= ceiling:
 *		if open_loop >= threshold, then floor = threshold - adj
 *		else ceiling = threshold - step
 * where:
 *	adj = APM hysteresis voltage established to minimize the number of
 *	      corners with artificially increased floor voltages
 *	step = voltage in microvolts of a single step of the VDD supply
 *
 * The open-loop voltage is also bounded by the new floor or ceiling value as
 * needed.
 *
 * Return: none
 */
void cprh_adjust_voltages_for_apm(struct cpr3_regulator *vreg)
{
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	struct cpr3_corner *corner;
	int i, adj, threshold, prev_ceiling, prev_floor, prev_open_loop;

	if (!ctrl->apm_threshold_volt) {
		/* APM not being used. */
		return;
	}

	ctrl->apm_threshold_volt = CPR3_ROUND(ctrl->apm_threshold_volt,
						ctrl->step_volt);
	ctrl->apm_adj_volt = CPR3_ROUND(ctrl->apm_adj_volt, ctrl->step_volt);

	threshold = ctrl->apm_threshold_volt;
	adj = ctrl->apm_adj_volt;

	for (i = 0; i < vreg->corner_count; i++) {
		corner = &vreg->corner[i];

		if (threshold <= corner->floor_volt
		    || threshold > corner->ceiling_volt)
			continue;

		prev_floor = corner->floor_volt;
		prev_ceiling = corner->ceiling_volt;
		prev_open_loop = corner->open_loop_volt;

		if (corner->open_loop_volt >= threshold) {
			corner->floor_volt = max(corner->floor_volt,
						 threshold - adj);
			if (corner->open_loop_volt < corner->floor_volt)
				corner->open_loop_volt = corner->floor_volt;
		} else {
			corner->ceiling_volt = threshold - ctrl->step_volt;
		}

		if (corner->floor_volt != prev_floor
		    || corner->ceiling_volt != prev_ceiling
		    || corner->open_loop_volt != prev_open_loop)
			cpr3_debug(vreg, "APM threshold=%d, APM adj=%d changed corner %d voltages; prev: floor=%d, ceiling=%d, open-loop=%d; new: floor=%d, ceiling=%d, open-loop=%d\n",
				threshold, adj, i, prev_floor, prev_ceiling,
				prev_open_loop, corner->floor_volt,
				corner->ceiling_volt, corner->open_loop_volt);
	}
}

/**
 * cprh_adjust_voltages_for_mem_acc() - adjust per-corner floor and ceiling
 *		voltages so that they do not intersect the MEM ACC threshold
 *		voltage
 * @vreg:		Pointer to the CPR3 regulator
 *
 * The following algorithm is applied:
 *	if floor < threshold <= ceiling:
 *		if open_loop >= threshold, then floor = threshold
 *		else ceiling = threshold - step
 * where:
 *	step = voltage in microvolts of a single step of the VDD supply
 *
 * The open-loop voltage is also bounded by the new floor or ceiling value as
 * needed.
 *
 * Return: none
 */
void cprh_adjust_voltages_for_mem_acc(struct cpr3_regulator *vreg)
{
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	struct cpr3_corner *corner;
	int i, threshold, prev_ceiling, prev_floor, prev_open_loop;

	if (!ctrl->mem_acc_threshold_volt) {
		/* MEM ACC not being used. */
		return;
	}

	ctrl->mem_acc_threshold_volt = CPR3_ROUND(ctrl->mem_acc_threshold_volt,
						ctrl->step_volt);

	threshold = ctrl->mem_acc_threshold_volt;

	for (i = 0; i < vreg->corner_count; i++) {
		corner = &vreg->corner[i];

		if (threshold <= corner->floor_volt
		    || threshold > corner->ceiling_volt)
			continue;

		prev_floor = corner->floor_volt;
		prev_ceiling = corner->ceiling_volt;
		prev_open_loop = corner->open_loop_volt;

		if (corner->open_loop_volt >= threshold) {
			corner->floor_volt = max(corner->floor_volt, threshold);
			if (corner->open_loop_volt < corner->floor_volt)
				corner->open_loop_volt = corner->floor_volt;
		} else {
			corner->ceiling_volt = threshold - ctrl->step_volt;
		}

		if (corner->floor_volt != prev_floor
		    || corner->ceiling_volt != prev_ceiling
		    || corner->open_loop_volt != prev_open_loop)
			cpr3_debug(vreg, "MEM ACC threshold=%d changed corner %d voltages; prev: floor=%d, ceiling=%d, open-loop=%d; new: floor=%d, ceiling=%d, open-loop=%d\n",
				threshold, i, prev_floor, prev_ceiling,
				prev_open_loop, corner->floor_volt,
				corner->ceiling_volt, corner->open_loop_volt);
	}
}

/**
 * cpr3_apply_closed_loop_offset_voltages() - modify the closed-loop voltage
 *		adjustments by the amounts that are needed for this
 *		fuse combo
 * @vreg:		Pointer to the CPR3 regulator
 * @volt_adjust:	Array of closed-loop voltage adjustment values of length
 *			vreg->corner_count which is further adjusted based upon
 *			offset voltage fuse values.
 * @fuse_volt_adjust:	Fused closed-loop voltage adjustment values of length
 *			vreg->fuse_corner_count.
 *
 * Return: 0 on success, errno on failure
 */
static int cpr3_apply_closed_loop_offset_voltages(struct cpr3_regulator *vreg,
			int *volt_adjust, int *fuse_volt_adjust)
{
	u32 *corner_map;
	int rc = 0, i;

	if (!of_find_property(vreg->of_node,
		"qcom,cpr-fused-closed-loop-voltage-adjustment-map", NULL)) {
		/* No closed-loop offset required. */
		return 0;
	}

	corner_map = kcalloc(vreg->corner_count, sizeof(*corner_map),
				GFP_KERNEL);
	if (!corner_map)
		return -ENOMEM;

	rc = cpr3_parse_corner_array_property(vreg,
		"qcom,cpr-fused-closed-loop-voltage-adjustment-map",
		1, corner_map);
	if (rc)
		goto done;

	for (i = 0; i < vreg->corner_count; i++) {
		if (corner_map[i] == 0) {
			continue;
		} else if (corner_map[i] > vreg->fuse_corner_count) {
			cpr3_err(vreg, "corner %d mapped to invalid fuse corner: %u\n",
				i, corner_map[i]);
			rc = -EINVAL;
			goto done;
		}

		volt_adjust[i] += fuse_volt_adjust[corner_map[i] - 1];
	}

done:
	kfree(corner_map);
	return rc;
}

/**
 * cpr3_enforce_inc_quotient_monotonicity() - Ensure that target quotients
 *		increase monotonically from lower to higher corners
 * @vreg:		Pointer to the CPR3 regulator
 *
 * Return: 0 on success, errno on failure
 */
static void cpr3_enforce_inc_quotient_monotonicity(struct cpr3_regulator *vreg)
{
	int i, j;

	for (i = 1; i < vreg->corner_count; i++) {
		for (j = 0; j < CPR3_RO_COUNT; j++) {
			if (vreg->corner[i].target_quot[j]
			    && vreg->corner[i].target_quot[j]
					< vreg->corner[i - 1].target_quot[j]) {
				cpr3_debug(vreg, "corner %d RO%u target quot=%u < corner %d RO%u target quot=%u; overriding: corner %d RO%u target quot=%u\n",
					i, j,
					vreg->corner[i].target_quot[j],
					i - 1, j,
					vreg->corner[i - 1].target_quot[j],
					i, j,
					vreg->corner[i - 1].target_quot[j]);
				vreg->corner[i].target_quot[j]
					= vreg->corner[i - 1].target_quot[j];
			}
		}
	}
}

/**
 * cpr3_enforce_dec_quotient_monotonicity() - Ensure that target quotients
 *		decrease monotonically from higher to lower corners
 * @vreg:		Pointer to the CPR3 regulator
 *
 * Return: 0 on success, errno on failure
 */
static void cpr3_enforce_dec_quotient_monotonicity(struct cpr3_regulator *vreg)
{
	int i, j;

	for (i = vreg->corner_count - 2; i >= 0; i--) {
		for (j = 0; j < CPR3_RO_COUNT; j++) {
			if (vreg->corner[i + 1].target_quot[j]
			    && vreg->corner[i].target_quot[j]
					> vreg->corner[i + 1].target_quot[j]) {
				cpr3_debug(vreg, "corner %d RO%u target quot=%u > corner %d RO%u target quot=%u; overriding: corner %d RO%u target quot=%u\n",
					i, j,
					vreg->corner[i].target_quot[j],
					i + 1, j,
					vreg->corner[i + 1].target_quot[j],
					i, j,
					vreg->corner[i + 1].target_quot[j]);
				vreg->corner[i].target_quot[j]
					= vreg->corner[i + 1].target_quot[j];
			}
		}
	}
}

/**
 * _cpr3_adjust_target_quotients() - adjust the target quotients for each
 *		corner of the regulator according to input adjustment and
 *		scaling arrays
 * @vreg:		Pointer to the CPR3 regulator
 * @volt_adjust:	Pointer to an array of closed-loop voltage adjustments
 *			with units of microvolts.  The array must have
 *			vreg->corner_count number of elements.
 * @ro_scale:		Pointer to a flattened 2D array of RO scaling factors.
 *			The array must have an inner dimension of CPR3_RO_COUNT
 *			and an outer dimension of vreg->corner_count
 * @label:		Null terminated string providing a label for the type
 *			of adjustment.
 *
 * Return: true if any corners received a positive voltage adjustment (> 0),
 *	   else false
 */
static bool _cpr3_adjust_target_quotients(struct cpr3_regulator *vreg,
		const int *volt_adjust, const int *ro_scale, const char *label)
{
	int i, j, quot_adjust;
	bool is_increasing = false;
	u32 prev_quot;

	for (i = 0; i < vreg->corner_count; i++) {
		for (j = 0; j < CPR3_RO_COUNT; j++) {
			if (vreg->corner[i].target_quot[j]) {
				quot_adjust = cpr3_quot_adjustment(
					ro_scale[i * CPR3_RO_COUNT + j],
					volt_adjust[i]);
				if (quot_adjust) {
					prev_quot = vreg->corner[i].
							target_quot[j];
					vreg->corner[i].target_quot[j]
						+= quot_adjust;
					cpr3_debug(vreg, "adjusted corner %d RO%d target quot %s: %u --> %u (%d uV)\n",
						i, j, label, prev_quot,
						vreg->corner[i].target_quot[j],
						volt_adjust[i]);
				}
			}
		}
		if (volt_adjust[i] > 0)
			is_increasing = true;
	}

	return is_increasing;
}

/**
 * cpr3_adjust_target_quotients() - adjust the target quotients for each
 *			corner according to device tree values and fuse values
 * @vreg:		Pointer to the CPR3 regulator
 * @fuse_volt_adjust:	Fused closed-loop voltage adjustment values of length
 *			vreg->fuse_corner_count. This parameter could be null
 *			pointer when no fused adjustments are needed.
 *
 * Return: 0 on success, errno on failure
 */
int cpr3_adjust_target_quotients(struct cpr3_regulator *vreg,
			int *fuse_volt_adjust)
{
	int i, rc;
	int *volt_adjust, *ro_scale;
	bool explicit_adjustment, fused_adjustment, is_increasing;

	explicit_adjustment = of_find_property(vreg->of_node,
		"qcom,cpr-closed-loop-voltage-adjustment", NULL);
	fused_adjustment = of_find_property(vreg->of_node,
		"qcom,cpr-fused-closed-loop-voltage-adjustment-map", NULL);

	if (!explicit_adjustment && !fused_adjustment && !vreg->aging_allowed) {
		/* No adjustment required. */
		return 0;
	} else if (!of_find_property(vreg->of_node,
			"qcom,cpr-ro-scaling-factor", NULL)) {
		cpr3_err(vreg, "qcom,cpr-ro-scaling-factor is required for closed-loop voltage adjustment, but is missing\n");
		return -EINVAL;
	}

	volt_adjust = kcalloc(vreg->corner_count, sizeof(*volt_adjust),
				GFP_KERNEL);
	ro_scale = kcalloc(vreg->corner_count * CPR3_RO_COUNT,
				sizeof(*ro_scale), GFP_KERNEL);
	if (!volt_adjust || !ro_scale) {
		rc = -ENOMEM;
		goto done;
	}

	rc = cpr3_parse_corner_array_property(vreg,
			"qcom,cpr-ro-scaling-factor", CPR3_RO_COUNT, ro_scale);
	if (rc) {
		cpr3_err(vreg, "could not load RO scaling factors, rc=%d\n",
			rc);
		goto done;
	}

	for (i = 0; i < vreg->corner_count; i++)
		memcpy(vreg->corner[i].ro_scale, &ro_scale[i * CPR3_RO_COUNT],
			sizeof(*ro_scale) * CPR3_RO_COUNT);

	if (explicit_adjustment) {
		rc = cpr3_parse_corner_array_property(vreg,
			"qcom,cpr-closed-loop-voltage-adjustment",
			1, volt_adjust);
		if (rc) {
			cpr3_err(vreg, "could not load closed-loop voltage adjustments, rc=%d\n",
				rc);
			goto done;
		}

		_cpr3_adjust_target_quotients(vreg, volt_adjust, ro_scale,
			"from DT");
		cpr3_enforce_inc_quotient_monotonicity(vreg);
	}

	if (fused_adjustment && fuse_volt_adjust) {
		memset(volt_adjust, 0,
			sizeof(*volt_adjust) * vreg->corner_count);

		rc = cpr3_apply_closed_loop_offset_voltages(vreg, volt_adjust,
				fuse_volt_adjust);
		if (rc) {
			cpr3_err(vreg, "could not apply fused closed-loop voltage reductions, rc=%d\n",
				rc);
			goto done;
		}

		is_increasing = _cpr3_adjust_target_quotients(vreg, volt_adjust,
					ro_scale, "from fuse");
		if (is_increasing)
			cpr3_enforce_inc_quotient_monotonicity(vreg);
		else
			cpr3_enforce_dec_quotient_monotonicity(vreg);
	}

done:
	kfree(volt_adjust);
	kfree(ro_scale);
	return rc;
}
