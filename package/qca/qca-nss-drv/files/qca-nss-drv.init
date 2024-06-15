#!/bin/sh /etc/rc.common
#
# Copyright (c) 2015-2017, The Linux Foundation. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

START=70

enable_rps() {
	irq_nss_rps=`grep nss_queue1 /proc/interrupts | cut -d ':' -f 1 | tr -d ' '`
	for entry in $irq_nss_rps
	do
		echo 2 > /proc/irq/$entry/smp_affinity
	done

	irq_nss_rps=`grep nss_queue2 /proc/interrupts | cut -d ':' -f 1 | tr -d ' '`
	for entry in $irq_nss_rps
	do
		echo 4 > /proc/irq/$entry/smp_affinity
	done

	irq_nss_rps=`grep nss_queue3 /proc/interrupts | cut -d ':' -f 1 | tr -d ' '`
	for entry in $irq_nss_rps
	do
		echo 8 > /proc/irq/$entry/smp_affinity
	done

	# Enable NSS RPS
	sysctl -w dev.nss.rps.enable=1 >/dev/null 2>/dev/null

}


start() {
	local rps_enabled="$(uci_get nss @general[0] enable_rps)"
	if [ "$rps_enabled" -eq 1 ]; then
		enable_rps
	fi
}
