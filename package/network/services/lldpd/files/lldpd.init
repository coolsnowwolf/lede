#!/bin/sh /etc/rc.common
# Copyright (C) 2008-2015 OpenWrt.org

START=90
STOP=01

USE_PROCD=1
LLDPCLI=/usr/sbin/lldpcli
LLDPSOCKET=/var/run/lldpd.socket
LLDPD_CONF=/tmp/lldpd.conf
LLDPD_CONFS_DIR=/tmp/lldpd.d

find_release_info()
{
	[ -s /etc/os-release ] && . /etc/os-release
	[ -z "$PRETTY_NAME" ] && [ -s /etc/openwrt_version ] && \
		PRETTY_NAME="$(cat /etc/openwrt_version)"

	echo "${PRETTY_NAME:-Unknown OpenWrt release} @ $(cat /proc/sys/kernel/hostname)"
}

write_lldpd_conf()
{
	. /lib/functions/network.sh

	local lldp_description

	config_load 'lldpd'
	config_get lldp_description 'config' 'lldp_description' "$(find_release_info)"

	local lldp_hostname
	config_get lldp_hostname 'config' 'lldp_hostname' "$(cat /proc/sys/kernel/hostname)"

	local ifaces
	config_get ifaces 'config' 'interface'

	local iface ifnames=""
	for iface in $ifaces; do
		local ifname=""
		if network_get_device ifname "$iface" || [ -e "/sys/class/net/$iface" ]; then
			append ifnames "${ifname:-$iface}" ","
		fi
	done

	local lldp_mgmt_ip
	config_get lldp_mgmt_ip 'config' 'lldp_mgmt_ip'

	# Clear out the config file first
	echo -n > "$LLDPD_CONF"
	[ -n "$ifnames" ] && echo "configure system interface pattern" "$ifnames" >> "$LLDPD_CONF"
	[ -n "$lldp_description" ] && echo "configure system description" "\"$lldp_description\"" >> "$LLDPD_CONF"
	[ -n "$lldp_hostname" ] && echo "configure system hostname" "\"$lldp_hostname\"" >> "$LLDPD_CONF"
	[ -n "$lldp_mgmt_ip" ] && echo "configure system ip management pattern" "\"$lldp_mgmt_ip\"" >> "$LLDPD_CONF"

	# Since lldpd's sysconfdir is /tmp, we'll symlink /etc/lldpd.d to /tmp/$LLDPD_CONFS_DIR
	[ -e $LLDPD_CONFS_DIR ] || ln -s /etc/lldpd.d $LLDPD_CONFS_DIR
}

service_triggers() {
	procd_add_reload_trigger "lldpd"
}

start_service() {

	local enable_cdp
	local enable_fdp
	local enable_sonmp
	local enable_edp
	local lldp_class
	local lldp_location
	local readonly_mode
	local agentxsocket

	config_load 'lldpd'
	config_get_bool enable_cdp 'config' 'enable_cdp' 0
	config_get_bool enable_fdp 'config' 'enable_fdp' 0
	config_get_bool enable_sonmp 'config' 'enable_sonmp' 0
	config_get_bool enable_edp 'config' 'enable_edp' 0
	config_get lldp_class 'config' 'lldp_class'
	config_get lldp_location 'config' 'lldp_location'
	config_get_bool readonly_mode 'config' 'readonly_mode' 0
	config_get agentxsocket 'config' 'agentxsocket'

	mkdir -p /var/run/lldp
	chown lldp:lldp /var/run/lldp

	# When lldpd starts, it also loads up what we write in this config file
	write_lldpd_conf

	procd_open_instance
	procd_set_param command /usr/sbin/lldpd -d

	[ $enable_cdp -gt 0 ] && procd_append_param command '-c'
	[ $enable_fdp -gt 0 ] && procd_append_param command '-f'
	[ $enable_sonmp -gt 0 ] && procd_append_param command '-s'
	[ $enable_edp -gt 0 ] && procd_append_param command '-e'
	[ $readonly_mode -gt 0 ] && procd_append_param command '-r'
	[ -n "$lldp_class" ] && procd_append_param command -M "$lldp_class"
	[ -n "$agentxsocket" ] && procd_append_param command -x -X "$agentxsocket"

	# set auto respawn behavior
	procd_set_param respawn
	procd_close_instance
}

reload_service() {
	running || return 1
	$LLDPCLI -u $LLDPSOCKET &> /dev/null <<-EOF
		pause
		unconfigure lldp custom-tlv
		unconfigure system interface pattern
		unconfigure system description
		unconfigure system hostname
		unconfigure system ip management pattern
	EOF
	# Rewrite lldpd.conf
	# If something changed it should be included by the lldpcli call
	write_lldpd_conf
	$LLDPCLI -u $LLDPSOCKET -c $LLDPD_CONF -c $LLDPD_CONFS_DIR &> /dev/null
	# Broadcast update over the wire
	$LLDPCLI -u $LLDPSOCKET &> /dev/null <<-EOF
		resume
		update
	EOF
	return 0
}

stop_service() {
	rm -rf /var/run/lldp $LLDPSOCKET
}
