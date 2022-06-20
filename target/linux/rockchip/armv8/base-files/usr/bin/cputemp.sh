#!/bin/bash

while true
do
echo "-----------------"
echo "Curr freq: `cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq`,`cat /sys/devices/system/cpu/cpu4/cpufreq/cpuinfo_cur_freq`"
echo "Temp: `cat /sys/class/thermal/thermal_zone0/temp`"
if [ -d /sys/devices/platform/pwm-fan ]; then
    CUR=`cat /sys/devices/virtual/thermal/cooling_device0/cur_state`
    MAX=`cat /sys/devices/virtual/thermal/cooling_device0/max_state`
    echo "Fan Level: ${CUR}/${MAX}"
fi
sleep 5
done
