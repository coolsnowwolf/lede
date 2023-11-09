#!/bin/sh
#usage: usb_modem_reset.sh reset_gpio active_low waittime
#       usb_modem_reset.sh 95         0          2
reset_gpio=95
active_low=0
wait_time=2

if [ "A" != "${1}A" ]; then
    reset_gpio=${1}
fi

if [ "A" != "${2}A" ]; then
    active_low=${2}
fi

if [ "A" != "${3}A" ]; then
    wait_time=${3}
fi

echo ${reset_gpio} > /sys/class/gpio/export 2> /dev/null
echo ${active_low} > /sys/class/gpio/gpio${reset_gpio}/active_low
echo out > /sys/class/gpio/gpio${reset_gpio}/direction
echo 1 > /sys/class/gpio/gpio${reset_gpio}/value
sleep ${wait_time}
echo 0 > /sys/class/gpio/gpio${reset_gpio}/value
