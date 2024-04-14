#!/bin/sh /etc/rc.common

START=45
STOP=89

start() {
	service_start /usr/bin/pcat-manager -D
}

stop() {
	service_stop /usr/bin/pcat-manager
}

restart() {
	killall -USR1 pcat-manager
	stop
	start
}

shutdown() {
	touch /tmp/pcat-manager-shutdown.tmp
	stop
	rfkill block wwan 2>/dev/null || true
}
