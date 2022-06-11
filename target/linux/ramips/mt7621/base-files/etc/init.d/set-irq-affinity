#!/bin/sh /etc/rc.common

START=99

start() {
	if grep -q 'processor.*: 2' /proc/cpuinfo; then
		mask=4
	elif grep -q 'processor.*: 1' /proc/cpuinfo; then
		mask=2
	else
		return
	fi

	for irq in $(grep "mt76..e" /proc/interrupts | cut -d: -f1 | sed 's, *,,')
	do
		echo "$mask" > "/proc/irq/$irq/smp_affinity"
		[ $mask = 4 ] && mask=8
	done
}
