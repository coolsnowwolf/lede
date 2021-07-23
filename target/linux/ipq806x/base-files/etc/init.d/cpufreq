#!/bin/sh /etc/rc.common

START=15

boot() {
  local governor

  governor=$(cat /sys/devices/system/cpu/cpufreq/policy0/scaling_governor)

  if [ "$governor" = "ondemand" ]; then
    # Effective only with ondemand
    echo 600000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq
    echo 600000 > /sys/devices/system/cpu/cpufreq/policy1/scaling_min_freq
    echo 10 > /sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor
    echo 50 > /sys/devices/system/cpu/cpufreq/ondemand/up_threshold
  fi
}
