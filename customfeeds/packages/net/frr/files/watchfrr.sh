#!/bin/sh
#
# This is NOT the init script!  This is the watchfrr start/stop/restart
# command handler, passed to watchfrr with the -s/-r/-k commands.  It is used
# internally by watchfrr to start the protocol daemons with the appropriate
# options.
#
# This script should be installed in  /usr/sbin/watchfrr.sh

log_success_msg() {
	:
}

log_warning_msg() {
	echo "$@" >&2
	[ -x /usr/bin/logger ] && echo "$@" \
		| /usr/bin/logger -t watchfrr.sh -p daemon.warn
}

log_failure_msg() {
	echo "$@" >&2
	[ -x /usr/bin/logger ] && echo "$@" \
		| /usr/bin/logger -t watchfrr.sh -p daemon.err
}

self="`dirname $0`"
if [ -r "$self/frrcommon.sh" ]; then
	. "$self/frrcommon.sh"
else
	. "/usr/sbin/frrcommon.sh"
fi

chownfrr $V_PATH

frrcommon_main "$@"
