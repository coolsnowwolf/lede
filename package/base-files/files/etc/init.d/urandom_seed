#!/bin/sh /etc/rc.common

START=99
USE_PROCD=1

start_service() {
    procd_open_instance "urandom_seed"
    procd_set_param command "/sbin/urandom_seed"
    procd_set_param stdout 1
    procd_set_param stderr 1
    procd_close_instance
}
