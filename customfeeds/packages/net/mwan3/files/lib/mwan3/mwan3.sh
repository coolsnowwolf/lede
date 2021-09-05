#!/bin/sh

IP4="ip -4"
IP6="ip -6"
IPS="ipset"
IPT4="iptables -t mangle -w"
IPT6="ip6tables -t mangle -w"
LOG="logger -t mwan3[$$] -p"
CONNTRACK_FILE="/proc/net/nf_conntrack"
IPv6_REGEX="([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|"
IPv6_REGEX="${IPv6_REGEX}([0-9a-fA-F]{1,4}:){1,7}:|"
IPv6_REGEX="${IPv6_REGEX}([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|"
IPv6_REGEX="${IPv6_REGEX}([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|"
IPv6_REGEX="${IPv6_REGEX}([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|"
IPv6_REGEX="${IPv6_REGEX}([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|"
IPv6_REGEX="${IPv6_REGEX}([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|"
IPv6_REGEX="${IPv6_REGEX}[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|"
IPv6_REGEX="${IPv6_REGEX}:((:[0-9a-fA-F]{1,4}){1,7}|:)|"
IPv6_REGEX="${IPv6_REGEX}fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|"
IPv6_REGEX="${IPv6_REGEX}::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|"
IPv6_REGEX="${IPv6_REGEX}([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])"

MWAN3_STATUS_DIR="/var/run/mwan3"
MWAN3TRACK_STATUS_DIR="/var/run/mwan3track"
MWAN3_INTERFACE_MAX=""
DEFAULT_LOWEST_METRIC=256
MMX_MASK=""
MMX_DEFAULT=""
MMX_BLACKHOLE=""
MM_BLACKHOLE=""

MMX_UNREACHABLE=""
MM_UNREACHABLE=""

# return true(=0) if has any mwan3 interface enabled
# otherwise return false
mwan3_rtmon_ipv4()
{
	local tid=1
	local idx=0
	local ret=1
	local tbl=""
	mkdir -p /tmp/mwan3rtmon
	($IP4 route list table main  | grep -v "^default\|linkdown" | sort -n; echo empty fixup) >/tmp/mwan3rtmon/ipv4.main
	while uci get mwan3.@interface[$idx] >/dev/null 2>&1 ; do
		idx=$((idx+1))
		tid=$idx
		[ "$(uci get mwan3.@interface[$((idx-1))].family)" = "ipv4" ] && {
			tbl=$($IP4 route list table $tid)
			if echo "$tbl" | grep -q ^default; then
				(echo "$tbl"  | grep -v "^default\|linkdown" | sort -n; echo empty fixup) >/tmp/mwan3rtmon/ipv4.$tid
				cat /tmp/mwan3rtmon/ipv4.$tid | grep -v -x -F -f /tmp/mwan3rtmon/ipv4.main | while read line; do
					$IP4 route del table $tid $line
				done
				cat /tmp/mwan3rtmon/ipv4.main | grep -v -x -F -f /tmp/mwan3rtmon/ipv4.$tid | while read line; do
					$IP4 route add table $tid $line
				done
			fi
		}
		if [ "$(uci get mwan3.@interface[$((idx-1))].enabled)" = "1" ]; then
			ret=0
		fi
	done
	rm -f /tmp/mwan3rtmon/ipv4.*
	return $ret
}

# return true(=0) if has any mwan3 interface enabled
# otherwise return false
mwan3_rtmon_ipv6()
{
	local tid=1
	local idx=0
	local ret=1
	local tbl=""
	mkdir -p /tmp/mwan3rtmon
	($IP6 route list table main  | grep -v "^default\|^::/0\|^fe80::/64\|^unreachable" | sort -n; echo empty fixup) >/tmp/mwan3rtmon/ipv6.main
	while uci get mwan3.@interface[$idx] >/dev/null 2>&1 ; do
		idx=$((idx+1))
		tid=$idx
		[ "$(uci get mwan3.@interface[$((idx-1))].family)" = "ipv6" ] && {
			tbl=$($IP6 route list table $tid)
			if echo "$tbl" | grep -q "^default\|^::/0"; then
				(echo "$tbl"  | grep -v "^default\|^::/0\|^unreachable" | sort -n; echo empty fixup) >/tmp/mwan3rtmon/ipv6.$tid
				cat /tmp/mwan3rtmon/ipv6.$tid | grep -v -x -F -f /tmp/mwan3rtmon/ipv6.main | while read line; do
					$IP6 route del table $tid $line
				done
				cat /tmp/mwan3rtmon/ipv6.main | grep -v -x -F -f /tmp/mwan3rtmon/ipv6.$tid | while read line; do
					$IP6 route add table $tid $line
				done
			fi
		}
		if [ "$(uci get mwan3.@interface[$((idx-1))].enabled)" = "1" ]; then
			ret=0
		fi
	done
	rm -f /tmp/mwan3rtmon/ipv6.*
	return $ret
}

# counts how many bits are set to 1
# n&(n-1) clears the lowest bit set to 1
mwan3_count_one_bits()
{
	local count n
	count=0
	n=$(($1))
	while [ "$n" -gt "0" ]; do
		n=$((n&(n-1)))
		count=$((count+1))
	done
	echo $count
}

# maps the 1st parameter so it only uses the bits allowed by the bitmask (2nd parameter)
# which means spreading the bits of the 1st parameter to only use the bits that are set to 1 in the 2nd parameter
# 0 0 0 0 0 1 0 1 (0x05) 1st parameter
# 1 0 1 0 1 0 1 0 (0xAA) 2nd parameter
#     1   0   1          result
mwan3_id2mask()
{
	local bit_msk bit_val result
	bit_val=0
	result=0
	for bit_msk in $(seq 0 31); do
		if [ $((($2>>bit_msk)&1)) = "1" ]; then
			if [ $((($1>>bit_val)&1)) = "1" ]; then
				result=$((result|(1<<bit_msk)))
			fi
			bit_val=$((bit_val+1))
		fi
	done
	printf "0x%x" $result
}

mwan3_init()
{
	local bitcnt
	local mmdefault

	[ -d $MWAN3_STATUS_DIR ] || mkdir -p $MWAN3_STATUS_DIR/iface_state

	# mwan3's MARKing mask (at least 3 bits should be set)
	if [ -e "${MWAN3_STATUS_DIR}/mmx_mask" ]; then
		MMX_MASK=$(cat "${MWAN3_STATUS_DIR}/mmx_mask")
		MWAN3_INTERFACE_MAX=$(uci_get_state mwan3 globals iface_max)
	else
		config_load mwan3
		config_get MMX_MASK globals mmx_mask '0x3F00'
		echo "$MMX_MASK" > "${MWAN3_STATUS_DIR}/mmx_mask"
		$LOG notice "Using firewall mask ${MMX_MASK}"

		bitcnt=$(mwan3_count_one_bits MMX_MASK)
		mmdefault=$(((1<<bitcnt)-1))
		MWAN3_INTERFACE_MAX=$(($mmdefault-3))
		uci_toggle_state mwan3 globals iface_max "$MWAN3_INTERFACE_MAX"
		$LOG notice "Max interface count is ${MWAN3_INTERFACE_MAX}"
	fi

	# mark mask constants
	bitcnt=$(mwan3_count_one_bits MMX_MASK)
	mmdefault=$(((1<<bitcnt)-1))
	MM_BLACKHOLE=$(($mmdefault-2))
	MM_UNREACHABLE=$(($mmdefault-1))

	# MMX_DEFAULT should equal MMX_MASK
	MMX_DEFAULT=$(mwan3_id2mask mmdefault MMX_MASK)
	MMX_BLACKHOLE=$(mwan3_id2mask MM_BLACKHOLE MMX_MASK)
	MMX_UNREACHABLE=$(mwan3_id2mask MM_UNREACHABLE MMX_MASK)
}

mwan3_lock() {
	lock /var/run/mwan3.lock
#	$LOG debug "$1 $2 (lock)"
}

mwan3_unlock() {
#	$LOG debug "$1 $2 (unlock)"
	lock -u /var/run/mwan3.lock
}

mwan3_lock_clean() {
	rm -rf /var/run/mwan3.lock
}

mwan3_get_iface_id()
{
	local _tmp _iface _iface_count

	_iface="$2"

	mwan3_get_id()
	{
		let _iface_count++
		[ "$1" == "$_iface" ] && _tmp=$_iface_count
	}
	config_foreach mwan3_get_id interface
	export "$1=$_tmp"
}

mwan3_set_custom_ipset_v4()
{
	local custom_network_v4

	for custom_network_v4 in $($IP4 route list table "$1" | awk '{print $1}' | egrep '[0-9]{1,3}(\.[0-9]{1,3}){3}'); do
		$LOG notice "Adding network $custom_network_v4 from table $1 to mwan3_custom_v4 ipset"
		$IPS -! add mwan3_custom_v4_temp $custom_network_v4
	done
}

mwan3_set_custom_ipset_v6()
{
	local custom_network_v6

	for custom_network_v6 in $($IP6 route list table "$1" | awk '{print $1}' | egrep "$IPv6_REGEX"); do
		$LOG notice "Adding network $custom_network_v6 from table $1 to mwan3_custom_v6 ipset"
		$IPS -! add mwan3_custom_v6_temp $custom_network_v6
	done
}

mwan3_set_custom_ipset()
{
	$IPS -! create mwan3_custom_v4 hash:net
	$IPS create mwan3_custom_v4_temp hash:net
	config_list_foreach "globals" "rt_table_lookup" mwan3_set_custom_ipset_v4
	$IPS swap mwan3_custom_v4_temp mwan3_custom_v4
	$IPS destroy mwan3_custom_v4_temp


	$IPS -! create mwan3_custom_v6 hash:net family inet6
	$IPS create mwan3_custom_v6_temp hash:net family inet6
	config_list_foreach "globals" "rt_table_lookup" mwan3_set_custom_ipset_v6
	$IPS swap mwan3_custom_v6_temp mwan3_custom_v6
	$IPS destroy mwan3_custom_v6_temp

	$IPS -! create mwan3_connected list:set
	$IPS -! add mwan3_connected mwan3_custom_v4
	$IPS -! add mwan3_connected mwan3_custom_v6
}

mwan3_set_connected_iptables()
{
	local connected_network_v4 connected_network_v6 source_network_v6

	$IPS -! create mwan3_connected_v4 hash:net
	$IPS create mwan3_connected_v4_temp hash:net

	for connected_network_v4 in $($IP4 route | awk '{print $1}' | egrep '[0-9]{1,3}(\.[0-9]{1,3}){3}'); do
		$IPS -! add mwan3_connected_v4_temp $connected_network_v4
	done

	for connected_network_v4 in $($IP4 route list table 0 | awk '{print $2}' | egrep '[0-9]{1,3}(\.[0-9]{1,3}){3}'); do
		$IPS -! add mwan3_connected_v4_temp $connected_network_v4
	done

	$IPS add mwan3_connected_v4_temp 224.0.0.0/3

	$IPS swap mwan3_connected_v4_temp mwan3_connected_v4
	$IPS destroy mwan3_connected_v4_temp

	$IPS -! create mwan3_connected_v6 hash:net family inet6
	$IPS create mwan3_connected_v6_temp hash:net family inet6

	for connected_network_v6 in $($IP6 route | awk '{print $1}' | egrep "$IPv6_REGEX"); do
		$IPS -! add mwan3_connected_v6_temp $connected_network_v6
	done

	$IPS swap mwan3_connected_v6_temp mwan3_connected_v6
	$IPS destroy mwan3_connected_v6_temp

	$IPS -! create mwan3_connected list:set
	$IPS -! add mwan3_connected mwan3_connected_v4
	$IPS -! add mwan3_connected mwan3_connected_v6

	$IPS -! create mwan3_source_v6 hash:net family inet6
	$IPS create mwan3_source_v6_temp hash:net family inet6
	for source_network_v6 in $($IP6 addr ls  | sed -ne 's/ *inet6 \([^ \/]*\).* scope global.*/\1/p'); do
		$IPS -! add mwan3_source_v6_temp $source_network_v6
	done
	$IPS swap mwan3_source_v6_temp mwan3_source_v6
	$IPS destroy mwan3_source_v6_temp

	$IPS -! create mwan3_dynamic_v4 hash:net
	$IPS -! add mwan3_connected mwan3_dynamic_v4

	$IPS -! create mwan3_dynamic_v6 hash:net family inet6
	$IPS -! add mwan3_connected mwan3_dynamic_v6
}

mwan3_set_general_rules()
{
	local IP

	for IP in "$IP4" "$IP6"; do

		RULE_NO=$(($MM_BLACKHOLE+2000))
		if [ -z "$($IP rule list | awk -v var="$RULE_NO:" '$1 == var')" ]; then
			$IP rule add pref $RULE_NO fwmark $MMX_BLACKHOLE/$MMX_MASK blackhole
		fi

		RULE_NO=$(($MM_UNREACHABLE+2000))
		if [ -z "$($IP rule list | awk -v var="$RULE_NO:" '$1 == var')" ]; then
			$IP rule add pref $RULE_NO fwmark $MMX_UNREACHABLE/$MMX_MASK unreachable
		fi
	done
}

mwan3_set_general_iptables()
{
	local IPT

	for IPT in "$IPT4" "$IPT6"; do

		if ! $IPT -S mwan3_ifaces_in &> /dev/null; then
			$IPT -N mwan3_ifaces_in
		fi

		if ! $IPT -S mwan3_connected &> /dev/null; then
			$IPT -N mwan3_connected
			$IPS -! create mwan3_connected list:set
			$IPT -A mwan3_connected \
				-m set --match-set mwan3_connected dst \
				-j MARK --set-xmark $MMX_DEFAULT/$MMX_MASK
		fi

		if ! $IPT -S mwan3_rules &> /dev/null; then
			$IPT -N mwan3_rules
		fi

		if ! $IPT -S mwan3_hook &> /dev/null; then
			$IPT -N mwan3_hook
			# do not mangle ipv6 ra service
			if [ "$IPT" = "$IPT6" ]; then
				$IPT6 -A mwan3_hook \
					-p ipv6-icmp \
					-m icmp6 --icmpv6-type 133 \
					-j RETURN
				$IPT6 -A mwan3_hook \
					-p ipv6-icmp \
					-m icmp6 --icmpv6-type 134 \
					-j RETURN
				$IPT6 -A mwan3_hook \
					-p ipv6-icmp \
					-m icmp6 --icmpv6-type 135 \
					-j RETURN
				$IPT6 -A mwan3_hook \
					-p ipv6-icmp \
					-m icmp6 --icmpv6-type 136 \
					-j RETURN
				$IPT6 -A mwan3_hook \
					-p ipv6-icmp \
					-m icmp6 --icmpv6-type 137 \
					-j RETURN
				# do not mangle outgoing echo request
				$IPT6 -A mwan3_hook \
					-m set --match-set mwan3_source_v6 src \
					-p ipv6-icmp \
					-m icmp6 --icmpv6-type 128 \
					-j RETURN

			fi
			$IPT -A mwan3_hook \
				-j CONNMARK --restore-mark --nfmask $MMX_MASK --ctmask $MMX_MASK
			$IPT -A mwan3_hook \
				-m mark --mark 0x0/$MMX_MASK \
				-j mwan3_ifaces_in
			$IPT -A mwan3_hook \
				-m mark --mark 0x0/$MMX_MASK \
				-j mwan3_connected
			$IPT -A mwan3_hook \
				-m mark --mark 0x0/$MMX_MASK \
				-j mwan3_rules
			$IPT -A mwan3_hook \
				-j CONNMARK --save-mark --nfmask $MMX_MASK --ctmask $MMX_MASK
			$IPT -A mwan3_hook \
				-m mark ! --mark $MMX_DEFAULT/$MMX_MASK \
				-j mwan3_connected
		fi

		if ! $IPT -S PREROUTING | grep mwan3_hook &> /dev/null; then
			$IPT -A PREROUTING -j mwan3_hook
		fi

		if ! $IPT -S OUTPUT | grep mwan3_hook &> /dev/null; then
			$IPT -A OUTPUT -j mwan3_hook
		fi
	done
}

mwan3_create_iface_iptables()
{
	local id family

	config_get family $1 family ipv4
	mwan3_get_iface_id id $1

	[ -n "$id" ] || return 0

	if [ "$family" == "ipv4" ]; then
		$IPS -! create mwan3_connected list:set

		if ! $IPT4 -S mwan3_ifaces_in &> /dev/null; then
			$IPT4 -N mwan3_ifaces_in
		fi

		if ! $IPT4 -S mwan3_iface_in_$1 &> /dev/null; then
			$IPT4 -N mwan3_iface_in_$1
		fi

		$IPT4 -F mwan3_iface_in_$1
		$IPT4 -A mwan3_iface_in_$1 \
			-i $2 \
			-m set --match-set mwan3_connected src \
			-m mark --mark 0x0/$MMX_MASK \
			-m comment --comment "default" \
			-j MARK --set-xmark $MMX_DEFAULT/$MMX_MASK
		$IPT4 -A mwan3_iface_in_$1 \
			-i $2 \
			-m mark --mark 0x0/$MMX_MASK \
			-m comment --comment "$1" \
			-j MARK --set-xmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK

		$IPT4 -D mwan3_ifaces_in \
			-m mark --mark 0x0/$MMX_MASK \
			-j mwan3_iface_in_$1 &> /dev/null
		$IPT4 -A mwan3_ifaces_in \
			-m mark --mark 0x0/$MMX_MASK \
			-j mwan3_iface_in_$1
	fi

	if [ "$family" == "ipv6" ]; then
		$IPS -! create mwan3_connected_v6 hash:net family inet6

		if ! $IPT6 -S mwan3_ifaces_in &> /dev/null; then
			$IPT6 -N mwan3_ifaces_in
		fi

		if ! $IPT6 -S mwan3_iface_in_$1 &> /dev/null; then
			$IPT6 -N mwan3_iface_in_$1
		fi

		$IPT6 -F mwan3_iface_in_$1
		$IPT6 -A mwan3_iface_in_$1 -i $2 \
			-m set --match-set mwan3_connected_v6 src \
			-m mark --mark 0x0/$MMX_MASK \
			-m comment --comment "default" \
			-j MARK --set-xmark $MMX_DEFAULT/$MMX_MASK
		$IPT6 -A mwan3_iface_in_$1 -i $2 -m mark --mark 0x0/$MMX_MASK \
			-m comment --comment "$1" \
			-j MARK --set-xmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK

		$IPT6 -D mwan3_ifaces_in \
			-m mark --mark 0x0/$MMX_MASK \
			-j mwan3_iface_in_$1 &> /dev/null
		$IPT6 -A mwan3_ifaces_in \
			-m mark --mark 0x0/$MMX_MASK \
			-j mwan3_iface_in_$1
	fi
}

mwan3_delete_iface_iptables()
{
	config_get family $1 family ipv4

	if [ "$family" == "ipv4" ]; then

		$IPT4 -D mwan3_ifaces_in \
			-m mark --mark 0x0/$MMX_MASK \
			-j mwan3_iface_in_$1 &> /dev/null
		$IPT4 -F mwan3_iface_in_$1 &> /dev/null
		$IPT4 -X mwan3_iface_in_$1 &> /dev/null
	fi

	if [ "$family" == "ipv6" ]; then

		$IPT6 -D mwan3_ifaces_in \
			-m mark --mark 0x0/$MMX_MASK \
			-j mwan3_iface_in_$1 &> /dev/null
		$IPT6 -F mwan3_iface_in_$1 &> /dev/null
		$IPT6 -X mwan3_iface_in_$1 &> /dev/null
	fi
}

mwan3_create_iface_route()
{
	local id route_args metric

	config_get family $1 family ipv4
	mwan3_get_iface_id id $1

	[ -n "$id" ] || return 0

	if [ "$family" == "ipv4" ]; then
		if ubus call network.interface.${1}_4 status &>/dev/null; then
			network_get_gateway route_args ${1}_4
		else
			network_get_gateway route_args $1
		fi

		if [ -n "$route_args" -a "$route_args" != "0.0.0.0" ]; then
			route_args="via $route_args"
		else
			route_args=""
		fi

		network_get_metric metric $1
		if [ -n "$metric" -a "$metric" != "0" ]; then
			route_args="$route_args metric $metric"
		fi

		$IP4 route flush table $id
		$IP4 route add table $id default $route_args dev $2
		mwan3_rtmon_ipv4
	fi

	if [ "$family" == "ipv6" ]; then
		if ubus call network.interface.${1}_6 status &>/dev/null; then
			network_get_gateway6 route_args ${1}_6
		else
			network_get_gateway6 route_args $1
		fi

		if [ -n "$route_args" -a "$route_args" != "::" ]; then
			route_args="via $route_args"
		else
			route_args=""
		fi

		network_get_metric metric $1
		if [ -n "$metric" -a "$metric" != "0" ]; then
			route_args="$route_args metric $metric"
		fi

		$IP6 route flush table $id
		$IP6 route add table $id default $route_args dev $2
		mwan3_rtmon_ipv6
	fi
}

mwan3_delete_iface_route()
{
	local id

	config_get family $1 family ipv4
	mwan3_get_iface_id id $1

	[ -n "$id" ] || return 0

	if [ "$family" == "ipv4" ]; then
		$IP4 route flush table $id
	fi

	if [ "$family" == "ipv6" ]; then
		$IP6 route flush table $id
	fi
}

mwan3_create_iface_rules()
{
	local id family

	config_get family $1 family ipv4
	mwan3_get_iface_id id $1

	[ -n "$id" ] || return 0

	if [ "$family" == "ipv4" ]; then

		while [ -n "$($IP4 rule list | awk '$1 == "'$(($id+1000)):'"')" ]; do
			$IP4 rule del pref $(($id+1000))
		done

		while [ -n "$($IP4 rule list | awk '$1 == "'$(($id+2000)):'"')" ]; do
			$IP4 rule del pref $(($id+2000))
		done

		$IP4 rule add pref $(($id+1000)) iif $2 lookup $id
		$IP4 rule add pref $(($id+2000)) fwmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK lookup $id
	fi

	if [ "$family" == "ipv6" ]; then

		while [ -n "$($IP6 rule list | awk '$1 == "'$(($id+1000)):'"')" ]; do
			$IP6 rule del pref $(($id+1000))
		done

		while [ -n "$($IP6 rule list | awk '$1 == "'$(($id+2000)):'"')" ]; do
			$IP6 rule del pref $(($id+2000))
		done

		$IP6 rule add pref $(($id+1000)) iif $2 lookup $id
		$IP6 rule add pref $(($id+2000)) fwmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK lookup $id
	fi
}

mwan3_delete_iface_rules()
{
	local id family

	config_get family $1 family ipv4
	mwan3_get_iface_id id $1

	[ -n "$id" ] || return 0

	if [ "$family" == "ipv4" ]; then

		while [ -n "$($IP4 rule list | awk '$1 == "'$(($id+1000)):'"')" ]; do
			$IP4 rule del pref $(($id+1000))
		done

		while [ -n "$($IP4 rule list | awk '$1 == "'$(($id+2000)):'"')" ]; do
			$IP4 rule del pref $(($id+2000))
		done
	fi

	if [ "$family" == "ipv6" ]; then

		while [ -n "$($IP6 rule list | awk '$1 == "'$(($id+1000)):'"')" ]; do
			$IP6 rule del pref $(($id+1000))
		done

		while [ -n "$($IP6 rule list | awk '$1 == "'$(($id+2000)):'"')" ]; do
			$IP6 rule del pref $(($id+2000))
		done
	fi
}

mwan3_delete_iface_ipset_entries()
{
	local id setname entry

	mwan3_get_iface_id id $1

	[ -n "$id" ] || return 0

	for setname in $(ipset -n list | grep ^mwan3_sticky_); do
		for entry in $(ipset list $setname | grep "$(echo $(mwan3_id2mask id MMX_MASK) | awk '{ printf "0x%08x", $1; }')" | cut -d ' ' -f 1); do
			$IPS del $setname $entry
		done
	done
}

mwan3_rtmon()
{
	pid="$(pgrep -f mwan3rtmon)"
	if [ "${pid}" != "" ]; then
		kill -USR1 "${pid}"
	else
		[ -x /usr/sbin/mwan3rtmon ] && /usr/sbin/mwan3rtmon &
	fi
}

mwan3_track()
{
	local track_ip track_ips pid

	mwan3_list_track_ips()
	{
		track_ips="$track_ips $1"
	}
	config_list_foreach $1 track_ip mwan3_list_track_ips

	for pid in $(pgrep -f "mwan3track $1 $2"); do
		kill -TERM "$pid" > /dev/null 2>&1
		sleep 1
		kill -KILL "$pid" > /dev/null 2>&1
	done
	if [ -n "$track_ips" ]; then
		[ -x /usr/sbin/mwan3track ] && /usr/sbin/mwan3track "$1" "$2" "$3" "$4" $track_ips &
	fi
}

mwan3_track_signal()
{
	local pid

	pid="$(pgrep -f "mwan3track $1 $2")"
	[ "${pid}" != "" ] && {
		kill -USR1 "${pid}"
	}
}

mwan3_set_policy()
{
	local iface_count id iface family metric probability weight device

	config_get iface $1 interface
	config_get metric $1 metric 1
	config_get weight $1 weight 1

	[ -n "$iface" ] || return 0
	network_get_device device $iface
	[ "$metric" -gt $DEFAULT_LOWEST_METRIC ] && $LOG warn "Member interface $iface has >$DEFAULT_LOWEST_METRIC metric. Not appending to policy" && return 0

	mwan3_get_iface_id id $iface

	[ -n "$id" ] || return 0

	config_get family $iface family ipv4

	if [ "$family" == "ipv4" ]; then

		if [ "$(mwan3_get_iface_hotplug_state $iface)" = "online" ]; then
			if [ "$metric" -lt "$lowest_metric_v4" ]; then

				total_weight_v4=$weight
				$IPT4 -F mwan3_policy_$policy
				$IPT4 -A mwan3_policy_$policy \
					-m mark --mark 0x0/$MMX_MASK \
					-m comment --comment "$iface $weight $weight" \
					-j MARK --set-xmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK

				lowest_metric_v4=$metric

			elif [ "$metric" -eq "$lowest_metric_v4" ]; then

				total_weight_v4=$(($total_weight_v4+$weight))
				probability=$(($weight*1000/$total_weight_v4))

				if [ "$probability" -lt 10 ]; then
					probability="0.00$probability"
				elif [ $probability -lt 100 ]; then
					probability="0.0$probability"
				elif [ $probability -lt 1000 ]; then
					probability="0.$probability"
				else
					probability="1"
				fi

				probability="-m statistic --mode random --probability $probability"

				$IPT4 -I mwan3_policy_$policy \
					-m mark --mark 0x0/$MMX_MASK $probability \
					-m comment --comment "$iface $weight $total_weight_v4" \
					-j MARK --set-xmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK
			fi
		else
			[ -n "$device" ] && {
				$IPT4 -S mwan3_policy_$policy | grep -q '.*--comment ".* [0-9]* [0-9]*"' || \
					$IPT4 -I mwan3_policy_$policy \
					-o $device \
					-m mark --mark 0x0/$MMX_MASK \
					-m comment --comment "out $iface $device" \
					-j MARK --set-xmark $MMX_DEFAULT/$MMX_MASK
			}
		fi
	fi

	if [ "$family" == "ipv6" ]; then

		if [ "$(mwan3_get_iface_hotplug_state $iface)" = "online" ]; then
			if [ "$metric" -lt "$lowest_metric_v6" ]; then

				total_weight_v6=$weight
				$IPT6 -F mwan3_policy_$policy
				$IPT6 -A mwan3_policy_$policy \
					-m mark --mark 0x0/$MMX_MASK \
					-m comment --comment "$iface $weight $weight" \
					-j MARK --set-xmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK

				lowest_metric_v6=$metric

			elif [ "$metric" -eq "$lowest_metric_v6" ]; then

				total_weight_v6=$(($total_weight_v6+$weight))
				probability=$(($weight*1000/$total_weight_v6))

				if [ "$probability" -lt 10 ]; then
					probability="0.00$probability"
				elif [ $probability -lt 100 ]; then
					probability="0.0$probability"
				elif [ $probability -lt 1000 ]; then
					probability="0.$probability"
				else
					probability="1"
				fi

				probability="-m statistic --mode random --probability $probability"

				$IPT6 -I mwan3_policy_$policy \
					-m mark --mark 0x0/$MMX_MASK \
					$probability \
					-m comment --comment "$iface $weight $total_weight_v6" \
					-j MARK --set-xmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK
			fi
		else
			[ -n "$device" ] && {
				$IPT6 -S mwan3_policy_$policy | grep -q '.*--comment ".* [0-9]* [0-9]*"' || \
					$IPT6 -I mwan3_policy_$policy \
					-o $device \
					-m mark --mark 0x0/$MMX_MASK \
					-m comment --comment "out $iface $device" \
					-j MARK --set-xmark $MMX_DEFAULT/$MMX_MASK
			}
		fi
	fi
}

mwan3_create_policies_iptables()
{
	local last_resort lowest_metric_v4 lowest_metric_v6 total_weight_v4 total_weight_v6 policy IPT

	policy="$1"

	config_get last_resort $1 last_resort unreachable

	if [ "$1" != $(echo "$1" | cut -c1-15) ]; then
		$LOG warn "Policy $1 exceeds max of 15 chars. Not setting policy" && return 0
	fi

	for IPT in "$IPT4" "$IPT6"; do

		if ! $IPT -S mwan3_policy_$1 &> /dev/null; then
			$IPT -N mwan3_policy_$1
		fi

		$IPT -F mwan3_policy_$1

		case "$last_resort" in
			blackhole)
				$IPT -A mwan3_policy_$1 \
					-m mark --mark 0x0/$MMX_MASK \
					-m comment --comment "blackhole" \
					-j MARK --set-xmark $MMX_BLACKHOLE/$MMX_MASK
			;;
			default)
				$IPT -A mwan3_policy_$1 \
					-m mark --mark 0x0/$MMX_MASK \
					-m comment --comment "default" \
					-j MARK --set-xmark $MMX_DEFAULT/$MMX_MASK
			;;
			*)
				$IPT -A mwan3_policy_$1 \
					-m mark --mark 0x0/$MMX_MASK \
					-m comment --comment "unreachable" \
					-j MARK --set-xmark $MMX_UNREACHABLE/$MMX_MASK
			;;
		esac
	done

	lowest_metric_v4=$DEFAULT_LOWEST_METRIC
	total_weight_v4=0

	lowest_metric_v6=$DEFAULT_LOWEST_METRIC
	total_weight_v6=0

	config_list_foreach $1 use_member mwan3_set_policy
}

mwan3_set_policies_iptables()
{
	config_foreach mwan3_create_policies_iptables policy
}

mwan3_set_sticky_iptables()
{
	local id iface

	for iface in $($IPT4 -S $policy | cut -s -d'"' -f2 | awk '{print $1}'); do

		if [ "$iface" == "$1" ]; then

			mwan3_get_iface_id id $1

			[ -n "$id" ] || return 0

			for IPT in "$IPT4" "$IPT6"; do
				if [ -n "$($IPT -S mwan3_iface_in_$1 2> /dev/null)" ]; then
					$IPT -I mwan3_rule_$rule \
						-m mark --mark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK \
						-m set ! --match-set mwan3_sticky_$rule src,src \
						-j MARK --set-xmark 0x0/$MMX_MASK
					$IPT -I mwan3_rule_$rule \
						-m mark --mark 0/$MMX_MASK \
						-j MARK --set-xmark $(mwan3_id2mask id MMX_MASK)/$MMX_MASK
				fi
			done
		fi
	done
}

mwan3_set_user_iptables_rule()
{
	local ipset family proto policy src_ip src_port src_iface src_dev
	local sticky dest_ip dest_port use_policy timeout rule policy IPT
	local global_logging rule_logging loglevel

	rule="$1"

	config_get sticky $1 sticky 0
	config_get timeout $1 timeout 600
	config_get ipset $1 ipset
	config_get proto $1 proto all
	config_get src_ip $1 src_ip
	config_get src_iface $1 src_iface
	network_get_device src_dev $src_iface
	config_get src_port $1 src_port
	config_get dest_ip $1 dest_ip
	config_get dest_port $1 dest_port
	config_get use_policy $1 use_policy
	config_get family $1 family any

	[ -z "$dest_ip" ] && unset dest_ip
	[ -z "$src_ip" ] && unset src_ip
	[ -z "$ipset" ] && unset ipset
	[ -z "$src_port" ]  && unset src_port
	[ -z "$dest_port" ]  && unset dest_port
	[ "$proto"  != 'tcp' ]  && [ "$proto" != 'udp' ] && {
		[ -n "$src_port" ] && {
			$LOG warn "src_port set to '$src_port' but proto set to '$proto' not tcp or udp. src_port will be ignored"
		}
		[ -n "$dest_port" ] && {
			$LOG warn "dest_port set to '$dest_port' but proto set to '$proto' not tcp or udp. dest_port will be ignored"
		}
		unset src_port
		unset dest_port
	}

	config_get rule_logging $1 logging 0
	config_get global_logging globals logging 0
	config_get loglevel globals loglevel notice

	if [ "$1" != $(echo "$1" | cut -c1-15) ]; then
		$LOG warn "Rule $1 exceeds max of 15 chars. Not setting rule" && return 0
	fi

	if [ -n "$ipset" ]; then
		ipset="-m set --match-set $ipset dst"
	fi

	if [ -n "$use_policy" ]; then
		if [ "$use_policy" == "default" ]; then
			policy="MARK --set-xmark $MMX_DEFAULT/$MMX_MASK"
		elif [ "$use_policy" == "unreachable" ]; then
			policy="MARK --set-xmark $MMX_UNREACHABLE/$MMX_MASK"
		elif [ "$use_policy" == "blackhole" ]; then
			policy="MARK --set-xmark $MMX_BLACKHOLE/$MMX_MASK"
		else
			if [ "$sticky" -eq 1 ]; then

				policy="mwan3_policy_$use_policy"

				for IPT in "$IPT4" "$IPT6"; do
					if ! $IPT -S $policy &> /dev/null; then
						$IPT -N $policy
					fi

					if ! $IPT -S mwan3_rule_$1 &> /dev/null; then
						$IPT -N mwan3_rule_$1
					fi

					$IPT -F mwan3_rule_$1
				done

				$IPS -! create mwan3_sticky_v4_$rule \
					hash:ip,mark markmask $MMX_MASK \
					timeout $timeout
				$IPS -! create mwan3_sticky_v6_$rule \
					hash:ip,mark markmask $MMX_MASK \
					timeout $timeout family inet6
				$IPS -! create mwan3_sticky_$rule list:set
				$IPS -! add mwan3_sticky_$rule mwan3_sticky_v4_$rule
				$IPS -! add mwan3_sticky_$rule mwan3_sticky_v6_$rule

				config_foreach mwan3_set_sticky_iptables interface

				for IPT in "$IPT4" "$IPT6"; do
					$IPT -A mwan3_rule_$1 \
						-m mark --mark 0/$MMX_MASK \
						-j $policy
					$IPT -A mwan3_rule_$1 \
						-m mark ! --mark 0xfc00/0xfc00 \
						-j SET --del-set mwan3_sticky_$rule src,src
					$IPT -A mwan3_rule_$1 \
						-m mark ! --mark 0xfc00/0xfc00 \
						-j SET --add-set mwan3_sticky_$rule src,src
				done

				policy="mwan3_rule_$1"
			else
				policy="mwan3_policy_$use_policy"

				for IPT in "$IPT4" "$IPT6"; do
					if ! $IPT -S $policy &> /dev/null; then
						$IPT -N $policy
					fi
				done

			fi
		fi
		for IPT in "$IPT4" "$IPT6"; do
			[ "$family" == "ipv4" ] && [ "$IPT" == "$IPT6" ] && continue
			[ "$family" == "ipv6" ] && [ "$IPT" == "$IPT4" ] && continue
			[ "$global_logging" = "1" ] && [ "$rule_logging" = "1" ] && {
				$IPT -A mwan3_rules \
				     -p $proto \
				     ${src_ip:+-s} $src_ip \
				     ${src_dev:+-i} $src_dev \
				     ${dest_ip:+-d} $dest_ip\
				     $ipset \
				     ${src_port:+-m} ${src_port:+multiport} ${src_port:+--sports} $src_port \
				     ${dest_port:+-m} ${dest_port:+multiport} ${dest_port:+--dports} $dest_port \
				     -m mark --mark 0/$MMX_MASK \
				     -m comment --comment "$1" \
				     -j LOG --log-level "$loglevel" --log-prefix "MWAN3($1)" &> /dev/null
			}

			$IPT -A mwan3_rules \
			     -p $proto \
			     ${src_ip:+-s} $src_ip \
			     ${src_dev:+-i} $src_dev \
			     ${dest_ip:+-d} $dest_ip\
			     $ipset \
			     ${src_port:+-m} ${src_port:+multiport} ${src_port:+--sports} $src_port \
			     ${dest_port:+-m} ${dest_port:+multiport} ${dest_port:+--dports} $dest_port \
			     -m mark --mark 0/$MMX_MASK \
			     -j $policy &> /dev/null
		done
	fi
}

mwan3_set_user_rules()
{
	local IPT

	for IPT in "$IPT4" "$IPT6"; do

		if ! $IPT -S mwan3_rules &> /dev/null; then
			$IPT -N mwan3_rules
		fi

		$IPT -F mwan3_rules
	done

	config_foreach mwan3_set_user_iptables_rule rule
}

mwan3_set_iface_hotplug_state() {
	local iface=$1
	local state=$2

	echo -n $state > $MWAN3_STATUS_DIR/iface_state/$iface
}

mwan3_get_iface_hotplug_state() {
	local iface=$1

	cat $MWAN3_STATUS_DIR/iface_state/$iface 2>/dev/null || echo "offline"
}

mwan3_report_iface_status()
{
	local device result track_ips tracking IP IPT

	mwan3_get_iface_id id $1
	network_get_device device $1
	config_get enabled "$1" enabled 0
	config_get family "$1" family ipv4

	if [ "$family" == "ipv4" ]; then
		IP="$IP4"
		IPT="$IPT4"
	fi

	if [ "$family" == "ipv6" ]; then
		IP="$IP6"
		IPT="$IPT6"
	fi

	if [ -z "$id" -o -z "$device" ]; then
		result="offline"
	elif [ -n "$($IP rule | awk '$1 == "'$(($id+1000)):'"')" ] && \
		[ -n "$($IP rule | awk '$1 == "'$(($id+2000)):'"')" ] && \
		[ -n "$($IPT -S mwan3_iface_in_$1 2> /dev/null)" ] && \
		[ -n "$($IP route list table $id default dev $device 2> /dev/null)" ]; then
		result="$(mwan3_get_iface_hotplug_state $1)"
	elif [ -n "$($IP rule | awk '$1 == "'$(($id+1000)):'"')" ] || \
		[ -n "$($IP rule | awk '$1 == "'$(($id+2000)):'"')" ] || \
		[ -n "$($IPT -S mwan3_iface_in_$1 2> /dev/null)" ] || \
		[ -n "$($IP route list table $id default dev $device 2> /dev/null)" ]; then
		result="error"
	elif [ "$enabled" == "1" ]; then
		result="offline"
	else
		result="disabled"
	fi

	mwan3_list_track_ips()
	{
		track_ips="$1 $track_ips"
	}
	config_list_foreach $1 track_ip mwan3_list_track_ips

	if [ -n "$track_ips" ]; then
		if [ -n "$(pgrep -f "mwan3track $1 $device")" ]; then
			tracking="active"
		else
			tracking="down"
		fi
	else
		tracking="not enabled"
	fi

	echo " interface $1 is $result and tracking is $tracking"
}

mwan3_report_policies()
{
	local ipt="$1"
	local policy="$2"

	local percent total_weight weight iface

	total_weight=$($ipt -S $policy | grep -v '.*--comment "out .*" .*$' | cut -s -d'"' -f2 | head -1 | awk '{print $3}')

	if [ ! -z "${total_weight##*[!0-9]*}" ]; then
		for iface in $($ipt -S $policy | grep -v '.*--comment "out .*" .*$' | cut -s -d'"' -f2 | awk '{print $1}'); do
			weight=$($ipt -S $policy | grep -v '.*--comment "out .*" .*$' | cut -s -d'"' -f2 | awk '$1 == "'$iface'"' | awk '{print $2}')
			percent=$(($weight*100/$total_weight))
			echo " $iface ($percent%)"
		done
	else
		echo " $($ipt -S $policy | grep -v '.*--comment "out .*" .*$' | sed '/.*--comment \([^ ]*\) .*$/!d;s//\1/;q')"
	fi
}

mwan3_report_policies_v4()
{
	local policy

	for policy in $($IPT4 -S | awk '{print $2}' | grep mwan3_policy_ | sort -u); do
		echo "$policy:" | sed 's/mwan3_policy_//'
		mwan3_report_policies "$IPT4" "$policy"
	done
}

mwan3_report_policies_v6()
{
	local policy

	for policy in $($IPT6 -S | awk '{print $2}' | grep mwan3_policy_ | sort -u); do
		echo "$policy:" | sed 's/mwan3_policy_//'
		mwan3_report_policies "$IPT6" "$policy"
	done
}

mwan3_report_connected_v4()
{
	local address

	if [ -n "$($IPT4 -S mwan3_connected 2> /dev/null)" ]; then
		$IPS -o save list mwan3_connected_v4 | grep add | cut -d " " -f 3
	fi
}

mwan3_report_connected_v6()
{
	local address

	if [ -n "$($IPT6 -S mwan3_connected 2> /dev/null)" ]; then
		$IPS -o save list mwan3_connected_v6 | grep add | cut -d " " -f 3
	fi
}

mwan3_report_rules_v4()
{
	if [ -n "$($IPT4 -S mwan3_rules 2> /dev/null)" ]; then
		$IPT4 -L mwan3_rules -n -v 2> /dev/null | tail -n+3 | sed 's/mark.*//' | sed 's/mwan3_policy_/- /' | sed 's/mwan3_rule_/S /'
	fi
}

mwan3_report_rules_v6()
{
	if [ -n "$($IPT6 -S mwan3_rules 2> /dev/null)" ]; then
		$IPT6 -L mwan3_rules -n -v 2> /dev/null | tail -n+3 | sed 's/mark.*//' | sed 's/mwan3_policy_/- /' | sed 's/mwan3_rule_/S /'
	fi
}

mwan3_flush_conntrack()
{
	local interface="$1"
	local action="$2"

	handle_flush() {
		local flush_conntrack="$1"
		local action="$2"

		if [ "$action" = "$flush_conntrack" ]; then
			echo f > ${CONNTRACK_FILE}
			$LOG info "Connection tracking flushed for interface '$interface' on action '$action'"
		fi
	}

	if [ -e "$CONNTRACK_FILE" ]; then
		config_list_foreach "$interface" flush_conntrack handle_flush "$action"
	fi
}

mwan3_track_clean()
{
	rm -rf "$MWAN3_STATUS_DIR/${1}" &> /dev/null
	[ -d "$MWAN3_STATUS_DIR" ] && {
		if [ -z "$(ls -A "$MWAN3_STATUS_DIR")" ]; then
			rm -rf "$MWAN3_STATUS_DIR"
		fi
	}
}
