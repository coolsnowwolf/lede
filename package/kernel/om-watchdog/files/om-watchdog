#!/bin/sh

GPIO=$1

trap "" INT HUP

echo $GPIO > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio${GPIO}/direction

while true; do
	echo 1 > /sys/class/gpio/gpio${GPIO}/value
	sleep 1
	echo 0 > /sys/class/gpio/gpio${GPIO}/value
	sleep 180
done
