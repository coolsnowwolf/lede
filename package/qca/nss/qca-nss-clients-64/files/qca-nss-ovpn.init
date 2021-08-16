#!/bin/sh /etc/rc.common

###########################################################################
# Copyright (c) 2019, The Linux Foundation. All rights reserved.
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted, provided that the
# above copyright notice and this permission notice appear in all copies.
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
###########################################################################

ecm_disable() {
	if [ ! -d /sys/module/ecm ]; then
	   return
	fi

	echo 1 > /sys/kernel/debug/ecm/front_end_ipv4_stop
	echo 1 > /sys/kernel/debug/ecm/front_end_ipv6_stop
	echo 1 > /sys/kernel/debug/ecm/ecm_db/defunct_all
	sleep 2
}

ecm_enable() {
	if [ ! -d /sys/module/ecm ]; then
	   return
	fi

	echo 0 > /sys/kernel/debug/ecm/ecm_db/defunct_all
	echo 0 > /sys/kernel/debug/ecm/front_end_ipv4_stop
	echo 0 > /sys/kernel/debug/ecm/front_end_ipv6_stop
}

restart() {
	ecm_disable

	/etc/init.d/openvpn stop
	rmmod qca-nss-ovpn-link
	rmmod qca-nss-ovpn-mgr

	insmod qca-nss-ovpn-mgr
	insmod qca-nss-ovpn-link

	if [ "$?" -gt 0 ]; then
		echo "Failed to load plugin. Please start ecm if not done already"
		ecm_enable
		return
	fi

	ecm_enable
}

start() {
	restart
}

stop() {
	ecm_disable

	/etc/init.d/openvpn stop
	rmmod qca-nss-ovpn-link
	rmmod qca-nss-ovpn-mgr

	ecm_enable
}
