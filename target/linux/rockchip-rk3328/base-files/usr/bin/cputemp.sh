#!/bin/bash

while true
do
echo "-----------------"
echo "Curr freq: "
cat /sys/devices/system/cpu/cpu[04]/cpufreq/cpuinfo_cur_freq
echo "Temp: "
cat /sys/class/thermal/thermal_zone0/temp
sleep 5
done
