#!/bin/sh /etc/rc.common

START=99
USE_PROCD=1

NAME=ps3netsrv
PROG=/usr/bin/ps3netsrv

_info() {
	logger -p daemon.info -t "$NAME" "$*"
}

_err() {
	logger -p daemon.err -t "$NAME" "$*"
}

_change_owner() {
	local u="$1"; shift

	local d
	for d in "$@"; do
		if [ -f "$d" ]; then
			chown "$u" "$d" 2>/dev/null || return 1
		elif [ -d "$d" ]; then
			chown -R "$u" "$d" 2>/dev/null || return 1
		fi
	done

	return 0
}

ps3netsrv_validate() {
	uci_validate_section "$NAME" ps3netsrv "$1" \
		'enabled:bool:0' \
		'dir:string:/root' \
		'user:string' \
		'port:range(1024,65535)'
}

ps3netsrv_start() {
	local section="$1"
	ps3netsrv_validate "$section" || { _err "Validation failed."; return 1; }

	[ "$enabled" = "1" ] || { _info "Instance \"${section}\" disabled."; return 1; }
	[ -n "$dir" ] || { _err "Please set PS3 Game dir."; return 1; }
	[ -n "$port" ] || { _err "Please set bind port."; return 1; }
	[ -d "$dir" ] || { _err "Please create PS3 Game dir first."; return 1; }

	if [ -n "$user" ]; then
		if ( user_exists "$user" && _change_owner "$user" "$log" ); then
				_info "ps3netsrv will run as user '${user}'."
				if [ "$user" != "root" ]; then
					_info "Please make sure user '${user}' has read access to directory: ${dir}"
				fi
		else
			_info "Set run user to '${user}' failed, default user will be used."
			user=
		fi
	fi

	procd_open_instance "${NAME}.${section}"
	procd_set_param command "$PROG" "$dir" "$port"

	procd_set_param respawn
	procd_set_param stdout 1
	procd_set_param stderr 1

	[ -n "$user" ] && \
		procd_set_param user "$user"

	procd_add_jail "${NAME}.${section}" log
	procd_add_jail_mount "$dir"
	procd_close_instance
}

service_triggers() {
	procd_add_reload_trigger "$NAME"
}

start_service() {
	config_load "$NAME"
	config_foreach ps3netsrv_start "ps3netsrv"
}
