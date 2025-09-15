#!/bin/sh /etc/rc.common
#
# Copyright (c) 2018-2019, 2021 The Linux Foundation. All rights reserved.
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

NSS_IPSEC_LOG_FILE=/tmp/.nss_ipsec_log
NSS_IPSEC_LOG_STR_ECM="ECM_Loaded"
NSS_IPSEC_OL_FILE=/tmp/qca_nss_ipsec_ol

ecm_load () {
	if [ ! -d /sys/module/ecm ]; then
		/etc/init.d/qca-nss-ecm start
		if [ -d /sys/module/ecm ]; then
			echo ${NSS_IPSEC_LOG_STR_ECM} >> ${NSS_IPSEC_LOG_FILE}
		fi
	fi
}

ecm_unload () {
	if [ -f /tmp/.nss_ipsec_log ]; then
		str=`grep ${NSS_IPSEC_LOG_STR_ECM} ${NSS_IPSEC_LOG_FILE}`
		if [[ $str == ${NSS_IPSEC_LOG_STR_ECM} ]]; then
			/etc/init.d/qca-nss-ecm stop
			`sed 's/${NSS_IPSEC_LOG_STR_ECM}/ /g' $NSS_IPSEC_LOG_FILE >  $NSS_IPSEC_LOG_FILE`
		fi
	fi
}

ecm_disable() {
	if [ ! -d /sys/module/ecm ]; then
		return;
	fi

	echo 1 > /sys/kernel/debug/ecm/front_end_ipv4_stop
	echo 1 > /sys/kernel/debug/ecm/front_end_ipv6_stop
	echo 1 > /sys/kernel/debug/ecm/ecm_db/defunct_all
	sleep 2
}

ecm_enable() {
	if [ ! -d /sys/module/ecm ]; then
		return;
	fi

	echo 0 > /sys/kernel/debug/ecm/ecm_db/defunct_all
	echo 0 > /sys/kernel/debug/ecm/front_end_ipv4_stop
	echo 0 > /sys/kernel/debug/ecm/front_end_ipv6_stop
}

kernel_version_check_5_4() {
	major_ver=$(uname -r | awk -F '.' '{print $1}')
	minor_ver=$(uname -r | awk -F '.' '{print $2}')
	if [ $major_ver -lt 5 ] || ([ $major_ver -eq 5 ] && [ $minor_ver -lt 4 ] ); then
		return 1
	else
		return 0
	fi
}

kernel_version_check_5_15() {
	major_ver=$(uname -r | awk -F '.' '{print $1}')
	minor_ver=$(uname -r | awk -F '.' '{print $2}')
	if [ $major_ver -lt 5 ] || ([ $major_ver -eq 5 ] && [ $minor_ver -lt 15 ] ); then
		return 1
	else
		return 0
	fi
}

start_klips() {
	if kernel_version_check_5_4; then
		echo "Kernel 5.4 doesn't support klips stack."
		return $?
	fi

	if kernel_version_check_5_15; then
		echo "Kernel 5.15 doesn't support klips stack."
		return $?
	fi

	touch $NSS_IPSEC_OL_FILE
	ecm_load

	local kernel_version=$(uname -r)

	insmod /lib/modules/${kernel_version}/qca-nss-ipsec-klips.ko
	if [ "$?" -gt 0 ]; then
		echo "Failed to load plugin. Please start ecm if not done already"
		ecm_enable
		rm $NSS_IPSEC_OL_FILE
		return
	fi

	/etc/init.d/ipsec start
	sleep 2
	ipsec eroute

	ecm_enable
}

stop_klips() {
	if kernel_version_check_5_4; then
		echo "Kernel 5.4 doesn't support klips stack."
		return $?
	fi

	if kernel_version_check_5_15; then
		echo "Kernel 5.15 doesn't support klips stack."
		return $?
	fi

	ecm_disable

	/etc/init.d/ipsec stop
	rmmod qca-nss-ipsec-klips
	rm $NSS_IPSEC_OL_FILE

	ecm_unload
}

start_xfrm() {
	touch $NSS_IPSEC_OL_FILE
	ecm_load

	local kernel_version=$(uname -r)

	# load all NETKEY modules first.
	for mod in xfrm_ipcomp ipcomp xfrm6_tunnel ipcomp6 xfrm6_mode_tunnel xfrm6_mode_beet xfrm6_mode_ro \
		xfrm6_mode_transport xfrm4_mode_transport xfrm4_mode_tunnel \
		xfrm4_tunnel xfrm4_mode_beet esp4 esp6 ah4 ah6 af_key
		do
			insmod $mod 2> /dev/null
		done

	# Now load the xfrm plugin
	insmod /lib/modules/${kernel_version}/qca-nss-ipsec-xfrm.ko
	if [ "$?" -gt 0 ]; then
		echo "Failed to load plugin. Please start ecm if not done already"
		ecm_enable
		rm $NSS_IPSEC_OL_FILE
		return
	fi

	/etc/init.d/ipsec start
	sleep 2

	ecm_enable
}

stop_xfrm() {
	ecm_disable

	#Shutdown Pluto first. Then only plugin can be removed.
	plutopid=/var/run/pluto/pluto.pid
	if [ -f $plutopid ]; then
		pid=`cat $plutopid`
		if [ ! -z "$pid" ]; then
			ipsec whack --shutdown | grep -v "002";
			if [ -s $plutopid ]; then
				echo "Attempt to shut Pluto down failed!  Trying kill:"
				kill $pid;
				sleep 5;
			fi
		fi
		rm -rf $plutopid
	fi
	ip xfrm state flush;
	ip xfrm policy flush;
	sleep 2

	#Now we can remove the plugin
	retries=5
	while [ -d /sys/module/qca_nss_ipsec_xfrm ]
	do
		rmmod qca-nss-ipsec-xfrm
		if [ "$?" -eq 0 ]; then
			rm $NSS_IPSEC_OL_FILE
			break
		fi

		if [ ${retries} -eq 0 ]; then
			echo "Failed to unload qca-nss-ipsec-xfrm plugin!"
			exit
		fi

		echo "XFRM plugin unload failed; retrying ${retries} times"
		sleep 1
		retries=`expr ${retries} - 1`
	done

	/etc/init.d/ipsec stop
	ecm_unload
}

start() {
	local protostack=`uci -q get ipsec.setup.protostack`
	if [ "$protostack" = "klips" ]; then
		start_klips
		return $?
	fi

	start_xfrm
	return $?
}

stop() {
	local protostack=`uci -q get ipsec.setup.protostack`
	if [ "$protostack" = "klips" ]; then
		stop_klips
		return $?
	fi

	stop_xfrm
	return $?
}

restart() {
	stop
	start
}
