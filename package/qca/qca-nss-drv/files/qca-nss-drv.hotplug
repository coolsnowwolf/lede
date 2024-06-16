#!/bin/sh
#
# Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
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

KERNEL=`uname -r`
case "${KERNEL}" in
	3.4*)
		select_or_load=load_nss_fw
		;;
	*)
		select_or_load=select_nss_fw
		;;
esac

load_nss_fw () {
	ls -l $1 | awk ' { print $9,$5 } '> /dev/console
	echo 1 > /sys/class/firmware/$DEVICENAME/loading
	cat $1 > /sys/class/firmware/$DEVICENAME/data
	echo 0 > /sys/class/firmware/$DEVICENAME/loading
}

select_nss_fw () {
	rm -f /lib/firmware/$DEVICENAME
	ln -s $1 /lib/firmware/$DEVICENAME
	ls -l /lib/firmware/$DEVICENAME | awk ' { print $9,$5 } '> /dev/console
}

[ "$ACTION" != "add" ] && exit

# dev name for UCI, since it doesn't let you use . or -
SDEVNAME=$(echo ${DEVICENAME} | sed s/[.-]/_/g)

SELECTED_FW=$(uci get nss.${SDEVNAME}.firmware 2>/dev/null)
[ -e "${SELECTED_FW}" ] && {
	$select_or_load ${SELECTED_FW}
	exit
}

case $DEVICENAME in
        qca-nss0* | qca-nss.0*)
                if [ -e /lib/firmware/qca-nss0-enterprise.bin ] ; then
                        $select_or_load /lib/firmware/qca-nss0-enterprise.bin
                else
                        $select_or_load /lib/firmware/qca-nss0-retail.bin
                fi
                exit
                ;;
        qca-nss1* | qca-nss.1*)
                if [ -e /lib/firmware/qca-nss1-enterprise.bin ] ; then
                        $select_or_load /lib/firmware/qca-nss1-enterprise.bin
                else
                        $select_or_load /lib/firmware/qca-nss1-retail.bin
                fi
                exit
                ;;
esac

