#!/bin/sh /etc/rc.common

START=99
STOP=99

SCRIPTS_BASE=/lib/k3screenctrl/

start() {
    /usr/bin/k3screenctrl &
}

stop() {
    killall k3screenctrl
}
