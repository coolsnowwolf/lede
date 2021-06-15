#!/bin/sh /etc/rc.common

START=19
STOP=21

USE_PROCD=1
NAME=wpad

start_service() {
	if [ -x "/usr/sbin/hostapd" ]; then
		mkdir -p /var/run/hostapd
		chown network:network /var/run/hostapd
		procd_open_instance hostapd
		procd_set_param command /usr/sbin/hostapd -s -g /var/run/hostapd/global
		procd_set_param respawn 3600 1 0
		[ -x /sbin/ujail -a -e /etc/capabilities/wpad.json ] && {
			procd_add_jail hostapd
			procd_set_param capabilities /etc/capabilities/wpad.json
			procd_set_param user network
			procd_set_param group network
			procd_set_param no_new_privs 1
		}
		procd_close_instance
	fi

	if [ -x "/usr/sbin/wpa_supplicant" ]; then
		mkdir -p /var/run/wpa_supplicant
		chown network:network /var/run/wpa_supplicant
		procd_open_instance supplicant
		procd_set_param command /usr/sbin/wpa_supplicant -n -s -g /var/run/wpa_supplicant/global
		procd_set_param respawn 3600 1 0
		[ -x /sbin/ujail -a -e /etc/capabilities/wpad.json ] && {
			procd_add_jail wpa_supplicant
			procd_set_param capabilities /etc/capabilities/wpad.json
			procd_set_param user network
			procd_set_param group network
			procd_set_param no_new_privs 1
		}
		procd_close_instance
	fi
}
