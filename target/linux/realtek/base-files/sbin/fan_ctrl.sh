#!/bin/sh

PSU_TEMP=$(cut -c1-2 /sys/class/hwmon/hwmon0/temp1_input)

FAN_CTRL='/sys/class/hwmon/hwmon0/pwm1'

PSU_THRESH=51000

if [ "$PSU_TEMP" -ge "$PSU_THRESH" ];then
	echo "250" > $FAN_CTRL
else
	echo "156" > $FAN_CTRL
fi
