#!/bin/sh /etc/rc.common
# Copyright (C) 2006 OpenWrt.org

START=50
USE_PROCD=1

validate_qos_section()
{
	uci_validate_section qos interface "${1}" \
		'enabled:bool' \
		'upload:uinteger' \
		'download:uinteger'
}

service_triggers()
{
	procd_add_reload_trigger "qos"
	procd_add_validation validate_qos_section
	qos-start
}

start_service() {
	qos-start
}

reload_service() {
	qos-start
}
