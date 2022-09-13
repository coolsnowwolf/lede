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

#ifndef __REGULATOR_CPR3_REGULATOR_H__
#define __REGULATOR_CPR3_REGULATOR_H__

#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/power/qcom/apm.h>
#include <linux/regulator/driver.h>

struct cpr3_controller;
struct cpr3_thread;

/**
 * struct cpr3_fuse_param - defines one contiguous segment of a fuse parameter
 *			    that is contained within a given row.
 * @row:	Fuse row number
 * @bit_start:	The first bit within the row of the fuse parameter segment
 * @bit_end:	The last bit within the row of the fuse parameter segment
 *
 * Each fuse row is 64 bits in length.  bit_start and bit_end may take values
 * from 0 to 63.  bit_start must be less than or equal to bit_end.
 */
struct cpr3_fuse_param {
	unsigned		row;
	unsigned		bit_start;
	unsigned		bit_end;
};

/* Each CPR3 sensor has 16 ring oscillators */
#define CPR3_RO_COUNT		16

/* The maximum number of sensors that can be present on a single CPR loop. */
#define CPR3_MAX_SENSOR_COUNT	256

/* This constant is used when allocating array printing buffers. */
#define MAX_CHARS_PER_INT	10

/**
 * struct cpr4_sdelta - CPR4 controller specific data structure for the sdelta
 *			adjustment table which is used to adjust the VDD supply
 *			voltage automatically based upon the temperature and/or
 *			the number of online CPU cores.
 * @allow_core_count_adj: Core count adjustments are allowed.
 * @allow_temp_adj:	Temperature based adjustments are allowed.
 * @max_core_count:	Maximum number of cores considered for core count
 *			adjustment logic.
 * @temp_band_count:	Number of temperature bands considered for temperature
 *			based adjustment logic.
 * @cap_volt:		CAP in uV to apply to SDELTA margins with multiple
 *			cpr3-regulators defined for single controller.
 * @table:		SDELTA table with per-online-core and temperature based
 *			adjustments of size (max_core_count * temp_band_count)
 *			Outer: core count
 *			Inner: temperature band
 *			Each element has units of VDD supply steps. Positive
 *			values correspond to a reduction in voltage and negative
 *			value correspond to an increase (this follows the SDELTA
 *			register semantics).
 * @allow_boost:	Voltage boost allowed.
 * @boost_num_cores:	The number of online cores at which the boost voltage
 *			adjustments will be applied
 * @boost_table:	SDELTA table with boost voltage adjustments of size
 *			temp_band_count. Each element has units of VDD supply
 *			steps. Positive values correspond to a reduction in
 *			voltage and negative value correspond to an increase
 *			(this follows the SDELTA register semantics).
 */
struct cpr4_sdelta {
	bool	allow_core_count_adj;
	bool	allow_temp_adj;
	int	max_core_count;
	int	temp_band_count;
	int	cap_volt;
	int	*table;
	bool	allow_boost;
	int	boost_num_cores;
	int	*boost_table;
};

/**
 * struct cpr3_corner - CPR3 virtual voltage corner data structure
 * @floor_volt:		CPR closed-loop floor voltage in microvolts
 * @ceiling_volt:	CPR closed-loop ceiling voltage in microvolts
 * @open_loop_volt:	CPR open-loop voltage (i.e. initial voltage) in
 *			microvolts
 * @last_volt:		Last known settled CPR closed-loop voltage which is used
 *			when switching to a new corner
 * @abs_ceiling_volt:	The absolute CPR closed-loop ceiling voltage in
 *			microvolts.  This is used to limit the ceiling_volt
 *			value when it is increased as a result of aging
 *			adjustment.
 * @unaged_floor_volt:	The CPR closed-loop floor voltage in microvolts before
 *			any aging adjustment is performed
 * @unaged_ceiling_volt: The CPR closed-loop ceiling voltage in microvolts
 *			before any aging adjustment is performed
 * @unaged_open_loop_volt: The CPR open-loop voltage (i.e. initial voltage) in
 *			microvolts before any aging adjusment is performed
 * @system_volt:	The system-supply voltage in microvolts or corners or
 *			levels
 * @mem_acc_volt:	The mem-acc-supply voltage in corners
 * @proc_freq:		Processor frequency in Hertz. For CPR rev. 3 and 4
 *			conrollers, this field is only used by platform specific
 *			CPR3 driver for interpolation. For CPRh-compliant
 *			controllers, this frequency is also utilized by the
 *			clock driver to determine the corner to CPU clock
 *			frequency mappings.
 * @cpr_fuse_corner:	Fused corner index associated with this virtual corner
 *			(only used by platform specific CPR3 driver for
 *			mapping purposes)
 * @target_quot:	Array of target quotient values to use for each ring
 *			oscillator (RO) for this corner.  A value of 0 should be
 *			specified as the target quotient for each RO that is
 *			unused by this corner.
 * @ro_scale:		Array of CPR ring oscillator (RO) scaling factors.  The
 *			scaling factor for each RO is defined from RO0 to RO15
 *			with units of QUOT/V.  A value of 0 may be specified for
 *			an RO that is unused.
 * @ro_mask:		Bitmap where each of the 16 LSBs indicate if the
 *			corresponding ROs should be masked for this corner
 * @irq_en:		Bitmap of the CPR interrupts to enable for this corner
 * @aging_derate:	The amount to derate the aging voltage adjustment
 *			determined for the reference corner in units of uV/mV.
 *			E.g. a value of 900 would imply that the adjustment for
 *			this corner should be 90% (900/1000) of that for the
 *			reference corner.
 * @use_open_loop:	Boolean indicating that open-loop (i.e CPR disabled) as
 *			opposed to closed-loop operation must be used for this
 *			corner on CPRh controllers.
 * @sdelta:		The CPR4 controller specific data for this corner. This
 *			field is applicable for CPR4 controllers.
 *
 * The value of last_volt is initialized inside of the cpr3_regulator_register()
 * call with the open_loop_volt value.  It can later be updated to the settled
 * VDD supply voltage.  The values for unaged_floor_volt, unaged_ceiling_volt,
 * and unaged_open_loop_volt are initialized inside of cpr3_regulator_register()
 * if ctrl->aging_required == true.  These three values must be pre-initialized
 * if cpr3_regulator_register() is called with ctrl->aging_required == false and
 * ctrl->aging_succeeded == true.
 *
 * The values of ro_mask and irq_en are initialized inside of the
 * cpr3_regulator_register() call.
 */
struct cpr3_corner {
	int			floor_volt;
	int			ceiling_volt;
	int			cold_temp_open_loop_volt;
	int			normal_temp_open_loop_volt;
	int			open_loop_volt;
	int			last_volt;
	int			abs_ceiling_volt;
	int			unaged_floor_volt;
	int			unaged_ceiling_volt;
	int			unaged_open_loop_volt;
	int			system_volt;
	int			mem_acc_volt;
	u32			proc_freq;
	int			cpr_fuse_corner;
	u32			target_quot[CPR3_RO_COUNT];
	u32			ro_scale[CPR3_RO_COUNT];
	u32			ro_mask;
	u32			irq_en;
	int			aging_derate;
	bool			use_open_loop;
	struct cpr4_sdelta	*sdelta;
};

/**
 * struct cprh_corner_band - CPRh controller specific data structure which
 *			encapsulates the range of corners and the SDELTA
 *			adjustment table to be applied to the corners within
 *			the min and max bounds of the corner band.
 * @corner:		Corner number which defines the corner band boundary
 * @sdelta:		The SDELTA adjustment table which contains core-count
 *			and temp based margin adjustments that are applicable
 *			to the corner band.
 */
struct cprh_corner_band {
	int			corner;
	struct cpr4_sdelta	*sdelta;
};

/**
 * struct cpr3_fuse_parameters - CPR4 fuse specific data structure which has
 * 			the required fuse parameters need for Close Loop CPR
 * @(*apss_ro_sel_param)[2]:       Pointer to RO select fuse details
 * @(*apss_init_voltage_param)[2]: Pointer to Target voltage fuse details
 * @(*apss_target_quot_param)[2]:  Pointer to Target quot fuse details
 * @(*apss_quot_offset_param)[2]:  Pointer to quot offset fuse details
 * @cpr_fusing_rev_param:          Pointer to CPR revision fuse details
 * @apss_speed_bin_param:          Pointer to Speed bin fuse details
 * @cpr_boost_fuse_cfg_param:      Pointer to Boost fuse cfg details
 * @apss_boost_fuse_volt_param:    Pointer to Boost fuse volt details
 * @misc_fuse_volt_adj_param:      Pointer to Misc fuse volt fuse details
 */
struct cpr3_fuse_parameters {
	struct cpr3_fuse_param (*apss_ro_sel_param)[2];
	struct cpr3_fuse_param (*apss_init_voltage_param)[2];
	struct cpr3_fuse_param (*apss_target_quot_param)[2];
	struct cpr3_fuse_param (*apss_quot_offset_param)[2];
	struct cpr3_fuse_param *cpr_fusing_rev_param;
	struct cpr3_fuse_param *apss_speed_bin_param;
	struct cpr3_fuse_param *cpr_boost_fuse_cfg_param;
	struct cpr3_fuse_param *apss_boost_fuse_volt_param;
	struct cpr3_fuse_param *misc_fuse_volt_adj_param;
};

struct cpr4_mem_acc_func {
	void (*set_mem_acc)(struct regulator_dev *);
	void (*clear_mem_acc)(struct regulator_dev *);
};

/**
 * struct cpr4_reg_data - CPR4 regulator specific data structure which is
 * target specific
 * @cpr_valid_fuse_count: Number of valid fuse corners
 * @fuse_ref_volt: 	  Pointer to fuse reference voltage
 * @fuse_step_volt: 	  CPR step voltage available in fuse
 * @cpr_clk_rate: 	  CPR clock rate
 * @boost_fuse_ref_volt:  Boost fuse reference voltage
 * @boost_ceiling_volt:   Boost ceiling voltage
 * @boost_floor_volt: 	  Boost floor voltage
 * @cpr3_fuse_params:     Pointer to CPR fuse parameters
 * @mem_acc_funcs:        Pointer to MEM ACC set/clear functions
 **/
struct cpr4_reg_data {
	u32 cpr_valid_fuse_count;
	int *fuse_ref_volt;
	u32 fuse_step_volt;
	u32 cpr_clk_rate;
	int boost_fuse_ref_volt;
	int boost_ceiling_volt;
	int boost_floor_volt;
	struct cpr3_fuse_parameters *cpr3_fuse_params;
	struct cpr4_mem_acc_func *mem_acc_funcs;
};
/**
 * struct cpr3_reg_data - CPR3 regulator specific data structure which is
 * target specific
 * @cpr_valid_fuse_count: Number of valid fuse corners
 * @(*init_voltage_param)[2]: Pointer to Target voltage fuse details
 * @fuse_ref_volt: 	  Pointer to fuse reference voltage
 * @fuse_step_volt: 	  CPR step voltage available in fuse
 * @cpr_clk_rate: 	  CPR clock rate
 * @cpr3_fuse_params:     Pointer to CPR fuse parameters
 **/
struct cpr3_reg_data {
	u32 cpr_valid_fuse_count;
	struct cpr3_fuse_param (*init_voltage_param)[2];
	int *fuse_ref_volt;
	u32 fuse_step_volt;
	u32 cpr_clk_rate;
};

/**
 * struct cpr3_regulator - CPR3 logical regulator instance associated with a
 *			given CPR3 hardware thread
 * @of_node:		Device node associated with the device tree child node
 *			of this CPR3 regulator
 * @thread:		Pointer to the CPR3 thread which manages this CPR3
 *			regulator
 * @name:		Unique name for this CPR3 regulator which is filled
 *			using the device tree regulator-name property
 * @rdesc:		Regulator description for this CPR3 regulator
 * @rdev:		Regulator device pointer for the regulator registered
 *			for this CPR3 regulator
 * @mem_acc_regulator:	Pointer to the optional mem-acc supply regulator used
 *			to manage memory circuitry settings based upon CPR3
 *			regulator output voltage.
 * @corner:		Array of all corners supported by this CPR3 regulator
 * @corner_count:	The number of elements in the corner array
 * @corner_band:	Array of all corner bands supported by CPRh compatible
 *			controllers
 * @cpr4_regulator_data Target specific cpr4 regulator data
 * @cpr3_regulator_data Target specific cpr3 regulator data
 * @corner_band_count:	The number of elements in the corner band array
 * @platform_fuses:	Pointer to platform specific CPR fuse data (only used by
 *			platform specific CPR3 driver)
 * @speed_bin_fuse:	Value read from the speed bin fuse parameter
 * @speed_bins_supported: The number of speed bins supported by the device tree
 *			configuration for this CPR3 regulator
 * @cpr_rev_fuse:	Value read from the CPR fusing revision fuse parameter
 * @fuse_combo:		Platform specific enum value identifying the specific
 *			combination of fuse values found on a given chip
 * @fuse_combos_supported: The number of fuse combinations supported by the
 *			device tree configuration for this CPR3 regulator
 * @fuse_corner_count:	Number of corners defined by fuse parameters
 * @fuse_corner_map:	Array of length fuse_corner_count which specifies the
 *			highest corner associated with each fuse corner.  Note
 *			that each element must correspond to a valid corner
 *			and that element values must be strictly increasing.
 *			Also, it is acceptable for the lowest fuse corner to map
 *			to a corner other than the lowest.  Likewise, it is
 *			acceptable for the highest fuse corner to map to a
 *			corner other than the highest.
 * @fuse_combo_corner_sum: The sum of the corner counts across all fuse combos
 * @fuse_combo_offset:	The device tree property array offset for the selected
 *			fuse combo
 * @speed_bin_corner_sum: The sum of the corner counts across all speed bins
 *			This may be specified as 0 if per speed bin parsing
 *			support is not required.
 * @speed_bin_offset:	The device tree property array offset for the selected
 *			speed bin
 * @fuse_combo_corner_band_sum: The sum of the corner band counts across all
 *			fuse combos
 * @fuse_combo_corner_band_offset: The device tree property array offset for
 *			the corner band count corresponding to the selected
 *			fuse combo
 * @speed_bin_corner_band_sum: The sum of the corner band counts across all
 *			speed bins. This may be specified as 0 if per speed bin
 *			parsing support is not required
 * @speed_bin_corner_band_offset: The device tree property array offset for the
 *			corner band count corresponding to the selected speed
 *			bin
 * @pd_bypass_mask:	Bit mask of power domains associated with this CPR3
 *			regulator
 * @dynamic_floor_corner: Index identifying the voltage corner for the CPR3
 *			regulator whose last_volt value should be used as the
 *			global CPR floor voltage if all of the power domains
 *			associated with this CPR3 regulator are bypassed
 * @uses_dynamic_floor: Boolean flag indicating that dynamic_floor_corner should
 *			be utilized for the CPR3 regulator
 * @current_corner:	Index identifying the currently selected voltage corner
 *			for the CPR3 regulator or less than 0 if no corner has
 *			been requested
 * @last_closed_loop_corner: Index identifying the last voltage corner for the
 *			CPR3 regulator which was configured when operating in
 *			CPR closed-loop mode or less than 0 if no corner has
 *			been requested.  CPR registers are only written to when
 *			using closed-loop mode.
 * @aggregated:		Boolean flag indicating that this CPR3 regulator
 *			participated in the last aggregation event
 * @debug_corner:	Index identifying voltage corner used for displaying
 *			corner configuration values in debugfs
 * @vreg_enabled:	Boolean defining the enable state of the CPR3
 *			regulator's regulator within the regulator framework.
 * @aging_allowed:	Boolean defining if CPR aging adjustments are allowed
 *			for this CPR3 regulator given the fuse combo of the
 *			device
 * @aging_allow_open_loop_adj: Boolean defining if the open-loop voltage of each
 *			corner of this regulator should be adjusted as a result
 *			of an aging measurement.  This flag can be set to false
 *			when the open-loop voltage adjustments have been
 *			specified such that they include the maximum possible
 *			aging adjustment.  This flag is only used if
 *			aging_allowed == true.
 * @aging_corner:	The corner that should be configured for this regulator
 *			when an aging measurement is performed.
 * @aging_max_adjust_volt: The maximum aging voltage margin in microvolts that
 *			may be added to the target quotients of this regulator.
 *			A value of 0 may be specified if this regulator does not
 *			require any aging adjustment.
 * @allow_core_count_adj: Core count adjustments are allowed for this regulator.
 * @allow_temp_adj:	Temperature based adjustments are allowed for this
 *			regulator.
 * @max_core_count:	Maximum number of cores considered for core count
 *			adjustment logic.
 * @allow_boost:	Voltage boost allowed for this regulator.
 *
 * This structure contains both configuration and runtime state data.  The
 * elements current_corner, last_closed_loop_corner, aggregated, debug_corner,
 * and vreg_enabled are state variables.
 */
struct cpr3_regulator {
	struct device_node	*of_node;
	struct cpr3_thread	*thread;
	const char		*name;
	struct regulator_desc	rdesc;
	struct regulator_dev	*rdev;
	struct regulator	*mem_acc_regulator;
	struct cpr3_corner	*corner;
	int			corner_count;
	struct cprh_corner_band *corner_band;
	struct cpr4_reg_data    *cpr4_regulator_data;
	struct cpr3_reg_data    *cpr3_regulator_data;
	u32			corner_band_count;

	void			*platform_fuses;
	int			speed_bin_fuse;
	int			speed_bins_supported;
	int			cpr_rev_fuse;
	int			part_type;
	int			part_type_supported;
	int			fuse_combo;
	int			fuse_combos_supported;
	int			fuse_corner_count;
	int			*fuse_corner_map;
	int			fuse_combo_corner_sum;
	int			fuse_combo_offset;
	int			speed_bin_corner_sum;
	int			speed_bin_offset;
	int			fuse_combo_corner_band_sum;
	int			fuse_combo_corner_band_offset;
	int			speed_bin_corner_band_sum;
	int			speed_bin_corner_band_offset;
	u32			pd_bypass_mask;
	int			dynamic_floor_corner;
	bool			uses_dynamic_floor;

	int			current_corner;
	int			last_closed_loop_corner;
	bool			aggregated;
	int			debug_corner;
	bool			vreg_enabled;

	bool			aging_allowed;
	bool			aging_allow_open_loop_adj;
	int			aging_corner;
	int			aging_max_adjust_volt;

	bool			allow_core_count_adj;
	bool			allow_temp_adj;
	int			max_core_count;
	bool			allow_boost;
};

/**
 * struct cpr3_thread - CPR3 hardware thread data structure
 * @thread_id:		Hardware thread ID
 * @of_node:		Device node associated with the device tree child node
 *			of this CPR3 thread
 * @ctrl:		Pointer to the CPR3 controller which manages this thread
 * @vreg:		Array of CPR3 regulators handled by the CPR3 thread
 * @vreg_count:		Number of elements in the vreg array
 * @aggr_corner:	CPR corner containing the in process aggregated voltage
 *			and target quotient configurations which will be applied
 * @last_closed_loop_aggr_corner: CPR corner containing the most recent
 *			configurations which were written into hardware
 *			registers when operating in closed loop mode (i.e. with
 *			CPR enabled)
 * @consecutive_up:	The number of consecutive CPR step up events needed to
 *			to trigger an up interrupt
 * @consecutive_down:	The number of consecutive CPR step down events needed to
 *			to trigger a down interrupt
 * @up_threshold:	The number CPR error steps required to generate an up
 *			event
 * @down_threshold:	The number CPR error steps required to generate a down
 *			event
 *
 * This structure contains both configuration and runtime state data.  The
 * elements aggr_corner and last_closed_loop_aggr_corner are state variables.
 */
struct cpr3_thread {
	u32			thread_id;
	struct device_node	*of_node;
	struct cpr3_controller	*ctrl;
	struct cpr3_regulator	*vreg;
	int			vreg_count;
	struct cpr3_corner	aggr_corner;
	struct cpr3_corner	last_closed_loop_aggr_corner;

	u32			consecutive_up;
	u32			consecutive_down;
	u32			up_threshold;
	u32			down_threshold;
};

/* Per CPR controller data */
/**
 * enum cpr3_mem_acc_corners - Constants which define the number of mem-acc
 *		regulator corners available in the mem-acc corner map array.
 * %CPR3_MEM_ACC_LOW_CORNER:	Index in mem-acc corner map array mapping to the
 *				mem-acc regulator corner
 *				to be used for low voltage vdd supply
 * %CPR3_MEM_ACC_HIGH_CORNER:	Index in mem-acc corner map array mapping to the
 *				mem-acc regulator corner to be used for high
 *				voltage vdd supply
 * %CPR3_MEM_ACC_CORNERS:	Number of elements in the mem-acc corner map
 *				array
 */
enum cpr3_mem_acc_corners {
	CPR3_MEM_ACC_LOW_CORNER		= 0,
	CPR3_MEM_ACC_HIGH_CORNER	= 1,
	CPR3_MEM_ACC_CORNERS		= 2,
};

/**
 * enum cpr3_count_mode - CPR3 controller count mode which defines the
 *		method that CPR sensor data is acquired
 * %CPR3_COUNT_MODE_ALL_AT_ONCE_MIN:	Capture all CPR sensor readings
 *					simultaneously and report the minimum
 *					value seen in successive measurements
 * %CPR3_COUNT_MODE_ALL_AT_ONCE_MAX:	Capture all CPR sensor readings
 *					simultaneously and report the maximum
 *					value seen in successive measurements
 * %CPR3_COUNT_MODE_STAGGERED:		Read one sensor at a time in a
 *					sequential fashion
 * %CPR3_COUNT_MODE_ALL_AT_ONCE_AGE:	Capture all CPR aging sensor readings
 *					simultaneously.
 */
enum cpr3_count_mode {
	CPR3_COUNT_MODE_ALL_AT_ONCE_MIN	= 0,
	CPR3_COUNT_MODE_ALL_AT_ONCE_MAX	= 1,
	CPR3_COUNT_MODE_STAGGERED	= 2,
	CPR3_COUNT_MODE_ALL_AT_ONCE_AGE	= 3,
};

/**
 * enum cpr_controller_type - supported CPR controller hardware types
 * %CPR_CTRL_TYPE_CPR3:	HW has CPR3 controller
 * %CPR_CTRL_TYPE_CPR4:	HW has CPR4 controller
 */
enum cpr_controller_type {
	CPR_CTRL_TYPE_CPR3,
	CPR_CTRL_TYPE_CPR4,
};

/**
 * cpr_setting - supported CPR global settings
 * %CPR_DEFAULT: default mode from dts will be used
 * %CPR_DISABLED: ceiling voltage will be used for all the corners
 * %CPR_OPEN_LOOP_EN: CPR will work in OL
 * %CPR_CLOSED_LOOP_EN: CPR will work in CL, if supported
 */
enum cpr_setting {
	CPR_DEFAULT		= 0,
	CPR_DISABLED		= 1,
	CPR_OPEN_LOOP_EN	= 2,
	CPR_CLOSED_LOOP_EN	= 3,
};

/**
 * struct cpr3_aging_sensor_info - CPR3 aging sensor information
 * @sensor_id		The index of the CPR3 sensor to be used in the aging
 *			measurement.
 * @ro_scale		The CPR ring oscillator (RO) scaling factor for the
 *			aging sensor with units of QUOT/V.
 * @init_quot_diff:	The fused quotient difference between aged and un-aged
 *			paths that was measured at manufacturing time.
 * @measured_quot_diff: The quotient difference measured at runtime.
 * @bypass_mask:	Bit mask of the CPR sensors that must be bypassed during
 *			the aging measurement for this sensor
 *
 * This structure contains both configuration and runtime state data.  The
 * element measured_quot_diff is a state variable.
 */
struct cpr3_aging_sensor_info {
	u32			sensor_id;
	u32			ro_scale;
	int			init_quot_diff;
	int			measured_quot_diff;
	u32			bypass_mask[CPR3_MAX_SENSOR_COUNT / 32];
};

/**
 * struct cpr3_reg_info - Register information data structure
 * @name:	Register name
 * @addr:	Register physical address
 * @value:	Register content
 * @virt_addr:	Register virtual address
 *
 * This data structure is used to dump some critical register contents
 * when the device crashes due to a kernel panic.
 */
struct cpr3_reg_info {
	const char	*name;
	u32		addr;
	u32		value;
	void __iomem	*virt_addr;
};

/**
 * struct cpr3_panic_regs_info - Data structure to dump critical register
 *		contents.
 * @reg_count:		Number of elements in the regs array
 * @regs:		Array of critical registers information
 *
 * This data structure is used to dump critical register contents when
 * the device crashes due to a kernel panic.
 */
struct cpr3_panic_regs_info {
	int			reg_count;
	struct cpr3_reg_info	*regs;
};

/**
 * struct cpr3_controller - CPR3 controller data structure
 * @dev:		Device pointer for the CPR3 controller device
 * @name:		Unique name for the CPR3 controller
 * @ctrl_id:		Controller ID corresponding to the VDD supply number
 *			that this CPR3 controller manages.
 * @cpr_ctrl_base:	Virtual address of the CPR3 controller base register
 * @fuse_base:		Virtual address of fuse row 0
 * @aging_possible_reg:	Virtual address of an optional platform-specific
 *			register that must be ready to determine if it is
 *			possible to perform an aging measurement.
 * @list:		list head used in a global cpr3-regulator list so that
 *			cpr3-regulator structs can be found easily in RAM dumps
 * @thread:		Array of CPR3 threads managed by the CPR3 controller
 * @thread_count:	Number of elements in the thread array
 * @sensor_owner:	Array of thread IDs indicating which thread owns a given
 *			CPR sensor
 * @sensor_count:	The number of CPR sensors found on the CPR loop managed
 *			by this CPR controller.  Must be equal to the number of
 *			elements in the sensor_owner array
 * @soc_revision:	Revision number of the SoC.  This may be unused by
 *			platforms that do not have different behavior for
 *			different SoC revisions.
 * @lock:		Mutex lock used to ensure mutual exclusion between
 *			all of the threads associated with the controller
 * @vdd_regulator:	Pointer to the VDD supply regulator which this CPR3
 *			controller manages
 * @system_regulator:	Pointer to the optional system-supply regulator upon
 *			which the VDD supply regulator depends.
 * @mem_acc_regulator:	Pointer to the optional mem-acc supply regulator used
 *			to manage memory circuitry settings based upon the
 *			VDD supply output voltage.
 * @vdd_limit_regulator: Pointer to the VDD supply limit regulator which is used
 *			for hardware closed-loop in order specify ceiling and
 *			floor voltage limits (platform specific)
 * @system_supply_max_volt: Voltage in microvolts which corresponds to the
 *			absolute ceiling voltage of the system-supply
 * @mem_acc_threshold_volt: mem-acc threshold voltage in microvolts
 * @mem_acc_corner_map: mem-acc regulator corners mapping to low and high
 *			voltage mem-acc settings for the memories powered by
 *			this CPR3 controller and its associated CPR3 regulators
 * @mem_acc_crossover_volt: Voltage in microvolts corresponding to the voltage
 *			that the VDD supply must be set to while a MEM ACC
 *			switch is in progress. This element must be initialized
 *			for CPRh controllers when a MEM ACC threshold voltage is
 *			defined.
 * @core_clk:		Pointer to the CPR3 controller core clock
 * @iface_clk:		Pointer to the CPR3 interface clock (platform specific)
 * @bus_clk:		Pointer to the CPR3 bus clock (platform specific)
 * @irq:		CPR interrupt number
 * @irq_affinity_mask:	The cpumask for the CPUs which the CPR interrupt should
 *			have affinity for
 * @cpu_hotplug_notifier: CPU hotplug notifier used to reset IRQ affinity when a
 *			CPU is brought back online
 * @ceiling_irq:	Interrupt number for the interrupt that is triggered
 *			when hardware closed-loop attempts to exceed the ceiling
 *			voltage
 * @apm:		Handle to the array power mux (APM)
 * @apm_threshold_volt:	Voltage in microvolts which defines the threshold
 *			voltage to determine the APM supply selection for
 *			each corner
 * @apm_crossover_volt:	Voltage in microvolts corresponding to the voltage that
 *			the VDD supply must be set to while an APM switch is in
 *			progress. This element must be initialized for CPRh
 *			controllers when an APM threshold voltage is defined
 * @apm_adj_volt:	Minimum difference between APM threshold voltage and
 *			open-loop voltage which allows the APM threshold voltage
 *			to be used as a ceiling
 * @apm_high_supply:	APM supply to configure if VDD voltage is greater than
 *			or equal to the APM threshold voltage
 * @apm_low_supply:	APM supply to configure if the VDD voltage is less than
 *			the APM threshold voltage
 * @base_volt:		Minimum voltage in microvolts supported by the VDD
 *			supply managed by this CPR controller
 * @corner_switch_delay_time: The delay time in nanoseconds used by the CPR
 *			controller to wait for voltage settling before
 *			acknowledging the OSM block after corner changes
 * @cpr_clock_rate:	CPR reference clock frequency in Hz.
 * @sensor_time:	The time in nanoseconds that each sensor takes to
 *			perform a measurement.
 * @loop_time:		The time in nanoseconds between consecutive CPR
 *			measurements.
 * @up_down_delay_time: The time to delay in nanoseconds between consecutive CPR
 *			measurements when the last measurement recommended
 *			increasing or decreasing the vdd-supply voltage.
 *			(platform specific)
 * @idle_clocks:	Number of CPR reference clock ticks that the CPR
 *			controller waits in transitional states.
 * @step_quot_init_min:	The default minimum CPR step quotient value.  The step
 *			quotient is the number of additional ring oscillator
 *			ticks observed when increasing one step in vdd-supply
 *			output voltage.
 * @step_quot_init_max:	The default maximum CPR step quotient value.
 * @step_volt:		Step size in microvolts between available set points
 *			of the VDD supply
 * @down_error_step_limit: CPR4 hardware closed-loop down error step limit which
 *			defines the maximum number of VDD supply regulator steps
 *			that the voltage may be reduced as the result of a
 *			single CPR measurement.
 * @up_error_step_limit: CPR4 hardware closed-loop up error step limit which
 *			defines the maximum number of VDD supply regulator steps
 *			that the voltage may be increased as the result of a
 *			single CPR measurement.
 * @count_mode:		CPR controller count mode
 * @count_repeat:	Number of times to perform consecutive sensor
 *			measurements when using all-at-once count modes.
 * @proc_clock_throttle: Defines the processor clock frequency throttling
 *			register value to use.  This can be used to reduce the
 *			clock frequency when a power domain exits a low power
 *			mode until CPR settles at a new voltage.
 *			(platform specific)
 * @cpr_allowed_hw:	Boolean which indicates if closed-loop CPR operation is
 *			permitted for a given chip based upon hardware fuse
 *			values
 * @cpr_allowed_sw:	Boolean which indicates if closed-loop CPR operation is
 *			permitted based upon software policies
 * @supports_hw_closed_loop: Boolean which indicates if this CPR3/4 controller
 *			physically supports hardware closed-loop CPR operation
 * @use_hw_closed_loop:	Boolean which indicates that this controller will be
 *			using hardware closed-loop operation in place of
 *			software closed-loop operation.
 * @ctrl_type:		CPR controller type
 * @saw_use_unit_mV:	Boolean which indicates the unit used in SAW PVC
 *			interface is mV.
 * @aggr_corner:	CPR corner containing the most recently aggregated
 *			voltage configurations which are being used currently
 * @cpr_enabled:	Boolean which indicates that the CPR controller is
 *			enabled and operating in closed-loop mode.  CPR clocks
 *			have been prepared and enabled whenever this flag is
 *			true.
 * @last_corner_was_closed_loop: Boolean indicating if the last known corners
 *			were updated during closed loop operation.
 * @cpr_suspended:	Boolean which indicates that CPR has been temporarily
 *			disabled while enterring system suspend.
 * @debugfs:		Pointer to the debugfs directory of this CPR3 controller
 * @aging_ref_volt:	Reference voltage in microvolts to configure when
 *			performing CPR aging measurements.
 * @aging_vdd_mode:	vdd-supply regulator mode to configure before performing
 *			a CPR aging measurement.  It should be one of
 *			REGULATOR_MODE_*.
 * @aging_complete_vdd_mode: vdd-supply regulator mode to configure after
 *			performing a CPR aging measurement.  It should be one of
 *			REGULATOR_MODE_*.
 * @aging_ref_adjust_volt: The reference aging voltage margin in microvolts that
 *			should be added to the target quotients of the
 *			regulators managed by this controller after derating.
 * @aging_required:	Flag which indicates that a CPR aging measurement still
 *			needs to be performed for this CPR3 controller.
 * @aging_succeeded:	Flag which indicates that a CPR aging measurement has
 *			completed successfully.
 * @aging_failed:	Flag which indicates that a CPR aging measurement has
 *			failed to complete successfully.
 * @aging_sensor:	Array of CPR3 aging sensors which are used to perform
 *			aging measurements at a runtime.
 * @aging_sensor_count:	Number of elements in the aging_sensor array
 * @aging_possible_mask: Optional bitmask used to mask off the
 *			aging_possible_reg register.
 * @aging_possible_val:	Optional value that the masked aging_possible_reg
 *			register must have in order for a CPR aging measurement
 *			to be possible.
 * @step_quot_fixed:	Fixed step quotient value used for target quotient
 *			adjustment if use_dynamic_step_quot is not set.
 *			This parameter is only relevant for CPR4 controllers
 *			when using the per-online-core or per-temperature
 *			adjustments.
 * @initial_temp_band:	Temperature band used for calculation of base-line
 *			target quotients (fused).
 * @use_dynamic_step_quot: Boolean value which indicates that margin adjustment
 *			of target quotient will be based on the step quotient
 *			calculated dynamically in hardware for each RO.
 * @allow_core_count_adj: Core count adjustments are allowed for this controller
 * @allow_temp_adj:	Temperature based adjustments are allowed for
 *			this controller
 * @allow_boost:	Voltage boost allowed for this controller.
 * @temp_band_count:	Number of temperature bands used for temperature based
 *			adjustment logic
 * @temp_points:	Array of temperature points in decidegrees Celsius used
 *			to specify the ranges for selected temperature bands.
 *			The array must have (temp_band_count - 1) elements
 *			allocated.
 * @temp_sensor_id_start: Start ID of temperature sensors used for temperature
 *			based adjustments.
 * @temp_sensor_id_end:	End ID of temperature sensors used for temperature
 *			based adjustments.
 * @voltage_settling_time: The time in nanoseconds that it takes for the
 *			VDD supply voltage to settle after being increased or
 *			decreased by step_volt microvolts which is used when
 *			SDELTA voltage margin adjustments are applied.
 * @cpr_global_setting:	Global setting for this CPR controller
 * @panic_regs_info:	Array of panic registers information which provides the
 *			list of registers to dump when the device crashes.
 * @panic_notifier:	Notifier block registered to global panic notifier list.
 *
 * This structure contains both configuration and runtime state data.  The
 * elements cpr_allowed_sw, use_hw_closed_loop, aggr_corner, cpr_enabled,
 * last_corner_was_closed_loop, cpr_suspended, aging_ref_adjust_volt,
 * aging_required, aging_succeeded, and aging_failed are state variables.
 *
 * The apm* elements do not need to be initialized if the VDD supply managed by
 * the CPR3 controller does not utilize an APM.
 *
 * The elements step_quot_fixed, initial_temp_band, allow_core_count_adj,
 * allow_temp_adj and temp* need to be initialized for CPR4 controllers which
 * are using per-online-core or per-temperature adjustments.
 */
struct cpr3_controller {
	struct device		*dev;
	const char		*name;
	int			ctrl_id;
	void __iomem		*cpr_ctrl_base;
	void __iomem		*fuse_base;
	void __iomem		*aging_possible_reg;
	struct list_head	list;
	struct cpr3_thread	*thread;
	int			thread_count;
	u8			*sensor_owner;
	int			sensor_count;
	int			soc_revision;
	struct mutex		lock;
	struct regulator	*vdd_regulator;
	struct regulator	*system_regulator;
	struct regulator	*mem_acc_regulator;
	struct regulator	*vdd_limit_regulator;
	int			system_supply_max_volt;
	int			mem_acc_threshold_volt;
	int			mem_acc_corner_map[CPR3_MEM_ACC_CORNERS];
	int			mem_acc_crossover_volt;
	struct clk		*core_clk;
	struct clk		*iface_clk;
	struct clk		*bus_clk;
	int			irq;
	struct cpumask		irq_affinity_mask;
	struct notifier_block	cpu_hotplug_notifier;
	int			ceiling_irq;
	struct msm_apm_ctrl_dev *apm;
	int			apm_threshold_volt;
	int			apm_crossover_volt;
	int			apm_adj_volt;
	enum msm_apm_supply	apm_high_supply;
	enum msm_apm_supply	apm_low_supply;
	int			base_volt;
	u32			corner_switch_delay_time;
	u32			cpr_clock_rate;
	u32			sensor_time;
	u32			loop_time;
	u32			up_down_delay_time;
	u32			idle_clocks;
	u32			step_quot_init_min;
	u32			step_quot_init_max;
	int			step_volt;
	u32			down_error_step_limit;
	u32			up_error_step_limit;
	enum cpr3_count_mode	count_mode;
	u32			count_repeat;
	u32			proc_clock_throttle;
	bool			cpr_allowed_hw;
	bool			cpr_allowed_sw;
	bool			supports_hw_closed_loop;
	bool			use_hw_closed_loop;
	enum cpr_controller_type ctrl_type;
	bool			saw_use_unit_mV;
	struct cpr3_corner	aggr_corner;
	bool			cpr_enabled;
	bool			last_corner_was_closed_loop;
	bool			cpr_suspended;
	struct dentry		*debugfs;

	int			aging_ref_volt;
	unsigned int		aging_vdd_mode;
	unsigned int		aging_complete_vdd_mode;
	int			aging_ref_adjust_volt;
	bool			aging_required;
	bool			aging_succeeded;
	bool			aging_failed;
	struct cpr3_aging_sensor_info *aging_sensor;
	int			aging_sensor_count;
	u32			cur_sensor_state;
	u32			aging_possible_mask;
	u32			aging_possible_val;

	u32			step_quot_fixed;
	u32			initial_temp_band;
	bool			use_dynamic_step_quot;
	bool			allow_core_count_adj;
	bool			allow_temp_adj;
	bool			allow_boost;
	int			temp_band_count;
	int			*temp_points;
	u32			temp_sensor_id_start;
	u32			temp_sensor_id_end;
	u32			voltage_settling_time;
	enum cpr_setting	cpr_global_setting;
	struct cpr3_panic_regs_info *panic_regs_info;
	struct notifier_block	panic_notifier;
};

/* Used for rounding voltages to the closest physically available set point. */
#define CPR3_ROUND(n, d) (DIV_ROUND_UP(n, d) * (d))

#define cpr3_err(cpr3_thread, message, ...) \
	pr_err("%s: " message, (cpr3_thread)->name, ##__VA_ARGS__)
#define cpr3_info(cpr3_thread, message, ...) \
	pr_info("%s: " message, (cpr3_thread)->name, ##__VA_ARGS__)
#define cpr3_debug(cpr3_thread, message, ...) \
	pr_debug("%s: " message, (cpr3_thread)->name, ##__VA_ARGS__)

/*
 * Offset subtracted from voltage corner values passed in from the regulator
 * framework in order to get internal voltage corner values.  This is needed
 * since the regulator framework treats 0 as an error value at regulator
 * registration time.
 */
#define CPR3_CORNER_OFFSET	1

#ifdef CONFIG_REGULATOR_CPR3

int cpr3_regulator_register(struct platform_device *pdev,
			struct cpr3_controller *ctrl);
int cpr3_open_loop_regulator_register(struct platform_device *pdev,
				      struct cpr3_controller *ctrl);
int cpr3_regulator_unregister(struct cpr3_controller *ctrl);
int cpr3_open_loop_regulator_unregister(struct cpr3_controller *ctrl);
int cpr3_regulator_suspend(struct cpr3_controller *ctrl);
int cpr3_regulator_resume(struct cpr3_controller *ctrl);

int cpr3_allocate_threads(struct cpr3_controller *ctrl, u32 min_thread_id,
			u32 max_thread_id);
int cpr3_map_fuse_base(struct cpr3_controller *ctrl,
			struct platform_device *pdev);
int cpr3_read_tcsr_setting(struct cpr3_controller *ctrl,
			   struct platform_device *pdev, u8 start, u8 end);
int cpr3_read_fuse_param(void __iomem *fuse_base_addr,
			const struct cpr3_fuse_param *param, u64 *param_value);
int cpr3_convert_open_loop_voltage_fuse(int ref_volt, int step_volt, u32 fuse,
			int fuse_len);
u64 cpr3_interpolate(u64 x1, u64 y1, u64 x2, u64 y2, u64 x);
int cpr3_parse_array_property(struct cpr3_regulator *vreg,
			const char *prop_name, int tuple_size, u32 *out);
int cpr3_parse_corner_array_property(struct cpr3_regulator *vreg,
			const char *prop_name, int tuple_size, u32 *out);
int cpr3_parse_corner_band_array_property(struct cpr3_regulator *vreg,
			const char *prop_name, int tuple_size, u32 *out);
int cpr3_parse_common_corner_data(struct cpr3_regulator *vreg);
int cpr3_parse_thread_u32(struct cpr3_thread *thread, const char *propname,
			u32 *out_value, u32 value_min, u32 value_max);
int cpr3_parse_ctrl_u32(struct cpr3_controller *ctrl, const char *propname,
			u32 *out_value, u32 value_min, u32 value_max);
int cpr3_parse_common_thread_data(struct cpr3_thread *thread);
int cpr3_parse_common_ctrl_data(struct cpr3_controller *ctrl);
int cpr3_parse_open_loop_common_ctrl_data(struct cpr3_controller *ctrl);
int cpr3_limit_open_loop_voltages(struct cpr3_regulator *vreg);
void cpr3_open_loop_voltage_as_ceiling(struct cpr3_regulator *vreg);
int cpr3_limit_floor_voltages(struct cpr3_regulator *vreg);
void cpr3_print_quots(struct cpr3_regulator *vreg);
int cpr3_determine_part_type(struct cpr3_regulator *vreg, int fuse_volt);
int cpr3_determine_temp_base_open_loop_correction(struct cpr3_regulator *vreg,
			int *fuse_volt);
int cpr3_adjust_fused_open_loop_voltages(struct cpr3_regulator *vreg,
			int *fuse_volt);
int cpr3_adjust_open_loop_voltages(struct cpr3_regulator *vreg);
int cpr3_quot_adjustment(int ro_scale, int volt_adjust);
int cpr3_voltage_adjustment(int ro_scale, int quot_adjust);
int cpr3_parse_closed_loop_voltage_adjustments(struct cpr3_regulator *vreg,
			u64 *ro_sel, int *volt_adjust,
			int *volt_adjust_fuse, int *ro_scale);
int cpr4_parse_core_count_temp_voltage_adj(struct cpr3_regulator *vreg,
			bool use_corner_band);
int cpr3_apm_init(struct cpr3_controller *ctrl);
int cpr3_mem_acc_init(struct cpr3_regulator *vreg);
void cprh_adjust_voltages_for_apm(struct cpr3_regulator *vreg);
void cprh_adjust_voltages_for_mem_acc(struct cpr3_regulator *vreg);
int cpr3_adjust_target_quotients(struct cpr3_regulator *vreg,
			int *fuse_volt_adjust);
int cpr3_handle_temp_open_loop_adjustment(struct cpr3_controller *ctrl,
			bool is_cold);
int cpr3_get_cold_temp_threshold(struct cpr3_regulator *vreg, int *cold_temp);
bool cpr3_can_adjust_cold_temp(struct cpr3_regulator *vreg);

#else

static inline int cpr3_regulator_register(struct platform_device *pdev,
			struct cpr3_controller *ctrl)
{
	return -ENXIO;
}

static inline int
cpr3_open_loop_regulator_register(struct platform_device *pdev,
				  struct cpr3_controller *ctrl);
{
	return -ENXIO;
}

static inline int cpr3_regulator_unregister(struct cpr3_controller *ctrl)
{
	return -ENXIO;
}

static inline int
cpr3_open_loop_regulator_unregister(struct cpr3_controller *ctrl)
{
	return -ENXIO;
}

static inline int cpr3_regulator_suspend(struct cpr3_controller *ctrl)
{
	return -ENXIO;
}

static inline int cpr3_regulator_resume(struct cpr3_controller *ctrl)
{
	return -ENXIO;
}

static inline int cpr3_get_thread_name(struct cpr3_thread *thread,
			struct device_node *thread_node)
{
	return -EPERM;
}

static inline int cpr3_allocate_threads(struct cpr3_controller *ctrl,
			u32 min_thread_id, u32 max_thread_id)
{
	return -EPERM;
}

static inline int cpr3_map_fuse_base(struct cpr3_controller *ctrl,
			struct platform_device *pdev)
{
	return -ENXIO;
}

static inline int cpr3_read_tcsr_setting(struct cpr3_controller *ctrl,
			   struct platform_device *pdev, u8 start, u8 end)
{
	return 0;
}

static inline int cpr3_read_fuse_param(void __iomem *fuse_base_addr,
			const struct cpr3_fuse_param *param, u64 *param_value)
{
	return -EPERM;
}

static inline int cpr3_convert_open_loop_voltage_fuse(int ref_volt,
			int step_volt, u32 fuse, int fuse_len)
{
	return -EPERM;
}

static inline u64 cpr3_interpolate(u64 x1, u64 y1, u64 x2, u64 y2, u64 x)
{
	return 0;
}

static inline int cpr3_parse_array_property(struct cpr3_regulator *vreg,
			const char *prop_name, int tuple_size, u32 *out)
{
	return -EPERM;
}

static inline int cpr3_parse_corner_array_property(struct cpr3_regulator *vreg,
			const char *prop_name, int tuple_size, u32 *out)
{
	return -EPERM;
}

static inline int cpr3_parse_corner_band_array_property(
			struct cpr3_regulator *vreg, const char *prop_name,
			int tuple_size, u32 *out)
{
	return -EPERM;
}

static inline int cpr3_parse_common_corner_data(struct cpr3_regulator *vreg)
{
	return -EPERM;
}

static inline int cpr3_parse_thread_u32(struct cpr3_thread *thread,
			const char *propname, u32 *out_value, u32 value_min,
			u32 value_max)
{
	return -EPERM;
}

static inline int cpr3_parse_ctrl_u32(struct cpr3_controller *ctrl,
			const char *propname, u32 *out_value, u32 value_min,
			u32 value_max)
{
	return -EPERM;
}

static inline int cpr3_parse_common_thread_data(struct cpr3_thread *thread)
{
	return -EPERM;
}

static inline int cpr3_parse_common_ctrl_data(struct cpr3_controller *ctrl)
{
	return -EPERM;
}

static inline int
cpr3_parse_open_loop_common_ctrl_data(struct cpr3_controller *ctrl)
{
	return -EPERM;
}

static inline int cpr3_limit_open_loop_voltages(struct cpr3_regulator *vreg)
{
	return -EPERM;
}

static inline void cpr3_open_loop_voltage_as_ceiling(
			struct cpr3_regulator *vreg)
{
	return;
}

static inline int cpr3_limit_floor_voltages(struct cpr3_regulator *vreg)
{
	return -EPERM;
}

static inline void cpr3_print_quots(struct cpr3_regulator *vreg)
{
	return;
}

static inline int
cpr3_determine_part_type(struct cpr3_regulator *vreg, int fuse_volt)
{
	return -EPERM;
}

static inline int
cpr3_determine_temp_base_open_loop_correction(struct cpr3_regulator *vreg,
			int *fuse_volt)
{
	return -EPERM;
}

static inline int cpr3_adjust_fused_open_loop_voltages(
			struct cpr3_regulator *vreg, int *fuse_volt)
{
	return -EPERM;
}

static inline int cpr3_adjust_open_loop_voltages(struct cpr3_regulator *vreg)
{
	return -EPERM;
}

static inline int cpr3_quot_adjustment(int ro_scale, int volt_adjust)
{
	return 0;
}

static inline int cpr3_voltage_adjustment(int ro_scale, int quot_adjust)
{
	return 0;
}

static inline int cpr3_parse_closed_loop_voltage_adjustments(
			struct cpr3_regulator *vreg, u64 *ro_sel,
			int *volt_adjust, int *volt_adjust_fuse, int *ro_scale)
{
	return 0;
}

static inline int cpr4_parse_core_count_temp_voltage_adj(
			struct cpr3_regulator *vreg, bool use_corner_band)
{
	return 0;
}

static inline int cpr3_apm_init(struct cpr3_controller *ctrl)
{
	return 0;
}

static inline int cpr3_mem_acc_init(struct cpr3_regulator *vreg)
{
	return 0;
}

static inline void cprh_adjust_voltages_for_apm(struct cpr3_regulator *vreg)
{
}

static inline void cprh_adjust_voltages_for_mem_acc(struct cpr3_regulator *vreg)
{
}

static inline int cpr3_adjust_target_quotients(struct cpr3_regulator *vreg,
			int *fuse_volt_adjust)
{
	return 0;
}

static inline int
cpr3_handle_temp_open_loop_adjustment(struct cpr3_controller *ctrl,
			bool is_cold)
{
	return 0;
}

static inline bool
cpr3_can_adjust_cold_temp(struct cpr3_regulator *vreg)
{
	return false;
}

static inline int
cpr3_get_cold_temp_threshold(struct cpr3_regulator *vreg, int *cold_temp)
{
	return 0;
}
#endif /* CONFIG_REGULATOR_CPR3 */

#endif /* __REGULATOR_CPR_REGULATOR_H__ */
