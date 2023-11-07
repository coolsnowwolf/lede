#!/bin/sh
RESET_GPIO=95

if [ "1A" ==  "${1}A" ]; then
    active_low=1
else
    active_low=0
fi

echo ${RESET_GPIO} > /sys/class/gpio/export > /dev/null
echo ${active_low} > /sys/class/gpio/gpio${RESET_GPIO}/active_low
echo out > /sys/class/gpio/gpio${RESET_GPIO}/direction
echo 1 > /sys/class/gpio/gpio${RESET_GPIO}/value
sleep 2
echo 0 > /sys/class/gpio/gpio${RESET_GPIO}/value
