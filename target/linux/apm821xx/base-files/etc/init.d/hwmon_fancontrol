#!/bin/sh /etc/rc.common

START=98

boot() {
	# configuring onboard temp/fan controller to run the fan on its own
	# for more information, please read https://www.kernel.org/doc/Documentation/hwmon/sysfs-interface

	case $(board_name) in
	netgear,wndr4700)
		path_to_hwmon='/sys/devices/platform/plb/plb:opb/4ef600700.i2c/i2c-0/0-001b/hwmon/hwmon1'
		echo 1 > "$path_to_hwmon/pwm1_enable"
		;;
	esac
}
