#!/bin/sh
. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

append_args() {
	while [ $# -gt 0 ]; do
		append cmdline "'${1//\'/\'\\\'\'}'"
		shift
	done
}

proto_openconnect_init_config() {
	proto_config_add_string "server"
	proto_config_add_int "port"
	proto_config_add_int "mtu"
	proto_config_add_int "juniper"
	proto_config_add_string "vpn_protocol"
	proto_config_add_boolean "no_dtls"
	proto_config_add_string "interface"
	proto_config_add_string "username"
	proto_config_add_string "serverhash"
	proto_config_add_string "authgroup"
	proto_config_add_string "usergroup"
	proto_config_add_string "password"
	proto_config_add_string "password2"
	proto_config_add_string "token_mode"
	proto_config_add_string "token_secret"
	proto_config_add_string "token_script"
	proto_config_add_string "os"
	proto_config_add_string "csd_wrapper"
	proto_config_add_array 'form_entry:regex("[^:]+:[^=]+=.*")'
	no_device=1
	available=1
}

proto_openconnect_add_form_entry() {
	[ -n "$1" ] && append_args --form-entry "$1"
}

proto_openconnect_setup() {
	local config="$1"

	json_get_vars \
		authgroup \
		csd_wrapper \
		form_entry \
		interface \
		juniper \
		vpn_protocol \
		mtu \
		no_dtls \
		os \
		password \
		password2 \
		port \
		server \
		serverhash \
		token_mode \
		token_script \
		token_secret \
		usergroup \
		username \

	grep -q tun /proc/modules || insmod tun
	ifname="vpn-$config"

	logger -t openconnect "initializing..."

	logger -t "openconnect" "adding host dependency for $server at $config"
	for ip in $(resolveip -t 10 "$server"); do
		logger -t "openconnect" "adding host dependency for $ip at $config"
		proto_add_host_dependency "$config" "$ip" "$interface"
	done

	[ -n "$port" ] && port=":$port"

	append_args "$server$port" -i "$ifname" --non-inter --syslog --script /lib/netifd/vpnc-script
	[ "$no_dtls" = 1 ] && append_args --no-dtls
	[ -n "$mtu" ] && append_args --mtu "$mtu"

	# migrate to standard config files
	[ -f "/etc/config/openconnect-user-cert-vpn-$config.pem" ] && mv "/etc/config/openconnect-user-cert-vpn-$config.pem" "/etc/openconnect/user-cert-vpn-$config.pem"
	[ -f "/etc/config/openconnect-user-key-vpn-$config.pem" ] && mv "/etc/config/openconnect-user-key-vpn-$config.pem" "/etc/openconnect/user-key-vpn-$config.pem"
	[ -f "/etc/config/openconnect-ca-vpn-$config.pem" ] && mv "/etc/config/openconnect-ca-vpn-$config.pem" "/etc/openconnect/ca-vpn-$config.pem"

	[ -f /etc/openconnect/user-cert-vpn-$config.pem ] && append_args -c "/etc/openconnect/user-cert-vpn-$config.pem"
	[ -f /etc/openconnect/user-key-vpn-$config.pem ] && append_args --sslkey "/etc/openconnect/user-key-vpn-$config.pem"
	[ -f /etc/openconnect/ca-vpn-$config.pem ] && {
		append_args --cafile "/etc/openconnect/ca-vpn-$config.pem"
		append_args --no-system-trust
	}

	if [ "${juniper:-0}" -gt 0 ]; then
		append_args --juniper
	fi

	[ -n "$vpn_protocol" ] && {
		append_args --protocol "$vpn_protocol"
	}

	[ -n "$serverhash" ] && {
		append_args "--servercert=$serverhash"
		append_args --no-system-trust
	}
	[ -n "$authgroup" ] && append_args --authgroup "$authgroup"
	[ -n "$usergroup" ] && append_args --usergroup "$usergroup"
	[ -n "$username" ] && append_args -u "$username"
	[ -n "$password" ] || [ "$token_mode" = "script" ] && {
		umask 077
		mkdir -p /var/etc
		pwfile="/var/etc/openconnect-$config.passwd"
		[ -n "$password" ] && {
			echo "$password" > "$pwfile"
			[ -n "$password2" ] && echo "$password2" >> "$pwfile"
		}
		[ "$token_mode" = "script" ] && {
			$token_script >> "$pwfile" 2> /dev/null || {
				logger -t openconenct "Cannot get password from script '$token_script'"
				proto_setup_failed "$config"
			}
		}
		append_args --passwd-on-stdin
	}

	[ -n "$token_mode" -a "$token_mode" != "script" ] && append_args "--token-mode=$token_mode"
	[ -n "$token_secret" ] && append_args "--token-secret=$token_secret"
	[ -n "$os" ] && append_args "--os=$os"
	[ -n "$csd_wrapper" ] && [ -x "$csd_wrapper" ] && append_args "--csd-wrapper=$csd_wrapper"

	json_for_each_item proto_openconnect_add_form_entry form_entry

	proto_export INTERFACE="$config"
	logger -t openconnect "executing 'openconnect $cmdline'"

	if [ -f "$pwfile" ]; then
		eval "proto_run_command '$config' /usr/sbin/openconnect-wrapper '$pwfile' $cmdline"
	else
		eval "proto_run_command '$config' /usr/sbin/openconnect $cmdline"
	fi
}

proto_openconnect_teardown() {
	local config="$1"

	pwfile="/var/etc/openconnect-$config.passwd"

	rm -f $pwfile
	logger -t openconnect "bringing down openconnect"
	proto_kill_command "$config" 2
}

add_protocol openconnect
