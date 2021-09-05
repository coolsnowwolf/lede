#!/bin/sh

stretch() {
	#zhenduiluanshezhiDNSderen
	local dnsmasq_server=$(uci -q get dhcp.@dnsmasq[0].server)
	local dnsmasq_noresolv=$(uci -q get dhcp.@dnsmasq[0].noresolv)
	local _flag
	for server in $dnsmasq_server; do
		[ -z "$(echo $server | grep '\/')" ] && _flag=1
	done
	[ -z "$_flag" ] && [ "$dnsmasq_noresolv" = "1" ] && {
		uci -q delete dhcp.@dnsmasq[0].noresolv
		uci -q set dhcp.@dnsmasq[0].resolvfile="$RESOLVFILE"
		uci commit dhcp
	}
}

backup_servers() {
	DNSMASQ_DNS=$(uci show dhcp | grep "@dnsmasq" | grep ".server=" | awk -F '=' '{print $2}' | sed "s/'//g" | tr ' ' ',')
	if [ -n "${DNSMASQ_DNS}" ]; then
		uci -q set $CONFIG.@global[0].dnsmasq_servers="${DNSMASQ_DNS}"
		uci commit $CONFIG
	fi
}

restore_servers() {
	OLD_SERVER=$(uci -q get $CONFIG.@global[0].dnsmasq_servers | tr "," " ")
	for server in $OLD_SERVER; do
		uci -q del_list dhcp.@dnsmasq[0].server=$server
		uci -q add_list dhcp.@dnsmasq[0].server=$server
	done
	uci commit dhcp
	uci -q delete $CONFIG.@global[0].dnsmasq_servers
	uci commit $CONFIG
}

logic_restart() {
	local no_log
	eval_set_val $@
	_LOG_FILE=$LOG_FILE
	[ -n "$no_log" ] && LOG_FILE="/dev/null"
	if [ -f "$TMP_PATH/default_DNS" ]; then
		backup_servers
		#sed -i "/list server/d" /etc/config/dhcp >/dev/null 2>&1
		for server in $(uci -q get dhcp.@dnsmasq[0].server); do
			[ -n "$(echo $server | grep '\/')" ] || uci -q del_list dhcp.@dnsmasq[0].server="$server" 
		done
		/etc/init.d/dnsmasq restart >/dev/null 2>&1
		restore_servers
	else
		/etc/init.d/dnsmasq restart >/dev/null 2>&1
	fi
	echolog "重启 dnsmasq 服务"
	LOG_FILE=${_LOG_FILE}
}

restart() {
	local no_log
	eval_set_val $@
	_LOG_FILE=$LOG_FILE
	[ -n "$no_log" ] && LOG_FILE="/dev/null"
	/etc/init.d/dnsmasq restart >/dev/null 2>&1
	echolog "重启 dnsmasq 服务"
	LOG_FILE=${_LOG_FILE}
}

gen_dnsmasq_items() {
	local ipsetlist=${1}; shift 1
	local fwd_dns=${1}; shift 1
	local outf=${1}; shift 1

	awk -v ipsetlist="${ipsetlist}" -v ipsetoutf="${TMP_DNSMASQ_PATH}/ipset.conf" -v fwd_dns="${fwd_dns}" -v outf="${outf}" '
		BEGIN {
			if(outf == "") {outf="/dev/stdout"; ipsetoutf="/dev/stdout";}
			split(fwd_dns, dns, ","); setdns=length(dns)>0; setlist=length(ipsetlist)>0;
			if(setdns) for(i in dns) if(length(dns[i])==0) delete dns[i];
			fail=1;
		}
		! /^$/&&!/^#/ {
			fail=0
			if(! (setdns || setlist)) {printf("server=%s\n", $0) >>outf; next;}
			if(setdns) for(i in dns) printf("server=/.%s/%s\n", $0, dns[i]) >>outf;
			
			if(setlist) printf("ipset=/.%s/%s\n", $0, ipsetlist) >>ipsetoutf;
		}
		END {fflush(outf); close(outf); fflush(ipsetoutf); close(ipsetoutf); exit(fail);}
	'
}

gen_dnsmasq_address_items() {
	local fwd_dns=${1}; shift 1
	local outf=${1}; shift 1

	awk -v fwd_dns="${fwd_dns}" -v outf="${outf}" '
		BEGIN {
			if(outf == "") outf="/dev/stdout";
			split(fwd_dns, dns, ","); setdns=length(dns)>0;
			if(setdns) for(i in dns) if(length(dns[i])==0) delete dns[i];
			fail=1;
		}
		! /^$/&&!/^#/ {
			fail=0
			if(! setdns) {printf("address=%s\n", $0) >>outf; next;}
			if(setdns) for(i in dns) printf("address=/.%s/%s\n", $0, dns[i]) >>outf;
		}
		END {fflush(outf); close(outf); exit(fail);}
	'
}

add() {
	local fwd_dns item servers msg
	local DNS_MODE TMP_DNSMASQ_PATH DNSMASQ_CONF_FILE DEFAULT_DNS LOCAL_DNS TUN_DNS CHINADNS_DNS TCP_NODE PROXY_MODE NO_LOGIC_LOG
	eval_set_val $@
	_LOG_FILE=$LOG_FILE
	[ -n "$NO_LOGIC_LOG" ] && LOG_FILE="/dev/null"
	global=$(echo "${PROXY_MODE}" | grep "global")
	returnhome=$(echo "${PROXY_MODE}" | grep "returnhome")
	chnlist=$(echo "${PROXY_MODE}" | grep "chnroute")
	gfwlist=$(echo "${PROXY_MODE}" | grep "gfwlist")
	mkdir -p "${TMP_DNSMASQ_PATH}" "${DNSMASQ_PATH}" "/var/dnsmasq.d"

	if [ "${DNS_MODE}" = "nonuse" ]; then
		echolog "  - 不对域名进行分流解析"
		LOG_FILE=${_LOG_FILE}
		return 0
	else
		#屏蔽列表
		sort -u "${RULES_PATH}/block_host" | gen_dnsmasq_address_items "0.0.0.0" "${TMP_DNSMASQ_PATH}/00-block_host.conf"

		#始终用国内DNS解析节点域名
		fwd_dns="${LOCAL_DNS}"
		servers=$(uci show "${CONFIG}" | grep ".address=" | cut -d "'" -f 2)
		hosts_foreach "servers" host_from_url | grep -v "google.c" | grep '[a-zA-Z]$' | sort -u | gen_dnsmasq_items "vpsiplist,vpsiplist6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/10-vpsiplist_host.conf"
		echolog "  - [$?]节点列表中的域名(vpsiplist)：${fwd_dns:-默认}"

		#始终用国内DNS解析直连（白名单）列表
		[ -s "${RULES_PATH}/direct_host" ] && {
			fwd_dns="${LOCAL_DNS}"
			#[ -n "$CHINADNS_DNS" ] && unset fwd_dns
			sort -u "${RULES_PATH}/direct_host" | gen_dnsmasq_items "whitelist,whitelist6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/11-direct_host.conf"
			echolog "  - [$?]域名白名单(whitelist)：${fwd_dns:-默认}"
		}
		
		if [ "$(config_t_get global_subscribe subscribe_proxy 0)" = "0" ]; then
			#如果没有开启通过代理订阅
			fwd_dns="${LOCAL_DNS}"
			for item in $(get_enabled_anonymous_secs "@subscribe_list"); do
				host_from_url "$(config_n_get ${item} url)" | gen_dnsmasq_items "whitelist,whitelist6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/12-subscribe.conf"
			done
			echolog "  - [$?]节点订阅域名(whitelist)：${fwd_dns:-默认}"
		else
			#如果开启了通过代理订阅
			fwd_dns="${TUN_DNS}"
			#[ -n "$CHINADNS_DNS" ] && unset fwd_dns
			for item in $(get_enabled_anonymous_secs "@subscribe_list"); do
				host_from_url "$(config_n_get ${item} url)" | gen_dnsmasq_items "blacklist,blacklist6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/91-subscribe.conf"
			done
			echolog "  - [$?]节点订阅域名(blacklist)：${fwd_dns:-默认}"
		fi
		
		#始终使用远程DNS解析代理（黑名单）列表
		[ -s "${RULES_PATH}/proxy_host" ] && {
			fwd_dns="${TUN_DNS}"
			#[ -n "$CHINADNS_DNS" ] && unset fwd_dns
			sort -u "${RULES_PATH}/proxy_host" | gen_dnsmasq_items "blacklist,blacklist6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/97-proxy_host.conf"
			echolog "  - [$?]代理域名表(blacklist)：${fwd_dns:-默认}"
		}

		#分流规则
		[ "$(config_n_get $TCP_NODE protocol)" = "_shunt" ] && {
			fwd_dns="${TUN_DNS}"
			local default_node_id=$(config_n_get $TCP_NODE default_node _direct)
			local shunt_ids=$(uci show $CONFIG | grep "=shunt_rules" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')
			for shunt_id in $shunt_ids; do
				local shunt_node_id=$(config_n_get $TCP_NODE ${shunt_id} nil)
				if [ "$shunt_node_id" = "nil" ] || [ "$shunt_node_id" = "_default" ] || [ "$shunt_node_id" = "_direct" ] || [ "$shunt_node_id" = "_blackhole" ]; then
					continue
				fi
				local shunt_node=$(config_n_get $shunt_node_id address nil)
				[ "$shunt_node" = "nil" ] && continue
				config_n_get $shunt_id domain_list | grep -v 'regexp:\|geosite:\|ext:' | sed 's/domain:\|full:\|//g' | tr -s "\r\n" "\n" | sort -u | gen_dnsmasq_items "shuntlist,shuntlist6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/98-shunt_host.conf"
			done
			echolog "  - [$?]V2ray/Xray分流规则(shuntlist)：${fwd_dns:-默认}"
		}
		
		count_hosts_str=
		[ -s "${RULES_PATH}/direct_host" ] && direct_hosts_str="$(echo -n $(cat ${RULES_PATH}/direct_host) | sed "s/ /|/g")"
		[ -s "${RULES_PATH}/proxy_host" ] && proxy_hosts_str="$(echo -n $(cat ${RULES_PATH}/proxy_host) | sed "s/ /|/g")"
		[ -n "$direct_hosts_str" ] && {
			tmp="${direct_hosts_str}"
			[ -n "$count_hosts_str" ] && tmp="${count_hosts_str}|${direct_hosts_str}"
			count_hosts_str="$tmp"
		}
		[ -n "$proxy_hosts_str" ] && {
			tmp="${proxy_hosts_str}"
			[ -n "$count_hosts_str" ] && tmp="${count_hosts_str}|${proxy_hosts_str}"
			count_hosts_str="$tmp"
		}

		#如果没有使用回国模式
		if [ -z "${returnhome}" ]; then
			# GFW 模式
			[ -s "${RULES_PATH}/gfwlist" ] && {
				if [ -n "$count_hosts_str" ]; then
					grep -v -E "$count_hosts_str" "${RULES_PATH}/gfwlist" > "${TMP_PATH}/gfwlist"
				else
					cp -a "${RULES_PATH}/gfwlist" "${TMP_PATH}/gfwlist"
				fi
			}
			fwd_dns="${TUN_DNS}"
			[ -n "$CHINADNS_DNS" ] && unset fwd_dns
			sort -u "${TMP_PATH}/gfwlist" | gen_dnsmasq_items "gfwlist,gfwlist6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/99-gfwlist.conf"
			echolog "  - [$?]防火墙域名表(gfwlist)：${fwd_dns:-默认}"
			rm -f "${TMP_PATH}/gfwlist"
			
			# 中国列表以外 模式
			[ -n "${CHINADNS_DNS}" ] && {
				fwd_dns="${LOCAL_DNS}"
				[ -n "$CHINADNS_DNS" ] && unset fwd_dns
				[ -s "${RULES_PATH}/chnlist" ] && {
					if [ -n "$count_hosts_str" ]; then
						grep -v -E "$count_hosts_str" "${RULES_PATH}/chnlist" | gen_dnsmasq_items "chnroute,chnroute6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/19-chinalist_host.conf"
					else
						sort -u "${RULES_PATH}/chnlist" | gen_dnsmasq_items "chnroute,chnroute6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/19-chinalist_host.conf"
					fi
					echolog "  - [$?]中国域名表(chnroute)：${fwd_dns:-默认}"
				}
			}
		else
			#回国模式
			[ -s "${RULES_PATH}/chnlist" ] && {
				if [ -n "$count_hosts_str" ]; then
					grep -v -E "$count_hosts_str" "${RULES_PATH}/chnlist" > "${TMP_PATH}/chnlist"
				else
					cp -a "${RULES_PATH}/chnlist" "${TMP_PATH}/chnlist"
				fi
			}
			fwd_dns="${TUN_DNS}"
			[ -s "${TMP_PATH}/chnlist" ] && sort -u "${TMP_PATH}/chnlist" | gen_dnsmasq_items "chnroute,chnroute6" "${fwd_dns}" "${TMP_DNSMASQ_PATH}/99-chinalist_host.conf"
			echolog "  - [$?]中国域名表(chnroute)：${fwd_dns:-默认}"
			rm -f "${TMP_PATH}/chnlist"
		fi
		
		awk '{gsub(/ipset=\//,""); gsub(/\//," ");key=$1;value=$2;if (sum[key] != "") {sum[key]=sum[key]","value} else {sum[key]=sum[key]value}} END{for(i in sum) print "ipset=/"i"/"sum[i]}' "${TMP_DNSMASQ_PATH}/ipset.conf" > "${TMP_DNSMASQ_PATH}/ipset.conf2"
		mv -f "${TMP_DNSMASQ_PATH}/ipset.conf2" "${TMP_DNSMASQ_PATH}/ipset.conf"
	fi
	
	echo "conf-dir=${TMP_DNSMASQ_PATH}" > $DNSMASQ_CONF_FILE
	[ -n "${CHINADNS_DNS}" ] && {
		echo "${DEFAULT_DNS}" > $TMP_PATH/default_DNS
		cat <<-EOF >> $DNSMASQ_CONF_FILE
			$(echo "${CHINADNS_DNS}" | sed 's/,/\n/g' | gen_dnsmasq_items)
			all-servers
			no-poll
			no-resolv
		EOF
		echolog "  - [$?]以上所列以外及默认(ChinaDNS-NG)：${CHINADNS_DNS}"
	}
	LOG_FILE=${_LOG_FILE}
}

del() {
	rm -rf /var/dnsmasq.d/dnsmasq-$CONFIG.conf
	rm -rf $DNSMASQ_PATH/dnsmasq-$CONFIG.conf
	rm -rf $TMP_DNSMASQ_PATH
}

arg1=$1
shift
case $arg1 in
stretch)
	stretch $@
	;;
add)
	add $@
	;;
del)
	del $@
	;;
restart)
	restart $@
	;;
logic_restart)
	logic_restart $@
	;;
*) ;;
esac
