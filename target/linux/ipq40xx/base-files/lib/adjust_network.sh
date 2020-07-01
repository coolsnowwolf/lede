#!/bin/sh
# this scripts is used for adjust cpu's choice of interrupts.
#

################################################
# Adjust smp_affinity of edma
# Globals:
#    None
# Arguments:
#    None
# Returns:
#    None
# Remark:
#    execute only once on start-up.
################################################
adjust_edma_smp_affinity() {
	grep -q edma_eth_ /proc/interrupts || return 0
	local nr=`cat /proc/cpuinfo | grep processor | wc -l`
	local cpu=0
	local tx_irq_num

	for tx_num in `seq 0 1 15` ; do
		cpu=`printf "%x" $((1<<((tx_num/4+3)%nr)))`
		tx_irq_num=`grep -m1 edma_eth_tx$tx_num /proc/interrupts | cut -d ':' -f 1 | tail -n1 | tr -d ' '`
		[ -n "$tx_irq_num" ] && echo $cpu > /proc/irq/$tx_irq_num/smp_affinity
	done

	for rx_num in `seq 0 1 7` ; do
		cpu=`printf "%x" $((1<<((rx_num/2)%nr)))`
		rx_irq_num=`grep -m1 edma_eth_rx$rx_num /proc/interrupts | cut -d ':' -f 1 | tail -n1 | tr -d ' '`
		[ -n "$rx_irq_num" ] && echo $cpu > /proc/irq/$rx_irq_num/smp_affinity
	done
}

################################################
# Adjust smp_affinity of ath10k for 2G and 5G
# Globals:
#    None
# Arguments:
#    None
# Returns:
#    None
# Remark:
#    execute only once on start-up.
################################################
adjust_radio_smp_affinity() {
	local irqs="`grep -E 'ath10k' /proc/interrupts | cut -d ':' -f 1 | tr -d ' '`"
	local nr=`cat /proc/cpuinfo | grep processor | wc -l`
	local idx=2

	for irq in $irqs; do
		cpu=`printf "%x" $((1<<((idx)%nr)))`
		echo $cpu > /proc/irq/$irq/smp_affinity
		idx=$((idx+1))
	done
}

################################################
# Adjust queue of eth
# Globals:
#    None
# Arguments:
#    None
# Returns:
#    None
# Remark:
#    Each network reboot needs to be executed.
################################################
adjust_eth_queue() {
	local nr=`cat /proc/cpuinfo | grep processor | wc -l`
	local cpu=`printf "%x" $(((1<<nr)-1))`

	for epath in /sys/class/net/eth[0-9]*; do
		test -e $epath || break
		echo $epath | grep -q "\." && continue
		eth=`basename $epath`
		for exps in /sys/class/net/$eth/queues/rx-[0-9]*/rps_cpus; do
			test -e $exps || break
			echo $cpu > $exps
			echo 256 > `dirname $exps`/rps_flow_cnt
		done
		which ethtool >/dev/null 2>&1 && ethtool -K $eth gro off
	done

	echo 1024 > /proc/sys/net/core/rps_sock_flow_entries
}
