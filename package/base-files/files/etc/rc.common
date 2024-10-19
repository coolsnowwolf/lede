#!/bin/sh
# Copyright (C) 2006-2012 OpenWrt.org

. $IPKG_INSTROOT/lib/functions.sh
. $IPKG_INSTROOT/lib/functions/service.sh

initscript=$1
action=${2:-help}
shift 2

start() {
	return 0
}

stop() {
	return 0
}

reload() {
	restart
}

restart() {
	trap '' TERM
	stop "$@"
	trap - TERM
	start "$@"
}

boot() {
	start "$@"
}

shutdown() {
	stop
}

disable() {
	name="$(basename "${initscript}")"
	rm -f "$IPKG_INSTROOT"/etc/rc.d/S??$name
	rm -f "$IPKG_INSTROOT"/etc/rc.d/K??$name
}

enable() {
	err=1
	name="$(basename "${initscript}")"
	[ "$START" ] && \
		ln -sf "../init.d/$name" "$IPKG_INSTROOT/etc/rc.d/S${START}${name##S[0-9][0-9]}" && \
		err=0
	[ "$STOP" ] && \
		ln -sf "../init.d/$name" "$IPKG_INSTROOT/etc/rc.d/K${STOP}${name##K[0-9][0-9]}" && \
		err=0
	return $err
}

enabled() {
	name="$(basename "${initscript}")"
	name="${name##[SK][0-9][0-9]}"
	{
		[ -z "${START:-}" ] || [ -L "$IPKG_INSTROOT/etc/rc.d/S${START}$name" ]
	} && {
		[ -z "${STOP:-}" ] || [ -L "$IPKG_INSTROOT/etc/rc.d/K${STOP}$name" ]
	}
}

depends() {
	return 0
}

ALL_HELP=""
ALL_COMMANDS="boot shutdown depends"
extra_command() {
	local cmd="$1"
	local help="$2"

	local extra="$(printf "%-16s%s" "${cmd}" "${help}")"
	ALL_HELP="${ALL_HELP}\t${extra}\n"
	ALL_COMMANDS="${ALL_COMMANDS} ${cmd}"
}

help() {
	cat <<EOF
Syntax: $initscript [command]

Available commands:
EOF
	echo -e "$ALL_HELP"
}

# for procd
start_service() {
	return 0
}

stop_service() {
	return 0
}

service_triggers() {
	return 0
}

service_data() {
	return 0
}

service_running() {
	local instance="${1:-*}"

	procd_running "$(basename $initscript)" "$instance"
}

${INIT_TRACE:+set -x}

extra_command "start" "Start the service"
extra_command "stop" "Stop the service"
extra_command "restart" "Restart the service"
extra_command "reload" "Reload configuration files (or restart if service does not implement reload)"
extra_command "enable" "Enable service autostart"
extra_command "disable" "Disable service autostart"
extra_command "enabled" "Check if service is started on boot"

. "$initscript"

[ -n "$USE_PROCD" ] && {
	extra_command "running" "Check if service is running"
	extra_command "status" "Service status"
	extra_command "trace" "Start with syscall trace"
	extra_command "info" "Dump procd service info"

	. $IPKG_INSTROOT/lib/functions/procd.sh
	basescript=$(readlink "$initscript")
	rc_procd() {
		local method="set"
		[ -n "$2" ] && method="add"
		procd_open_service "$(basename ${basescript:-$initscript})" "$initscript"
		"$@"
		procd_close_service "$method"
	}

	start() {
		rc_procd start_service "$@"
		if eval "type service_started" 2>/dev/null >/dev/null; then
			service_started
		fi
	}

	trace() {
		TRACE_SYSCALLS=1
		start "$@"
	}

	info() {
		json_init
		json_add_string name "$(basename ${basescript:-$initscript})"
		json_add_boolean verbose "1"
		_procd_ubus_call list
	}

	stop() {
		procd_lock
		stop_service "$@"
		procd_kill "$(basename ${basescript:-$initscript})" "$1"
		if eval "type service_stopped" 2>/dev/null >/dev/null; then
			service_stopped
		fi
	}

	reload() {
		if eval "type reload_service" 2>/dev/null >/dev/null; then
			procd_lock
			reload_service "$@"
		else
			start
		fi
	}

	running() {
		service_running "$@"
	}

	status() {
		if eval "type status_service" 2>/dev/null >/dev/null; then
			status_service "$@"
		else
			_procd_status "$(basename ${basescript:-$initscript})" "$1"
		fi
	}
}

ALL_COMMANDS="${ALL_COMMANDS} ${EXTRA_COMMANDS}"
ALL_HELP="${ALL_HELP}${EXTRA_HELP}"
list_contains ALL_COMMANDS "$action" || action=help
$action "$@"
