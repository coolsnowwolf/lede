#!/bin/sh
. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_vpnc_init_config() {
	proto_config_add_string "server"
	proto_config_add_string "username"
	proto_config_add_string "hexpasswd"
	proto_config_add_string "authgroup"
	proto_config_add_string "password"
	proto_config_add_string "token_mode"
	proto_config_add_string "token_secret"
	proto_config_add_string "interface"
	proto_config_add_string "passgroup"
	proto_config_add_string "hexpassgroup"
	proto_config_add_string "domain"
	proto_config_add_string "vendor"
	proto_config_add_string "natt_mode"
	proto_config_add_string "dh_group"
	proto_config_add_string "pfs"
	proto_config_add_boolean "enable_single_des"
	proto_config_add_boolean "enable_no_enc"
	proto_config_add_int "mtu"
	proto_config_add_string "local_addr"
	proto_config_add_int "local_port"
	proto_config_add_int "udp_port"
	proto_config_add_int "dpd_idle"
	proto_config_add_string "auth_mode"
	proto_config_add_string "target_network"
	proto_config_add_boolean "authfail"
	no_device=1
	available=1
}

proto_vpnc_setup() {
	local config="$1"

	json_get_vars server username hexpasswd authgroup password token_mode token_secret interface passgroup hexpassgroup domain vendor natt_mode dh_group pfs enable_single_des enable_no_enc mtu local_addr local_port udp_port dpd_idle auth_mode target_network authfail

	grep -q tun /proc/modules || insmod tun

	logger -t vpnc "initializing..."
	serv_addr=
	for ip in $(resolveip -4t 10 "$server"); do
		( proto_add_host_dependency "$config" "$ip" $interface )
		serv_addr=1
	done
	[ -n "$serv_addr" ] || {
		logger -t vpnc "Could not resolve server address: '$server'"
		sleep 60
		proto_setup_failed "$config"
		exit 1
	}

	mkdir -p /var/etc
	umask 077
	pwfile="/var/etc/vpnc-$config.conf"
	echo "IPSec gateway $server" > "$pwfile"
	cmdline="--no-detach --pid-file /var/run/vpnc-$config.pid --ifname vpn-$config --non-inter --script /lib/netifd/vpnc-script $pwfile"

	[ -f /etc/vpnc/ca-vpn-$config.pem ] && echo "CA-File /etc/vpnc/ca-vpn-$config.pem" >> "$pwfile"
	[ -n "$hexpasswd" ] && echo "Xauth obfuscated password $hexpasswd" >> "$pwfile"
	[ -n "$authgroup" ] && echo "IPSec ID $authgroup" >> "$pwfile"
	[ -n "$username" ] && echo "Xauth username $username" >> "$pwfile"
	[ -n "$password" ] && echo "Xauth password $password" >> "$pwfile"
	[ -n "$passgroup" ] && echo "IPSec secret $passgroup" >> "$pwfile"
	[ -n "$hexpassgroup" ] && echo "IPSec obfuscated secret $hexpassgroup" >> "$pwfile"
	[ -n "$domain" ] && echo "Domain $domain" >> "$pwfile"
	[ -n "$vendor" ] && echo "Vendor $vendor" >> "$pwfile"
	[ -n "$natt_mode" ] && echo "NAT Traversal Mode $natt_mode" >> "$pwfile"
	[ -n "$dh_group" ] && echo "IKE DH Group $dh_group" >> "$pwfile"
	[ -n "$pfs" ] && echo "Perfect Forward Secrecy $pfs" >> "$pwfile"
	[ "${enable_single_des:-0}" -gt 0 ] && echo "Enable Single DES" >> "$pwfile"
	[ "${enable_no_enc:-0}" -gt 0 ] && echo "Enable no encryption" >> "$pwfile"
	[ -n "$mtu" ] && echo "Interface MTU $mtu" >> "$pwfile"
	[ -n "$local_addr" ] && echo "Local Addr $local_addr" >> "$pwfile"
	[ -n "$local_port" ] && echo "Local Port $local_port" >> "$pwfile"
	[ -n "$udp_port" ] && echo "Cisco UDP Encapsulation Port $udp_port" >> "$pwfile"
	[ -n "$dpd_idle" ] && echo "DPD idle timeout (our side) $dpd_idle" >> "$pwfile"
	[ -n "$auth_mode" ] && echo "IKE Authmode $auth_mode" >> "$pwfile"
	[ -n "$target_network" ] && echo "IPSEC target network $target_network" >> "$pwfile"

	proto_export INTERFACE="$config"
	logger -t vpnc "executing 'vpnc $cmdline'"
	proto_run_command "$config" /usr/sbin/vpnc $cmdline
}

proto_vpnc_teardown() {
	local config="$1"

	pwfile="/var/etc/vpnc-$config.conf"

	json_get_var authfail authfail
	# On error exit (vpnc only has success = 0 and error = 1, so
	# we can't be fine-grained and say only auth error)
	# and authfail setting true, then don't retry starting vpnc
	# This is used for the case were the server blocks repeated
	# failed authentication attempts (which will occur if the password
	# is wrong, for example).
	if [ ${ERROR:-0} -gt 0 ] && [ "${authfail:-0}" -gt 0 ]; then
		proto_block_restart "$config"
	fi

	rm -f $pwfile
	logger -t vpnc "bringing down vpnc"
	proto_kill_command "$config" 2
}

add_protocol vpnc
