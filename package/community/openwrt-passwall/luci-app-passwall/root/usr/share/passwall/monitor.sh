#!/bin/sh

CONFIG=passwall
TMP_PATH=/var/etc/$CONFIG
TMP_BIN_PATH=$TMP_PATH/bin
TMP_ID_PATH=$TMP_PATH/id

config_n_get() {
	local ret=$(uci -q get $CONFIG.$1.$2 2>/dev/null)
	echo ${ret:=$3}
}

config_t_get() {
	local index=0
	[ -n "$4" ] && index=$4
	local ret=$(uci -q get $CONFIG.@$1[$index].$2 2>/dev/null)
	echo ${ret:=$3}
}

if [ "$(pgrep -f $CONFIG/monitor.sh | wc -l)" -gt 2 ]; then
	exit 1
fi

ENABLED=$(config_t_get global enabled 0)
[ "$ENABLED" != 1 ] && return 1
ENABLED=$(config_t_get global_delay start_daemon 0)
[ "$ENABLED" != 1 ] && return 1
sleep 58s
while [ "$ENABLED" -eq 1 ]
do
	#TCP
	[ -f "$TMP_ID_PATH/TCP" ] && {
		TCP_NODE=$(cat $TMP_ID_PATH/TCP)
		if [ "$TCP_NODE" != "nil" ]; then
			#kcptun
			use_kcp=$(config_n_get $TCP_NODE use_kcp 0)
			if [ $use_kcp -gt 0 ]; then
				icount=$(pgrep -af "$TMP_BIN_PATH/kcptun.*(tcp|TCP)" | grep -v -E 'acl/|acl_' | wc -l)
				if [ $icount = 0 ]; then
					/etc/init.d/$CONFIG restart
					exit 0
				fi
			fi
			icount=$(pgrep -af "$TMP_BIN_PATH.*(tcp|TCP)" | grep -v -E 'kcptun|acl/|acl_' | wc -l)
			if [ $icount = 0 ]; then
				/etc/init.d/$CONFIG restart
				exit 0
			fi
		fi
	}

	#udp
	[ -f "$TMP_ID_PATH/UDP" ] && {
		UDP_NODE=$(cat $TMP_ID_PATH/UDP)
		if [ "$UDP_NODE" != "nil" ]; then
			[ "$UDP_NODE" == "tcp" ] && UDP_NODE=$TCP_NODE
			icount=$(pgrep -af "$TMP_BIN_PATH.*(udp|UDP)" | grep -v -E 'acl/|acl_' | wc -l)
			if [ $icount = 0 ]; then
				/etc/init.d/$CONFIG restart
				exit 0
			fi
		fi
	}

	#dns
	dns_mode=$(config_t_get global dns_mode)
	if [ "$dns_mode" == "pdnsd" ] || [ "$dns_mode" == "dns2socks" ] || [ "$dns_mode" == "v2ray_doh" ] || [ "$dns_mode" == "xray_doh" ]; then
		icount=$(netstat -apn | grep 7913 | wc -l)
		if [ $icount = 0 ]; then
			/etc/init.d/$CONFIG restart
			exit 0
		fi
	fi
	
	[ -f "$TMP_BIN_PATH/chinadns-ng" ] && {
		if ! pgrep -x "$TMP_BIN_PATH/chinadns-ng" > /dev/null 2>&1; then
			/etc/init.d/$CONFIG restart
			exit 0
		fi
	}

	#haproxy
	use_haproxy=$(config_t_get global_haproxy balancing_enable 0)
	if [ $use_haproxy -gt 0 ]; then
		if ! pgrep -x "$TMP_BIN_PATH/haproxy" > /dev/null 2>&1; then
			/etc/init.d/$CONFIG restart
			exit 0
		fi
	fi
	
	sleep 58s
done
