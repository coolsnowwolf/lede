#!/bin/sh /etc/rc.common

START=35
STOP=85
USE_PROCD=1

start_service() {
	procd_open_instance
	procd_set_param command /usr/sbin/odhcpd
	procd_set_param respawn
	procd_close_instance
}

reload_service() {
	procd_send_signal odhcpd
}

service_triggers()
{
	procd_add_reload_trigger "dhcp"
}

