#!/bin/sh /etc/rc.common

START=99
STOP=01

USE_PROCD=1

start_service() {
    procd_open_instance mtkiappd
    procd_set_param command /usr/sbin/mtkiappd -e br-lan WIRELESS_IOCTL_PARAM
    procd_set_param respawn
    procd_set_param netdev br-lan
    procd_set_param netdev ra0
    procd_close_instance
}

reload_service() {
    stop
    start
}
