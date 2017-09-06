#!/bin/sh /etc/rc.common
# Copyright (C) 2016 OpenWrt.org

START=99

USE_PROCD=1
PROG=/usr/bin/bugchecker.sh

# To actually make bugchecker.sh run, see comments
# at top of its file.

start_service () {
        procd_open_instance
        procd_set_param command "$PROG"
        procd_close_instance
}
