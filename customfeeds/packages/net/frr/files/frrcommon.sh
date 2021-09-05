#!/bin/sh
#
#
# This is a "library" of sorts for use by the other FRR shell scripts.  It
# has most of the daemon start/stop logic, but expects the following shell
# functions/commands to be provided by the "calling" script:
#
#   log_success_msg
#   log_warning_msg
#   log_failure_msg
#
# (coincidentally, these are LSB standard functions.)
#
# Sourcing this file in a shell script will load FRR config variables but
# not perform any action.  Note there is an "exit 1" if the main config
# file does not exist.
#
# This script should be installed in  /usr/sbin/frrcommon.sh
# FRR_PATHSPACE is passed in from watchfrr
suffix="${FRR_PATHSPACE:+/${FRR_PATHSPACE}}"
nsopt="${FRR_PATHSPACE:+-N ${FRR_PATHSPACE}}"

PATH=/bin:/usr/bin:/sbin:/usr/sbin
D_PATH="/usr/sbin" # /usr/lib/frr
C_PATH="/etc/frr" # /etc/frr
V_PATH="/var/run/frr" # /var/run/frr
VTYSH="/usr/bin/vtysh" # /usr/bin/vtysh
FRR_USER="network" # frr
FRR_GROUP="network" # frr
FRR_VTY_GROUP="" # frrvty
FRR_CONFIG_MODE="0600" # 0600
FRR_DEFAULT_PROFILE="traditional" # traditional / datacenter

# ORDER MATTERS FOR $DAEMONS!
# - keep zebra first
# - watchfrr does NOT belong in this list

DAEMONS="zebra bgpd ripd ripngd ospfd ospf6d isisd babeld pimd ldpd nhrpd eigrpd sharpd pbrd staticd bfdd fabricd vrrpd"
RELOAD_SCRIPT="$D_PATH/frr-reload.py"

#
# general helpers
#

debug() {
	[ -n "$watchfrr_debug" ] || return 0

	printf '%s %s(%s):' "`date +%Y-%m-%dT%H:%M:%S.%N`" "$0" $$ >&2
	# this is to show how arguments are split regarding whitespace & co.
	# (e.g. for use with `debug "message" "$@"`)
	while [ $# -gt 0 ]; do
		printf ' "%s"' "$1" >&2
		shift
	done
	printf '\n' >&2
}

chownfrr() {
	[ -n "$FRR_USER" ] && chown "$FRR_USER" "$1"
	[ -n "$FRR_GROUP" ] && chgrp "$FRR_GROUP" "$1"
	[ -n "$FRR_CONFIG_MODE" ] && chmod "$FRR_CONFIG_MODE" "$1"
	if [ -d "$1" ]; then
		chmod u+x "$1"
	fi
}

vtysh_b () {
	[ "$1" = "watchfrr" ] && return 0
	[ -r "$C_PATH/frr.conf" ] || return 0
	if [ -n "$1" ]; then
		"$VTYSH" `echo $nsopt` -b -d "$1"
	else
		"$VTYSH" `echo $nsopt` -b
	fi
}

daemon_inst() {
	# note this sets global variables ($dmninst, $daemon, $inst)
	dmninst="$1"
	daemon="${dmninst%-*}"
	inst=""
	[ "$daemon" != "$dmninst" ] && inst="${dmninst#*-}"
}

daemon_list() {
	# note $1 and $2 specify names for global variables to be set
	local enabled disabled evar dvar
	enabled=""
	disabled=""
	evar="$1"
	dvar="$2"

	for daemon in $DAEMONS; do
		eval cfg=\$$daemon
		eval inst=\$${daemon}_instances
		[ "$daemon" = zebra -o "$daemon" = staticd ] && cfg=yes
		if [ -n "$cfg" -a "$cfg" != "no" -a "$cfg" != "0" ]; then
			if ! daemon_prep "$daemon" "$inst"; then
				continue
			fi
			debug "$daemon enabled"
#			enabled="$enabled $daemon"

			if [ -n "$inst" ]; then
				debug "$daemon multi-instance $inst"
				oldifs="${IFS}"
				IFS="${IFS},"
				for i in $inst; do
					enabled="$enabled $daemon-$i"
				done
				IFS="${oldifs}"
			else
				enabled="$enabled $daemon"
			fi
		else
			debug "$daemon disabled"
			disabled="$disabled $daemon"
		fi
	done

	enabled="${enabled# }"
	disabled="${disabled# }"
	[ -z "$evar" ] && echo "$enabled"
	[ -n "$evar" ] && eval $evar="\"$enabled\""
	[ -n "$dvar" ] && eval $dvar="\"$disabled\""
}

#
# individual daemon management
#

daemon_prep() {
	local daemon inst cfg
	daemon="$1"
	inst="$2"
	[ "$daemon" = "watchfrr" ] && return 0
	[ -x "$D_PATH/$daemon" ] || {
		log_failure_msg "cannot start $daemon${inst:+ (instance $inst)}: daemon binary not installed"
		return 1
	}
	[ -r "$C_PATH/frr.conf" ] && return 0

	cfg="$C_PATH/$daemon${inst:+-$inst}.conf"
	if [ ! -r "$cfg" ]; then
		touch "$cfg"
		chownfrr "$cfg"
	fi
	return 0
}

daemon_start() {
	local dmninst daemon inst args instopt wrap bin
	daemon_inst "$1"

	ulimit -n $MAX_FDS > /dev/null 2> /dev/null
	daemon_prep "$daemon" "$inst" || return 1
	if test ! -d "$V_PATH"; then
		mkdir -p "$V_PATH"
		chown $FRR_USER "$V_PATH"
	fi

	eval wrap="\$${daemon}_wrap"
	bin="$D_PATH/$daemon"
	instopt="${inst:+-n $inst}"
	eval args="\$${daemon}_options"

	if eval "$all_wrap $wrap $bin $nsopt -d $frr_global_options $instopt $args"; then
		log_success_msg "Started $dmninst"
		vtysh_b "$daemon"
	else
		log_failure_msg "Failed to start $dmninst!"
	fi
}

daemon_stop() {
	local dmninst daemon inst pidfile vtyfile pid cnt fail
	daemon_inst "$1"

	pidfile="$V_PATH/$daemon${inst:+-$inst}.pid"
	vtyfile="$V_PATH/$daemon${inst:+-$inst}.vty"

	[ -r "$pidfile" ] || fail="pid file not found"
	[ -z "$fail" ] && pid="`cat \"$pidfile\"`"
	[ -z "$fail" -a -z "$pid" ] && fail="pid file is empty"
	[ -n "$fail" ] || kill -0 "$pid" 2>/dev/null || fail="pid $pid not running"

	if [ -n "$fail" ]; then
		log_failure_msg "Cannot stop $dmninst: $fail"
		return 1
	fi

	debug "kill -2 $pid"
	kill -2 "$pid"
	cnt=1200
	while kill -0 "$pid" 2>/dev/null; do
		sleep 1
		[ $(( cnt -= 1 )) -gt 0 ] || break
	done
	if kill -0 "$pid" 2>/dev/null; then
		log_failure_msg "Failed to stop $dmninst, pid $pid still running"
		still_running=1
		return 1
	else
		log_success_msg "Stopped $dmninst"
		rm -f "$pidfile"
		return 0
	fi
}

daemon_status() {
	local dmninst daemon inst pidfile pid fail
	daemon_inst "$1"

	pidfile="$V_PATH/$daemon${inst:+-$inst}.pid"

	[ -r "$pidfile" ] || return 3
	pid="`cat \"$pidfile\"`"
	[ -z "$pid" ] && return 1
	kill -0 "$pid" 2>/dev/null || return 1
	return 0
}

print_status() {
	daemon_status "$1"
	rv=$?
	if [ "$rv" -eq 0 ]; then
		log_success_msg "Status of $1: running"
	else
		log_failure_msg "Status of $1: FAILED"
	fi
	return $rv
}

#
# all-daemon commands
#

all_start() {
	daemon_list daemons
	for dmninst in $daemons; do
		daemon_start "$dmninst"
	done
}

all_stop() {
	local pids reversed

	daemon_list daemons disabled
	[ "$1" = "--reallyall" ] && daemons="$daemons $disabled"

	reversed=""
	for dmninst in $daemons; do
		reversed="$dmninst $reversed"
	done

	for dmninst in $reversed; do
		daemon_stop "$dmninst" &
		pids="$pids $!"
	done
	for pid in $pids; do
		wait $pid
	done
}

all_status() {
	local fail

	daemon_list daemons
	fail=0
	for dmninst in $daemons; do
		print_status "$dmninst" || fail=1
	done
	return $fail
}

#
# config sourcing
#

load_old_config() {
	oldcfg="$1"
	[ -r "$oldcfg" ] || return 0
	[ -s "$oldcfg" ] || return 0
	grep -v '^[[:blank:]]*\(#\|$\)' "$oldcfg" > /dev/null || return 0

	log_warning_msg "Reading deprecated $oldcfg.  Please move its settings to $C_PATH/daemons and remove it."

	# save off settings from daemons for the OR below
	for dmn in $DAEMONS; do eval "_new_$dmn=\${$dmn:-no}"; done

	. "$oldcfg"

	# OR together the daemon enabling options between config files
	for dmn in $DAEMONS; do eval "test \$_new_$dmn != no && $dmn=\$_new_$dmn; unset _new_$dmn"; done
}

[ -r "$C_PATH/daemons" ] || {
	log_failure_msg "cannot run $@: $C_PATH/daemons does not exist"
	exit 1
}
. "$C_PATH/daemons"

if [ -z "$FRR_PATHSPACE" ]; then
	load_old_config "$C_PATH/daemons.conf"
	load_old_config "/etc/default/frr"
	load_old_config "/etc/sysconfig/frr"
fi

if { declare -p watchfrr_options 2>/dev/null || true; } | grep -q '^declare \-a'; then
	log_warning_msg "watchfrr_options contains a bash array value." \
		"The configured value is intentionally ignored since it is likely wrong." \
		"Please remove or fix the setting."
	unset watchfrr_options
fi

if test -z "$frr_profile"; then
	# try to autodetect config profile
	if test -d /etc/cumulus; then
		frr_profile=datacenter
	# elif test ...; then
	# -- add your distro/system here
	elif test -n "$FRR_DEFAULT_PROFILE"; then
		frr_profile="$FRR_DEFAULT_PROFILE"
	fi
fi
test -n "$frr_profile" && frr_global_options="$frr_global_options -F $frr_profile"


#
# other defaults and dispatch
#

frrcommon_main() {
	local cmd

	debug "frrcommon_main" "$@"

	cmd="$1"
	shift

	if [ "$1" = "all" -o -z "$1" ]; then
		case "$cmd" in
		start)	all_start;;
		stop)	all_stop;;
		restart)
			all_stop
			all_start
			;;
		*)	$cmd "$@";;
		esac
	else
		case "$cmd" in
		start)	daemon_start "$@";;
		stop)	daemon_stop "$@";;
		restart)
			daemon_stop "$@"
			daemon_start "$@"
			;;
		*)	$cmd "$@";;
		esac
	fi
}
