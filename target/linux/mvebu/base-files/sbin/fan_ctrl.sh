#!/bin/sh

CPU_TEMP=`cut -c1-2 /sys/class/hwmon/hwmon2/temp1_input`
DDR_TEMP=`cut -c1-2 /sys/class/hwmon/hwmon1/temp1_input`
WIFI_TEMP=`cut -c1-2 /sys/class/hwmon/hwmon1/temp2_input`

CPU_LOW=85
CPU_HIGH=95
DDR_LOW=65
DDR_HIGH=75
WIFI_LOW=100
WIFI_HIGH=115

if [ -d /sys/devices/pwm_fan ];then
	FAN_CTRL=/sys/devices/pwm_fan/hwmon/hwmon0/pwm1
elif [ -d /sys/devices/platform/pwm_fan ];then
	FAN_CTRL=/sys/devices/platform/pwm_fan/hwmon/hwmon0/pwm1
else
	exit 0
fi

if [ "$CPU_TEMP" -ge "$CPU_HIGH" -o "$DDR_TEMP" -ge "$DDR_HIGH" -o "$WIFI_TEMP" -ge "$WIFI_HIGH" ];then
	echo "255" > $FAN_CTRL
elif [ "$CPU_TEMP" -ge "$CPU_LOW" -o "$DDR_TEMP" -ge "$DDR_LOW" -o "$WIFI_TEMP" -ge "$WIFI_LOW" ];then
	echo "100" > $FAN_CTRL
else
	echo "0" > $FAN_CTRL
fi
