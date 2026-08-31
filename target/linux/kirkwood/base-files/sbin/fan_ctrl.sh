#!/bin/sh

CPU_TEMP=$(cut -c1-2 /sys/devices/platform/ocp@f1000000/f1011000.i2c/i2c-0/0-0048/hwmon/hwmon?/temp1_input)

CPU_LOW=45
CPU_MID=50
CPU_HIGH=55

if [ ! -e /sys/devices/platform/ocp@f1000000/f1011000.i2c/i2c-0/0-001b/hwmon/hwmon?/pwm1 ]; then
	exit 0
else
	FAN_CTRL=$(ls /sys/devices/platform/ocp@f1000000/f1011000.i2c/i2c-0/0-001b/hwmon/hwmon?/pwm1)
fi

if [ "$CPU_TEMP" -ge "$CPU_HIGH" ]; then
	echo "255" > $FAN_CTRL
elif [ "$CPU_TEMP" -ge "$CPU_MID" ]; then
	echo "100" > $FAN_CTRL
elif [ "$CPU_TEMP" -ge "$CPU_LOW" ]; then
	echo "50" > $FAN_CTRL
else
	echo "0" > $FAN_CTRL
fi
