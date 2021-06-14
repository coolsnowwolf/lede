#
# service: simple wrapper around start-stop-daemon
#
# Usage: service ACTION EXEC ARGS...
#
# Action:
#   -C	check if EXEC is alive
#   -S	start EXEC, passing it ARGS as its arguments
#   -K	kill EXEC, sending it a TERM signal if not specified otherwise
#
# Environment variables exposed:
#   SERVICE_DAEMONIZE	run EXEC in background
#   SERVICE_WRITE_PID	create a pid-file and use it for matching
#   SERVICE_MATCH_EXEC	use EXEC command-line for matching (default)
#   SERVICE_MATCH_NAME	use EXEC process name for matching
#   SERVICE_USE_PID	assume EXEC create its own pid-file and use it for matching
#   SERVICE_NAME	process name to use (default to EXEC file part)
#   SERVICE_PID_FILE	pid file to use (default to /var/run/$SERVICE_NAME.pid)
#   SERVICE_SIG		signal to send when using -K
#   SERVICE_SIG_RELOAD	default signal used when reloading
#   SERVICE_SIG_STOP	default signal used when stopping
#   SERVICE_STOP_TIME	time to wait for a process to stop gracefully before killing it
#   SERVICE_UID		user EXEC should be run as
#   SERVICE_GID		group EXEC should be run as
#
#   SERVICE_DEBUG	don't do anything, but show what would be done
#   SERVICE_QUIET	don't print anything
#

SERVICE_QUIET=1
SERVICE_SIG_RELOAD="HUP"
SERVICE_SIG_STOP="TERM"
SERVICE_STOP_TIME=5
SERVICE_MATCH_EXEC=1

service() {
	local ssd
	local exec
	local name
	local start
	ssd="${SERVICE_DEBUG:+echo }start-stop-daemon${SERVICE_QUIET:+ -q}"
	case "$1" in
	  -C)
		ssd="$ssd -K -t"
		;;
	  -S)
		ssd="$ssd -S${SERVICE_DAEMONIZE:+ -b}${SERVICE_WRITE_PID:+ -m}"
		start=1
		;;
	  -K)
		ssd="$ssd -K${SERVICE_SIG:+ -s $SERVICE_SIG}"
		;;
	  *)
		echo "service: unknown ACTION '$1'" 1>&2
		return 1
	esac
	shift
	exec="$1"
	[ -n "$exec" ] || {
		echo "service: missing argument" 1>&2
		return 1
	}
	[ -x "$exec" ] || {
		echo "service: file '$exec' is not executable" 1>&2
		return 1
	}
	name="${SERVICE_NAME:-${exec##*/}}"
	[ -z "$SERVICE_USE_PID$SERVICE_WRITE_PID$SERVICE_PID_FILE" ] \
		|| ssd="$ssd -p ${SERVICE_PID_FILE:-/var/run/$name.pid}"
	[ -z "$SERVICE_MATCH_NAME" ] || ssd="$ssd -n $name"
	ssd="$ssd${SERVICE_UID:+ -c $SERVICE_UID${SERVICE_GID:+:$SERVICE_GID}}"
	[ -z "$SERVICE_MATCH_EXEC$start" ] || ssd="$ssd -x $exec"
	shift
	$ssd${1:+ -- "$@"}
}

service_check() {
	service -C "$@"
}

service_signal() {
	SERVICE_SIG="${SERVICE_SIG:-USR1}" service -K "$@"
}

service_start() {
	service -S "$@"
}

service_stop() {
	local try
	SERVICE_SIG="${SERVICE_SIG:-$SERVICE_SIG_STOP}" service -K "$@" || return 1
	while [ $((try++)) -lt $SERVICE_STOP_TIME ]; do
		service -C "$@" || return 0
		sleep 1
	done
	SERVICE_SIG="KILL" service -K "$@"
	sleep 1
	! service -C "$@"
}

service_reload() {
	SERVICE_SIG="${SERVICE_SIG:-$SERVICE_SIG_RELOAD}" service -K "$@"
}
