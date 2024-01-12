/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/thermal.h>

#include "cpr3-regulator.h"

#define IPQ807x_NPU_FUSE_CORNERS		2
#define IPQ817x_NPU_FUSE_CORNERS		1
#define IPQ807x_NPU_FUSE_STEP_VOLT		8000
#define IPQ807x_NPU_VOLTAGE_FUSE_SIZE		6
#define IPQ807x_NPU_CPR_CLOCK_RATE		19200000

#define IPQ807x_NPU_CPR_TCSR_START		6
#define IPQ807x_NPU_CPR_TCSR_END		7

#define NPU_TSENS				5

u32 g_valid_npu_fuse_count = IPQ807x_NPU_FUSE_CORNERS;
/**
 * struct cpr3_ipq807x_npu_fuses - NPU specific fuse data for IPQ807x
 * @init_voltage:	Initial (i.e. open-loop) voltage fuse parameter value
 *			for each fuse corner (raw, not converted to a voltage)
 * This struct holds the values for all of the fuses read from memory.
 */
struct cpr3_ipq807x_npu_fuses {
	u64	init_voltage[IPQ807x_NPU_FUSE_CORNERS];
};

/*
 * Constants which define the name of each fuse corner.
 */
enum cpr3_ipq807x_npu_fuse_corner {
	CPR3_IPQ807x_NPU_FUSE_CORNER_NOM	= 0,
	CPR3_IPQ807x_NPU_FUSE_CORNER_TURBO	= 1,
};

static const char * const cpr3_ipq807x_npu_fuse_corner_name[] = {
	[CPR3_IPQ807x_NPU_FUSE_CORNER_NOM]	= "NOM",
	[CPR3_IPQ807x_NPU_FUSE_CORNER_TURBO]	= "TURBO",
};

/*
 * IPQ807x NPU fuse parameter locations:
 *
 * Structs are organized with the following dimensions:
 *	Outer: 0 to 1 for fuse corners from lowest to highest corner
 *	Inner: large enough to hold the longest set of parameter segments which
 *		fully defines a fuse parameter, +1 (for NULL termination).
 *		Each segment corresponds to a contiguous group of bits from a
 *		single fuse row.  These segments are concatentated together in
 *		order to form the full fuse parameter value.  The segments for
 *		a given parameter may correspond to different fuse rows.
 */
static struct cpr3_fuse_param
ipq807x_npu_init_voltage_param[IPQ807x_NPU_FUSE_CORNERS][2] = {
	{{73, 22, 27}, {} },
	{{73, 16, 21}, {} },
};

/*
 * Open loop voltage fuse reference voltages in microvolts for IPQ807x
 */
static int
ipq807x_npu_fuse_ref_volt [IPQ807x_NPU_FUSE_CORNERS] = {
	912000,
	992000,
};

/*
 * IPQ9574 (Few parameters are changed, remaining are same as IPQ807x)
 */
#define IPQ9574_NPU_FUSE_CORNERS		2
#define IPQ9574_NPU_FUSE_STEP_VOLT		10000
#define IPQ9574_NPU_CPR_CLOCK_RATE		24000000

/*
 * fues parameters for IPQ9574
 */
static struct cpr3_fuse_param
ipq9574_npu_init_voltage_param[IPQ9574_NPU_FUSE_CORNERS][2] = {
	{{105, 12, 17}, {} },
	{{105,  6, 11}, {} },
};

/*
 * Open loop voltage fuse reference voltages in microvolts for IPQ9574
 */
static int
ipq9574_npu_fuse_ref_volt [IPQ9574_NPU_FUSE_CORNERS] = {
	862500,
	987500,
};

struct cpr3_controller *g_ctrl;

void cpr3_npu_temp_notify(int sensor, int temp, int low_notif)
{
	u32 prev_sensor_state;

	if (sensor != NPU_TSENS)
		return;

	prev_sensor_state = g_ctrl->cur_sensor_state;
	if (low_notif)
		g_ctrl->cur_sensor_state |= BIT(sensor);
	else
		g_ctrl->cur_sensor_state &= ~BIT(sensor);

	if (!prev_sensor_state && g_ctrl->cur_sensor_state)
		cpr3_handle_temp_open_loop_adjustment(g_ctrl, true);
	else if (prev_sensor_state && !g_ctrl->cur_sensor_state)
		cpr3_handle_temp_open_loop_adjustment(g_ctrl, false);
}

/**
 * cpr3_ipq807x_npu_read_fuse_data() - load NPU specific fuse parameter values
 * @vreg:		Pointer to the CPR3 regulator
 *
 * This function allocates a cpr3_ipq807x_npu_fuses struct, fills it with
 * values read out of hardware fuses, and finally copies common fuse values
 * into the CPR3 regulator struct.
 *
 * Return: 0 on success, errno on failure
 */
static int cpr3_ipq807x_npu_read_fuse_data(struct cpr3_regulator *vreg)
{
	void __iomem *base = vreg->thread->ctrl->fuse_base;
	struct cpr3_ipq807x_npu_fuses *fuse;
	int i, rc;

	fuse = devm_kzalloc(vreg->thread->ctrl->dev, sizeof(*fuse), GFP_KERNEL);
	if (!fuse)
		return -ENOMEM;

	for (i = 0; i < g_valid_npu_fuse_count; i++) {
		rc = cpr3_read_fuse_param(base,
					  vreg->cpr3_regulator_data->init_voltage_param[i],
					  &fuse->init_voltage[i]);
		if (rc) {
			cpr3_err(vreg, "Unable to read fuse-corner %d initial voltage fuse, rc=%d\n",
				 i, rc);
			return rc;
		}
	}

	vreg->fuse_corner_count	= g_valid_npu_fuse_count;
	vreg->platform_fuses	= fuse;

	return 0;
}

/**
 * cpr3_npu_parse_corner_data() - parse NPU corner data from device tree
 *		properties of the CPR3 regulator's device node
 * @vreg:		Pointer to the CPR3 regulator
 *
 * Return: 0 on success, errno on failure
 */
static int cpr3_npu_parse_corner_data(struct cpr3_regulator *vreg)
{
	int rc;

	rc = cpr3_parse_common_corner_data(vreg);
	if (rc) {
		cpr3_err(vreg, "error reading corner data, rc=%d\n", rc);
		return rc;
	}

	return rc;
}

/**
 * cpr3_ipq807x_npu_calculate_open_loop_voltages() - calculate the open-loop
 *		voltage for each corner of a CPR3 regulator
 * @vreg:		Pointer to the CPR3 regulator
 * @temp_correction:    Temperature based correction
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
static int cpr3_ipq807x_npu_calculate_open_loop_voltages(
			struct cpr3_regulator *vreg, bool temp_correction)
{
	struct cpr3_ipq807x_npu_fuses *fuse = vreg->platform_fuses;
	struct cpr3_controller *ctrl = vreg->thread->ctrl;
	int i, j, rc = 0;
	u64 freq_low, volt_low, freq_high, volt_high;
	int *fuse_volt;
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
			fuse_volt[i] = vreg->cpr3_regulator_data->fuse_ref_volt[i];
		else
			fuse_volt[i] = cpr3_convert_open_loop_voltage_fuse(
				vreg->cpr3_regulator_data->fuse_ref_volt[i],
				vreg->cpr3_regulator_data->fuse_step_volt,
				fuse->init_voltage[i],
				IPQ807x_NPU_VOLTAGE_FUSE_SIZE);

		/* Log fused open-loop voltage values for debugging purposes. */
		cpr3_info(vreg, "fused %8s: open-loop=%7d uV\n",
			  cpr3_ipq807x_npu_fuse_corner_name[i],
			  fuse_volt[i]);
	}

	rc = cpr3_determine_part_type(vreg,
			fuse_volt[CPR3_IPQ807x_NPU_FUSE_CORNER_TURBO]);
	if (rc) {
		cpr3_err(vreg,
			"fused part type detection failed failed, rc=%d\n", rc);
		goto done;
	}

	rc = cpr3_adjust_fused_open_loop_voltages(vreg, fuse_volt);
	if (rc) {
		cpr3_err(vreg,
			"fused open-loop voltage adjustment failed, rc=%d\n",
			rc);
		goto done;
	}
	if (temp_correction) {
		rc = cpr3_determine_temp_base_open_loop_correction(vreg,
								fuse_volt);
		if (rc) {
			cpr3_err(vreg,
				"temp open-loop voltage adj. failed, rc=%d\n",
				rc);
			goto done;
		}
	}

	for (i = 1; i < vreg->fuse_corner_count; i++) {
		if (fuse_volt[i] < fuse_volt[i - 1]) {
			cpr3_info(vreg,
				"fuse corner %d voltage=%d uV < fuse corner %d \
				voltage=%d uV; overriding: fuse corner %d \
				voltage=%d\n",
				  i, fuse_volt[i], i - 1, fuse_volt[i - 1],
				  i, fuse_volt[i - 1]);
			fuse_volt[i] = fuse_volt[i - 1];
		}
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
			cpr3_err(vreg,
				"open-loop voltage adjustment failed, rc=%d\n",
				 rc);
	}

	kfree(fuse_volt);
	kfree(fmax_corner);
	return rc;
}

/**
 * cpr3_npu_print_settings() - print out NPU CPR configuration settings into
 *		the kernel log for debugging purposes
 * @vreg:		Pointer to the CPR3 regulator
 */
static void cpr3_npu_print_settings(struct cpr3_regulator *vreg)
{
	struct cpr3_corner *corner;
	int i;

	cpr3_debug(vreg,
		"Corner: Frequency (Hz), Fuse Corner, Floor (uV), \
		Open-Loop (uV), Ceiling (uV)\n");
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
 * cpr3_ipq807x_npu_calc_temp_based_ol_voltages() - Calculate the open loop
 * voltages based on temperature based correction margins
 * @vreg:               Pointer to the CPR3 regulator
 */

static int
cpr3_ipq807x_npu_calc_temp_based_ol_voltages(struct cpr3_regulator *vreg,
						bool temp_correction)
{
	int rc, i;

	rc = cpr3_ipq807x_npu_calculate_open_loop_voltages(vreg,
							temp_correction);
	if (rc) {
		cpr3_err(vreg,
			"unable to calculate open-loop voltages, rc=%d\n", rc);
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

	for (i = 0; i < vreg->corner_count; i++) {
		if (temp_correction)
			vreg->corner[i].cold_temp_open_loop_volt =
				vreg->corner[i].open_loop_volt;
		else
			vreg->corner[i].normal_temp_open_loop_volt =
				vreg->corner[i].open_loop_volt;
	}

	cpr3_npu_print_settings(vreg);

	return rc;
}

/**
 * cpr3_npu_init_thread() - perform steps necessary to initialize the
 *		configuration data for a CPR3 thread
 * @thread:		Pointer to the CPR3 thread
 *
 * Return: 0 on success, errno on failure
 */
static int cpr3_npu_init_thread(struct cpr3_thread *thread)
{
	int rc;

	rc = cpr3_parse_common_thread_data(thread);
	if (rc) {
		cpr3_err(thread->ctrl,
			"thread %u CPR thread data from DT- failed, rc=%d\n",
			 thread->thread_id, rc);
		return rc;
	}

	return 0;
}

/**
 * cpr3_npu_init_regulator() - perform all steps necessary to initialize the
 *		configuration data for a CPR3 regulator
 * @vreg:		Pointer to the CPR3 regulator
 *
 * Return: 0 on success, errno on failure
 */
static int cpr3_npu_init_regulator(struct cpr3_regulator *vreg)
{
	struct cpr3_ipq807x_npu_fuses *fuse;
	int rc, cold_temp = 0;
	bool can_adj_cold_temp = cpr3_can_adjust_cold_temp(vreg);

	rc = cpr3_ipq807x_npu_read_fuse_data(vreg);
	if (rc) {
		cpr3_err(vreg, "unable to read CPR fuse data, rc=%d\n", rc);
		return rc;
	}

	fuse = vreg->platform_fuses;

	rc = cpr3_npu_parse_corner_data(vreg);
	if (rc) {
		cpr3_err(vreg,
			"Cannot read CPR corner data from DT, rc=%d\n", rc);
		return rc;
	}

	rc = cpr3_mem_acc_init(vreg);
	if (rc) {
		if (rc != -EPROBE_DEFER)
			cpr3_err(vreg,
			"Cannot initialize mem-acc regulator settings, rc=%d\n",
			 rc);
		return rc;
	}

	if (can_adj_cold_temp) {
		rc = cpr3_ipq807x_npu_calc_temp_based_ol_voltages(vreg, true);
		if (rc) {
			cpr3_err(vreg,
			"unable to calculate open-loop voltages, rc=%d\n", rc);
			return rc;
		}
	}

	rc = cpr3_ipq807x_npu_calc_temp_based_ol_voltages(vreg, false);
	if (rc) {
		cpr3_err(vreg,
			"unable to calculate open-loop voltages, rc=%d\n", rc);
		return rc;
	}

	if (can_adj_cold_temp) {
		cpr3_info(vreg,
		"Normal and Cold condition init done. Default to normal.\n");

		rc = cpr3_get_cold_temp_threshold(vreg, &cold_temp);
		if (rc) {
			cpr3_err(vreg,
			"Get cold temperature threshold failed, rc=%d\n", rc);
			return rc;
		}
		register_low_temp_notif(NPU_TSENS, cold_temp,
							cpr3_npu_temp_notify);
	}

	return rc;
}

/**
 * cpr3_npu_init_controller() - perform NPU CPR3 controller specific
 *		initializations
 * @ctrl:		Pointer to the CPR3 controller
 *
 * Return: 0 on success, errno on failure
 */
static int cpr3_npu_init_controller(struct cpr3_controller *ctrl)
{
	int rc;

	rc = cpr3_parse_open_loop_common_ctrl_data(ctrl);
	if (rc) {
		if (rc != -EPROBE_DEFER)
			cpr3_err(ctrl, "unable to parse common controller data, rc=%d\n",
				 rc);
		return rc;
	}

	ctrl->ctrl_type = CPR_CTRL_TYPE_CPR3;
	ctrl->supports_hw_closed_loop = false;

	return 0;
}

static const struct cpr3_reg_data ipq807x_cpr_npu = {
	.cpr_valid_fuse_count = IPQ807x_NPU_FUSE_CORNERS,
	.init_voltage_param = ipq807x_npu_init_voltage_param,
	.fuse_ref_volt = ipq807x_npu_fuse_ref_volt,
	.fuse_step_volt = IPQ807x_NPU_FUSE_STEP_VOLT,
	.cpr_clk_rate = IPQ807x_NPU_CPR_CLOCK_RATE,
};

static const struct cpr3_reg_data ipq817x_cpr_npu = {
	.cpr_valid_fuse_count = IPQ817x_NPU_FUSE_CORNERS,
	.init_voltage_param = ipq807x_npu_init_voltage_param,
	.fuse_ref_volt = ipq807x_npu_fuse_ref_volt,
	.fuse_step_volt = IPQ807x_NPU_FUSE_STEP_VOLT,
	.cpr_clk_rate = IPQ807x_NPU_CPR_CLOCK_RATE,
};

static const struct cpr3_reg_data ipq9574_cpr_npu = {
	.cpr_valid_fuse_count = IPQ9574_NPU_FUSE_CORNERS,
	.init_voltage_param = ipq9574_npu_init_voltage_param,
	.fuse_ref_volt = ipq9574_npu_fuse_ref_volt,
	.fuse_step_volt = IPQ9574_NPU_FUSE_STEP_VOLT,
	.cpr_clk_rate = IPQ9574_NPU_CPR_CLOCK_RATE,
};

static struct of_device_id cpr3_regulator_match_table[] = {
	{
		.compatible = "qcom,cpr3-ipq807x-npu-regulator",
		.data = &ipq807x_cpr_npu
	},
	{
		.compatible = "qcom,cpr3-ipq817x-npu-regulator",
		.data = &ipq817x_cpr_npu
	},
	{
		.compatible = "qcom,cpr3-ipq9574-npu-regulator",
		.data = &ipq9574_cpr_npu
	},
	{}
};

static int cpr3_npu_regulator_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cpr3_controller *ctrl;
	int i, rc;
	const struct of_device_id *match;
	struct cpr3_reg_data *cpr_data;

	if (!dev->of_node) {
		dev_err(dev, "Device tree node is missing\n");
		return -EINVAL;
	}

	ctrl = devm_kzalloc(dev, sizeof(*ctrl), GFP_KERNEL);
	if (!ctrl)
		return -ENOMEM;
	g_ctrl = ctrl;

	match = of_match_device(cpr3_regulator_match_table, &pdev->dev);
	if (!match)
		return -ENODEV;

	cpr_data = (struct cpr3_reg_data *)match->data;
	g_valid_npu_fuse_count = cpr_data->cpr_valid_fuse_count;
	dev_info(dev, "NPU CPR valid fuse count: %d\n", g_valid_npu_fuse_count);
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

	rc = cpr3_read_tcsr_setting(ctrl, pdev, IPQ807x_NPU_CPR_TCSR_START,
				    IPQ807x_NPU_CPR_TCSR_END);
	if (rc) {
		cpr3_err(ctrl, "could not read CPR tcsr rsetting\n");
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

	rc = cpr3_npu_init_controller(ctrl);
	if (rc) {
		if (rc != -EPROBE_DEFER)
			cpr3_err(ctrl, "failed to initialize CPR controller parameters, rc=%d\n",
				 rc);
		return rc;
	}

	rc = cpr3_npu_init_thread(&ctrl->thread[0]);
	if (rc) {
		cpr3_err(ctrl, "thread initialization failed, rc=%d\n", rc);
		return rc;
	}

	for (i = 0; i < ctrl->thread[0].vreg_count; i++) {
		ctrl->thread[0].vreg[i].cpr3_regulator_data = cpr_data;
		rc = cpr3_npu_init_regulator(&ctrl->thread[0].vreg[i]);
		if (rc) {
			cpr3_err(&ctrl->thread[0].vreg[i], "regulator initialization failed, rc=%d\n",
				 rc);
			return rc;
		}
	}

	platform_set_drvdata(pdev, ctrl);

	return cpr3_open_loop_regulator_register(pdev, ctrl);
}

static int cpr3_npu_regulator_remove(struct platform_device *pdev)
{
	struct cpr3_controller *ctrl = platform_get_drvdata(pdev);

	return cpr3_open_loop_regulator_unregister(ctrl);
}

static struct platform_driver cpr3_npu_regulator_driver = {
	.driver		= {
		.name		= "qcom,cpr3-npu-regulator",
		.of_match_table	= cpr3_regulator_match_table,
		.owner		= THIS_MODULE,
	},
	.probe		= cpr3_npu_regulator_probe,
	.remove		= cpr3_npu_regulator_remove,
};

static int cpr3_regulator_init(void)
{
	return platform_driver_register(&cpr3_npu_regulator_driver);
}
arch_initcall(cpr3_regulator_init);

static void cpr3_regulator_exit(void)
{
	platform_driver_unregister(&cpr3_npu_regulator_driver);
}
module_exit(cpr3_regulator_exit);

MODULE_DESCRIPTION("QCOM CPR3 NPU regulator driver");
MODULE_LICENSE("Dual BSD/GPLv2");
MODULE_ALIAS("platform:npu-ipq807x");
