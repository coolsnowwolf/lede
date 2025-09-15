#!/bin/sh

[ "$ACTION" != "add" ] && exit

function log()
{
	local status="$1"
	local feature="$2"
	local interface="$3"

	if [ $status -eq 0 ]; then
		logger "[ethtool] $feature: disabled on $interface"
	fi

	if [ $status -eq 1 ]; then
		logger -s "[ethtool] $feature: failed to disable on $interface"
	fi

	if [ $status -gt 1 ]; then
		logger "[ethtool] $feature: no changes performed on $interface"
	fi
}

function interface_is_virtual()
{
    local interface="$1"
	[ -d /sys/devices/virtual/net/"$interface"/ ] || return 1
	return 0
}

function get_base_interface()
{
    local interface="$1"
	echo "$interface" | grep -Eo '^[a-z]*[0-9]*' 2>/dev/null || return 1
	return 0
}

function disable_offloads()
{
	local interface="$1"
	local features
	local cmd

	# Check if we can change features
	if ethtool -k $interface 1>/dev/null 2>/dev/null; then
		# Filter whitespaces
		# Get only enabled/not fixed features
		# Filter features that are only changeable by global keyword
		# Filter empty lines
		# Cut to First column
		features=$(ethtool -k "$interface" | awk '{$1=$1;print}' \
										   | grep -E '^.+: on$' \
										   | grep -v -E '^tx-checksum-.+$' \
										   | grep -v -E '^tx-scatter-gather.+$' \
										   | grep -v -E '^tx-tcp.+segmentation.+$' \
										   | grep -v -E '^tx-udp-fragmentation$' \
										   | grep -v -E '^tx-generic-segmentation$' \
										   | grep -v -E '^rx-gro$' \
										   | grep -v -E '^rx-gro$' \
										   | grep -v -E '^$' \
										   | cut -d: -f1)

		# Replace feature name by global keyword
		features=$(echo "$features" | sed -e s/rx-checksumming/rx/ \
										  -e s/tx-checksumming/tx/ \
										  -e s/scatter-gather/sg/ \
										  -e s/tcp-segmentation-offload/tso/ \
										  -e s/udp-fragmentation-offload/ufo/ \
										  -e s/generic-segmentation-offload/gso/ \
										  -e s/generic-receive-offload/gro/ \
										  -e s/large-receive-offload/lro/ \
										  -e s/rx-vlan-offload/rxvlan/ \
										  -e s/tx-vlan-offload/txvlan/ \
										  -e s/ntuple-filters/ntuple/ \
										  -e s/receive-hashing/rxhash/)

		# Check if we can disable anything
		if [ -z "$features" ]; then
			logger "[ethtool] offloads: no changes performed on $interface"
			return 0
		fi

		# Construct ethtool command line
		cmd="-K $interface"

		for feature in $features; do
			cmd="$cmd $feature off"
		done

		# Try to disable offloads
		ethtool $cmd 1>/dev/null 2>/dev/null
		log $? "Offloads" "$interface"

	else
		log $? "Offloads" "$interface"
	fi
}

function disable_flow_control()
{
	local interface="$1"
	local features
	local cmd

	# Check if we can change settings
	if ethtool -a $interface 1>/dev/null 2>/dev/null; then
		# Construct ethtool command line
		cmd="-A $interface autoneg off tx off rx off"

		# Try to disable flow control
		ethtool $cmd 1>/dev/null 2>/dev/null
		log $? "Flow Control" "$interface"

	else
		log $? "Flow Control" "$interface"
	fi
}

function disable_interrupt_moderation()
{
	local interface="$1"
	local features
	local cmd

	# Check if we can change settings
	if ethtool -c $interface 1>/dev/null 2>/dev/null; then
		# Construct ethtool command line
		cmd="-C $interface adaptive-tx off adaptive-rx off"

		# Try to disable adaptive interrupt moderation
		ethtool $cmd 1>/dev/null 2>/dev/null
		log $? "Adaptive Interrupt Moderation" "$interface"

		features=$(ethtool -c $interface | awk '{$1=$1;print}' \
										 | grep -v -E '^.+: 0$|Adaptive|Coalesce' \
										 | grep -v -E '^$' \
										 | cut -d: -f1)

		# Check if we can disable anything
		if [ -z "$features" ]; then
			logger "[ethtool] Interrupt Moderation: no changes performed on $interface"
			return 0
		fi

		# Construct ethtool command line
		cmd="-C $interface"

		for feature in $features; do
			cmd="$cmd $feature 0"
		done

		# Try to disable interrupt Moderation
		ethtool $cmd 1>/dev/null 2>/dev/null
		log $? "Interrupt Moderation" "$interface"

	else
		log $? "Interrupt Moderation" "$interface"
	fi
}

function disable_interface_offloads() {
    #local interface=$(get_base_interface "$1")
    #{ [ -z "$interface" ] || interface_is_virtual "$interface"; } && exit 0

	local interface="$1"

    local disable_offloads="$(uci get ecm.@general[0].disable_offloads)"
	if [ "$disable_offloads" -eq 1 ]; then
		disable_offloads "$interface"
	fi

    local disable_flow_control="$(uci get ecm.@general[0].disable_flow_control)"
	if [ "$disable_flow_control" -eq 1 ]; then
		disable_flow_control "$interface"
	fi

    local disable_interrupt_moderation="$(uci get ecm.@general[0].disable_interrupt_moderation)"
	if [ "$disable_interrupt_moderation" -eq 1 ]; then
		disable_interrupt_moderation "$interface"
	fi
}

if [ "$ACTION" = add ]; then
	disable_interface_offloads "$INTERFACE"
fi

exit 0
