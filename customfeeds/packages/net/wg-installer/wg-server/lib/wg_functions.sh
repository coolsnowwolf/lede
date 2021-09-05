. /usr/share/libubox/jshn.sh
. /usr/share/wginstaller/wg.sh

wg_timeout () {
	local int=$1

	handshake=$(wg show $int latest-handshakes | awk '{print $2}')
	timeout=$(uci get wgserver.@server[0].timeout_handshake)

	if [ $handshake -ge $timeout ]; then
		echo "1"
	else
		echo "0"
	fi
}

wg_check_interface () {
	local int=$1
	if [ $(wg_timeout $int) -eq "1" ]; then
		ip link del dev $int
	fi
}

wg_check_interfaces () {
	wg_interfaces=$(wg show interfaces)
	for interface in $wg_interfaces; do
		wg_check_interface $interface
	done
}

wg_get_usage () {
	num_interfaces=$(wg show interfaces | wc -w)
	json_init
	json_add_int "num_interfaces" $num_interfaces
	echo $(json_dump)
}

wg_register () {
	local uplink_bw=$1
	local mtu=$2
	local public_key=$3

	base_prefix=$(uci get wgserver.@server[0].base_prefix)
	port_start=$(uci get wgserver.@server[0].port_start)
	port_end=$(uci get wgserver.@server[0].port_end)

	port=$(next_port $port_start $port_end)
	ifname="wg_$port"

	offset=$(($port - $port_start))
	gw_ip=$(owipcalc $base_prefix add $offset next 128) # gateway ip
	gw_ip_assign="${gw_ip}/128"

	gw_key=$(uci get wgserver.@server[0].wg_key)
	gw_pub=$(uci get wgserver.@server[0].wg_pub)
	wg_server_pubkey=$(cat $gw_pub)

	# create wg tunnel
	ip link add dev $ifname type wireguard
	wg set $ifname listen-port $port private-key $gw_key peer $public_key allowed-ips ::0/0
	ip -6 a a $gw_ip_assign dev $ifname
	ip -6 a a fe80::1/64 dev $ifname
	ip link set up dev $ifname
	ip link set mtu $mtu dev $ifname

	# craft return address
	json_init
	json_add_string "pubkey" $wg_server_pubkey
	json_add_string "gw_ip" $gw_ip_assign
	json_add_int "port" $port

	echo $(json_dump)
}
