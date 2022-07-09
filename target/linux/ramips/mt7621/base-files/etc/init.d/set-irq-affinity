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

	for irq in $(grep -E "mt|ra" /proc/interrupts | cut -d: -f1 | sed 's, *,,')
	do
		echo "$mask" > "/proc/irq/$irq/smp_affinity"
		[ $mask = 4 ] && mask=8
	done

	[ -e "/sys/class/net/eth0/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/eth0/queues/rx-0/rps_cpus"
	[ -e "/sys/class/net/ra0/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/ra0/queues/rx-0/rps_cpus"
	[ -e "/sys/class/net/rai0/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/rai0/queues/rx-0/rps_cpus"
	[ -e "/sys/class/net/rax0/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/rax0/queues/rx-0/rps_cpus"
	[ -e "/sys/class/net/apcli0/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/apcli0/queues/rx-0/rps_cpus"
	[ -e "/sys/class/net/apclix0/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/apclix0/queues/rx-0/rps_cpus"
	[ -e "/sys/class/net/apclii0/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/apclii0/queues/rx-0/rps_cpus"
	[ -e "/sys/class/net/wlan0/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/wlan0/queues/rx-0/rps_cpus"
	[ -e "/sys/class/net/wlan1/queues/rx-0/rps_cpus" ] && echo "6" > "/sys/class/net/wlan1/queues/rx-0/rps_cpus"
	echo "8" > "/proc/irq/23/smp_affinity"
	echo "8" > "/proc/irq/24/smp_affinity"
}
