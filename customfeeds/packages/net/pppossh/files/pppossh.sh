#!/bin/sh

SSH=/usr/bin/ssh
[ -x "$SSH" ] || {
	echo "Cannot find executable $SSH." >&2
	exit 1
}

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

INCLUDE_ONLY=1

. ./ppp.sh

proto_pppossh_init_config() {
	ppp_generic_init_config
	config_add_string server sshuser ipaddr peeraddr ssh_options
	config_add_array 'identity:list(string)'
	config_add_int port use_hostdep
	available=1
	no_device=1
}

proto_pppossh_setup() {
	local config="$1"
	local iface="$2"
	local user="$(id -nu)"
	local home=$(sh -c "echo ~$user")
	local server port sshuser ipaddr peeraddr ssh_options identity use_hostdep
	local ip fn errmsg opts pty

	json_get_vars port sshuser ipaddr peeraddr ssh_options use_hostdep
	json_get_var server server && {
		[ -z "$use_hostdep" ] && use_hostdep=1
		for ip in $(resolveip -t 5 "$server"); do
			if [ "$use_hostdep" -gt 0 ]; then
				( proto_add_host_dependency "$config" "$ip" )
			else
				break
			fi
		done
	}
	[ -n "$ip" ] || errmsg="${errmsg}Could not resolve $server\n"
	[ -n "$sshuser" ] || errmsg="${errmsg}Missing sshuser option\n"

	json_get_values identity identity
	[ -z "$identity" ] && identity="$home/.ssh/id_rsa $home/.ssh/id_dsa"
	for fn in $identity; do
		[ -f "$fn" ] && opts="$opts -i $fn"
	done
	[ -n "$opts" ] || errmsg="${errmsg}Cannot find valid identity file\n"

	[ -n "$errmsg" ] && {
		echo -ne "$errmsg" >&2
		proto_setup_failed "$config"
		exit 1
	}
	opts="$opts ${port:+-p $port}"
	opts="$opts ${ssh_options}"
	opts="$opts $sshuser@$server"
	pty="exec env 'HOME=$home' $SSH $opts pppd nodetach notty noauth"

	ppp_generic_setup "$config" noauth pty "$pty" "$ipaddr:$peeraddr"
}

proto_pppossh_teardown() {
	ppp_generic_teardown "$@"
}

add_protocol pppossh
