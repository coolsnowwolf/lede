#!/bin/sh /etc/rc.common

START=18

plasmacloud_common() {
	# configuring onboard temp/fan controller to run the fan on its own
	# for more information, please read https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface

	local path_to_hwmon="$(find /sys/bus/i2c/devices/4-002e/hwmon/ -type d -maxdepth 1 -mindepth 1 -name 'hwmon*')"

	# adt7476 fan control chip
	# 3 temp sensors. Set to lowest speed at 35C and max speed at 70C
	# set critical temp to 80 as per the datasheet
	echo 7 > "$path_to_hwmon/pwm2_auto_channels_temp"
	echo 1 > "$path_to_hwmon/pwm2_auto_point1_pwm"
	echo 255 > "$path_to_hwmon/pwm2_auto_point2_pwm"
	echo 22500 > "$path_to_hwmon/pwm2_freq"

	echo 80000 > "$path_to_hwmon/temp1_crit"
	echo 80000 > "$path_to_hwmon/temp2_crit"
	echo 80000 > "$path_to_hwmon/temp3_crit"

	echo 75000 > "$path_to_hwmon/temp1_crit_hyst"
	echo 75000 > "$path_to_hwmon/temp2_crit_hyst"
	echo 75000 > "$path_to_hwmon/temp3_crit_hyst"

	echo 50000 > "$path_to_hwmon/temp1_auto_point1_temp"
	echo 50000 > "$path_to_hwmon/temp2_auto_point1_temp"
	echo 50000 > "$path_to_hwmon/temp3_auto_point1_temp"

	echo 70000 > "$path_to_hwmon/temp1_auto_point2_temp"
	echo 70000 > "$path_to_hwmon/temp2_auto_point2_temp"
	echo 70000 > "$path_to_hwmon/temp3_auto_point2_temp"

	echo 2 > "$path_to_hwmon/pwm2_enable"
}

dlink_dgs_1250() {
	local path_temp_hwmon="$(find /sys/devices/platform/i2c-gpio-4/i2c-4/4-0048/hwmon/ -type d -maxdepth 1 -mindepth 1 -name 'hwmon*')"
	local path_fan_hwmon="$(find /sys/devices/platform/gpio-fan/hwmon/ -type d -maxdepth 1 -mindepth 1 -name 'hwmon*')"

	# Activate fan and let LM75 steer high/low speed via alert pin
	# vendor defaults for DGS-1250-28X are 69C low->high and 57C high->low
	echo 1 > "$path_fan_hwmon/fan1_target"
	echo 69000 > "$path_temp_hwmon/temp1_max"
	echo 57000 > "$path_temp_hwmon/temp1_max_hyst"
}

linksys_lgs328mpc_v2() {
	# The U-Boot of this device does not set up the LM63 fan controller for us, so we do it ourselves:
	# Let LM63 steer high/low speed full automatically, vendor defaults are 40C low->high and 31C high->low.
	# For this, reduce PMW frequency as otherwise the fan won't start at low speeds.
	# Temperature point 1 is set to 1C and a hysteresis of 9C gives 1C - 9C = -8C - most likely always on.
	# Temperature point 2 is set to 40C and a hysteresis of 9C gives 40C - 9C = 31C.
	# Other points must be monotonic for the kernel to accept auto mode.
	local path_to_hwmon="$(find /sys/bus/i2c/devices/0-004c/hwmon/ -type d -maxdepth 1 -mindepth 1 -name 'hwmon*')"

	[ -n "$path_to_hwmon" ] || return 0

	# switch to manual mode, otherwise LUT is not writable
	echo 1 > "$path_to_hwmon/pwm1_enable"
	# Set PWM Frequency to 22Hz as otherwise the fan doesn't spin at low duty cycles.
	echo 22 > "$path_to_hwmon/pwm1_freq"

	echo 1000  > "$path_to_hwmon/pwm1_auto_point1_temp" # 1°C
	echo 128   > "$path_to_hwmon/pwm1_auto_point1_pwm"  # 50% fan speed
	echo 40000 > "$path_to_hwmon/pwm1_auto_point2_temp" # 40°C
	echo 255   > "$path_to_hwmon/pwm1_auto_point2_pwm"  # 100% fan speed
	echo 41000 > "$path_to_hwmon/pwm1_auto_point3_temp" # monotonically increasing entries so the driver accepts this LUT
	echo 255   > "$path_to_hwmon/pwm1_auto_point3_pwm"
	echo 42000 > "$path_to_hwmon/pwm1_auto_point4_temp"
	echo 255   > "$path_to_hwmon/pwm1_auto_point4_pwm"
	echo 43000 > "$path_to_hwmon/pwm1_auto_point5_temp"
	echo 255   > "$path_to_hwmon/pwm1_auto_point5_pwm"
	echo 44000 > "$path_to_hwmon/pwm1_auto_point6_temp"
	echo 255   > "$path_to_hwmon/pwm1_auto_point6_pwm"
	echo 45000 > "$path_to_hwmon/pwm1_auto_point7_temp"
	echo 255   > "$path_to_hwmon/pwm1_auto_point7_pwm"
	echo 46000 > "$path_to_hwmon/pwm1_auto_point8_temp"
	echo 255   > "$path_to_hwmon/pwm1_auto_point8_pwm"

	# set temperature hysteresis to 9K
	echo 9000 > "$path_to_hwmon/pwm1_auto_point_temp_hyst"

	# switch to automatic, LUT driven mode
	echo 2 > "$path_to_hwmon/pwm1_enable"
}

boot() {
	case $(board_name) in
	plasmacloud,esx28|\
	plasmacloud,psx28)
		plasmacloud_common
		;;
	d-link,dgs-1250-28x)
		dlink_dgs_1250
		;;
	linksys,lgs328mpc-v2)
		linksys_lgs328mpc_v2
		;;
	esac
}
