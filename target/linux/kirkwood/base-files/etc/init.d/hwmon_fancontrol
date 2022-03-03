#!/bin/sh /etc/rc.common

START=98

boot() {
	# configuring onboard temp/fan controller to run the fan on its own
	# for more information, please read https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface

	case $(board_name) in
	ctera,c200-v1)
		path_to_hwmon='/sys/devices/platform/ocp@f1000000/f1011000.i2c/i2c-0/0-004c/hwmon/hwmon0'

		# It should be related to hdd temerature instead lm63 temp
		echo 1 > "$path_to_hwmon/pwm1_enable"
		echo 128 > "$path_to_hwmon/pwm1"
		;;
	iom,ix2-200)
		path_to_hwmon='/sys/class/hwmon/hwmon0'
		echo 2 > "$path_to_hwmon/pwm1_enable" # fan is on pwm1
		;;
	netgear,readynas-duo-v2)
		path_to_hwmon='/sys/class/hwmon/hwmon0'
		echo 1200 > "$path_to_hwmon/fan1_target" # set target rpm
		;;
	seagate,blackarmor-nas220)
		path_to_hwmon='/sys/devices/platform/ocp@f1000000/f1011000.i2c/i2c-0/0-002e/hwmon/hwmon0'
		# adt7476 fan control chip. 3 temp sensors. Set to 1/4 speed at 35C and max speed at 48C.
		echo 7 > "$path_to_hwmon/pwm1_auto_channels_temp"
		echo 64 > "$path_to_hwmon/pwm1_auto_point1_pwm"
		echo 255 > "$path_to_hwmon/pwm1_auto_point2_pwm"
		echo 35000 > "$path_to_hwmon/temp1_auto_point1_temp"
		echo 48000 > "$path_to_hwmon/temp1_auto_point2_temp"
		echo 35000 > "$path_to_hwmon/temp2_auto_point1_temp"
		echo 48000 > "$path_to_hwmon/temp2_auto_point2_temp"
		echo 35000 > "$path_to_hwmon/temp3_auto_point1_temp"
		echo 48000 > "$path_to_hwmon/temp3_auto_point2_temp"
		echo 2 > "$path_to_hwmon/pwm1_enable"
		;;
	zyxel,nsa310b)
		path_to_hwmon='/sys/devices/platform/ocp@f1000000/f1011000.i2c/i2c-0/0-002e/hwmon/hwmon0'
		# use the max. value of (temp1) OR (temp2) OR (temp3) as an input
		# for the PWM of the cooling fan
		echo 123 > "$path_to_hwmon/pwm1_auto_channels"
		# Temperature sensor #1 placed on mainboard
		echo 30000 > "$path_to_hwmon/temp1_auto_temp_min"
		echo 49600 > "$path_to_hwmon/temp1_auto_temp_max"
		# Temperature sensor #2 placed on mainboard
		# range: 0 to 127000 in steps of 1000 [millicelsius]
		echo 30000 > "$path_to_hwmon/temp2_auto_temp_min"
		# range: 0 to 127000 in steps of ???? [millicelsius]
		echo 49600 > "$path_to_hwmon/temp2_auto_temp_max"
		# Temperature sensor #3 placed close to a chipset
		# range: 0 to 60000 in steps of 1000 [millicelsius]
		echo 23000 > "$path_to_hwmon/temp3_auto_temp_min"
		# pre-defined steps: 103000, 122000, 143300, 170000 in [millicelsius]
		echo 103000 > "$path_to_hwmon/temp3_auto_temp_max"
		;;
	esac
}
