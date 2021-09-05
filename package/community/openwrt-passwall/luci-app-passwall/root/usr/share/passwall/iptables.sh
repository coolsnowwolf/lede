#!/bin/sh

IPSET_LANIPLIST="laniplist"
IPSET_VPSIPLIST="vpsiplist"
IPSET_SHUNTLIST="shuntlist"
IPSET_GFW="gfwlist"
IPSET_CHN="chnroute"
IPSET_BLACKLIST="blacklist"
IPSET_WHITELIST="whitelist"
IPSET_BLOCKLIST="blocklist"

IPSET_LANIPLIST6="laniplist6"
IPSET_VPSIPLIST6="vpsiplist6"
IPSET_SHUNTLIST6="shuntlist6"
IPSET_GFW6="gfwlist6"
IPSET_CHN6="chnroute6"
IPSET_BLACKLIST6="blacklist6"
IPSET_WHITELIST6="whitelist6"
IPSET_BLOCKLIST6="blocklist6"

FORCE_INDEX=2

ipt_n="iptables -t nat -w"
ipt_m="iptables -t mangle -w"
ip6t_m="ip6tables -t mangle -w"
FWI=$(uci -q get firewall.passwall.path 2>/dev/null)
FAKE_IP=198.18.0.0/16

factor() {
	if [ -z "$1" ] || [ -z "$2" ]; then
		echo ""
	elif [ "$1" == "1:65535" ]; then
		echo ""
	else
		echo "$2 $1"
	fi
}

dst() {
	echo "-m set $2 --match-set $1 dst"
}

comment() {
	local name=$(echo $1 | sed 's/ /_/g')
	echo "-m comment --comment '$name'"
}

destroy_ipset() {
	for i in "$@"; do
		ipset -q -F $i
		ipset -q -X $i
	done
}

RULE_LAST_INDEX() {
	[ $# -ge 3 ] || {
		echolog "索引列举方式不正确（iptables），终止执行！"
		exit 1
	}
	local ipt_tmp=${1}; shift
	local chain=${1}; shift
	local list=${1}; shift
	local default=${1:-0}; shift
	local _index=$($ipt_tmp -n -L $chain --line-numbers 2>/dev/null | grep "$list" | head -n 1 | awk '{print $1}')
	echo "${_index:-${default}}"
}

REDIRECT() {
	local redirect="-j REDIRECT --to-ports $1"
	[ "$2" == "TPROXY" ] && redirect="-j TPROXY --tproxy-mark 0x1/0x1 --on-port $1"
	[ "$2" == "MARK" ] && redirect="-j MARK --set-mark $1"
	echo $redirect
}

get_redirect_ipt() {
	case "$1" in
	disable)
		echo "-j RETURN"
		;;
	global)
		echo "$(REDIRECT $2 $3)"
		;;
	gfwlist)
		echo "$(dst $IPSET_GFW) $(REDIRECT $2 $3)"
		;;
	chnroute)
		echo "$(dst $IPSET_CHN !) $(REDIRECT $2 $3)"
		;;
	returnhome)
		echo "$(dst $IPSET_CHN) $(REDIRECT $2 $3)"
		;;
	direct/proxy)
		echo "-j RETURN"
		;;
	esac
}

get_redirect_ip6t() {
	case "$1" in
	disable)
		echo "-j RETURN"
		;;
	global)
		echo "$(REDIRECT $2 $3)"
		;;
	gfwlist)
		echo "$(dst $IPSET_GFW6) $(REDIRECT $2 $3)"
		;;
	chnroute)
		echo "$(dst $IPSET_CHN6 !) $(REDIRECT $2 $3)"
		;;
	returnhome)
		echo "$(dst $IPSET_CHN6) $(REDIRECT $2 $3)"
		;;
	direct/proxy)
		echo "-j RETURN"
		;;
	esac
}

get_action_chain_name() {
	case "$1" in
	disable)
		echo "不代理"
		;;
	global)
		echo "全局代理"
		;;
	gfwlist)
		echo "防火墙列表"
		;;
	chnroute)
		echo "中国列表以外"
		;;
	returnhome)
		echo "中国列表"
		;;
	direct/proxy)
		echo "仅使用直连/代理列表"
		;;
	esac
}

gen_laniplist() {
	cat <<-EOF
		0.0.0.0/8
		10.0.0.0/8
		100.64.0.0/10
		127.0.0.0/8
		169.254.0.0/16
		172.16.0.0/12
		192.168.0.0/16
		224.0.0.0/4
		240.0.0.0/4
	EOF
}

gen_laniplist_6() {
	cat <<-EOF
		::/128
		::1/128
		::ffff:0:0/96
		::ffff:0:0:0/96
		64:ff9b::/96
		100::/64
		2001::/32
		2001:20::/28
		2001:db8::/32
		2002::/16
		fc00::/7
		fe80::/10
		ff00::/8
	EOF
}

load_acl() {
	local items=$(get_enabled_anonymous_secs "@acl_rule")
	[ -n "$items" ] && {
		local item
		local socks_port redir_port dns_port dnsmasq_port
		local ipt_tmp msg msg2
		socks_port=11100
		redir_port=11200
		dns_port=11300
		dnsmasq_port=11400
		echolog "访问控制："
		for item in $items; do
			local enabled sid remarks ip_mac tcp_proxy_mode udp_proxy_mode tcp_no_redir_ports udp_no_redir_ports tcp_redir_ports udp_redir_ports tcp_node udp_node dns_mode dns_forward dns_doh
			local ip mac ip_or_mac ip_mac_list tcp_port udp_port tcp_node_remark udp_node_remark config_file
			sid=$(uci -q show "${CONFIG}.${item}" | grep "=acl_rule" | awk -F '=' '{print $1}' | awk -F '.' '{print $2}')
			eval $(uci -q show "${CONFIG}.${item}" | cut -d'.' -sf 3-)
			[ -z "${ip_mac}" ] && continue
			
			for i in ${ip_mac}; do
				ip_or_mac=$(lua_api "ip_or_mac(\"${i}\")")
				if [ "${ip_or_mac}" = "ip" ] || [ "${ip_or_mac}" = "mac" ]; then
					ip_mac_list="$ip_mac_list $i"
				fi
			done
			[ -z "${ip_mac_list}" ] && continue
			tcp_proxy_mode=${tcp_proxy_mode:-default}
			udp_proxy_mode=${udp_proxy_mode:-default}
			tcp_no_redir_ports=${tcp_no_redir_ports:-default}
			udp_no_redir_ports=${udp_no_redir_ports:-default}
			tcp_redir_ports=${tcp_redir_ports:-default}
			udp_redir_ports=${udp_redir_ports:-default}
			tcp_node=${tcp_node:-default}
			udp_node=${udp_node:-default}
			dns_mode=${dns_mode:-dns2socks}
			dns_forward=${dns_forward:-8.8.8.8}
			([ "$dns_mode" = "v2ray_doh" ] || [ "$dns_mode" = "xray_doh" ]) && dns_forward=${dns_doh:-https://dns.google/dns-query,8.8.8.8}
			[ "$tcp_proxy_mode" = "default" ] && tcp_proxy_mode=$TCP_PROXY_MODE
			[ "$udp_proxy_mode" = "default" ] && udp_proxy_mode=$UDP_PROXY_MODE
			[ "$tcp_no_redir_ports" = "default" ] && tcp_no_redir_ports=$TCP_NO_REDIR_PORTS
			[ "$udp_no_redir_ports" = "default" ] && udp_no_redir_ports=$UDP_NO_REDIR_PORTS
			[ "$tcp_redir_ports" = "default" ] && tcp_redir_ports=$TCP_REDIR_PORTS
			[ "$udp_redir_ports" = "default" ] && udp_redir_ports=$UDP_REDIR_PORTS
			[ "$tcp_node" != "nil" ] && {
				if [ "$tcp_node" = "default" ]; then
					tcp_node=$TCP_NODE
					tcp_port=$TCP_REDIR_PORT
				else
					[ "$(config_get_type $tcp_node nil)" = "nodes" ] && {
						run_dns() {
							local _dns_port
							[ -n $1 ] && _dns_port=$1
							[ -z ${_dns_port} ] && {
								dns_port=$(get_new_port $(expr $dns_port + 1))
								_dns_port=$dns_port
								if [ "$dns_mode" = "dns2socks" ]; then
									run_dns2socks flag=acl_${sid} socks_address=127.0.0.1 socks_port=$socks_port listen_address=0.0.0.0 listen_port=${_dns_port} dns=$dns_forward cache=1
								elif ([ "$dns_mode" = "v2ray_doh" ] || [ "$dns_mode" = "xray_doh" ]); then
									_doh_url=$(echo $dns_forward | awk -F ',' '{print $1}')
									_doh_host_port=$(echo $_doh_url | sed "s/https:\/\///g" | awk -F '/' '{print $1}')
									_doh_host=$(echo $_doh_host_port | awk -F ':' '{print $1}')
									_doh_port=$(echo $_doh_host_port | awk -F ':' '{print $2}')
									_doh_bootstrap=$(echo $dns_forward | cut -d ',' -sf 2-)
									config_file=$TMP_ACL_PATH/${tcp_node}_SOCKS_${socks_port}_DNS.json
									run_v2ray_doh_socks flag=acl_${sid} socks_address=127.0.0.1 socks_port=$socks_port listen_address=0.0.0.0 listen_port=${_dns_port} doh_bootstrap=${_doh_bootstrap} doh_url=${_doh_url} doh_host=${_doh_host} config_file=$config_file
								fi
								eval node_${tcp_node}_$(echo -n "${dns_forward}" | md5sum | cut -d " " -f1)=${_dns_port}
							}
							
							dnsmasq_port=$(get_new_port $(expr $dnsmasq_port + 1))
							redirect_dns_port=$dnsmasq_port
							mkdir -p $TMP_ACL_PATH/$sid
							echo "port=${dnsmasq_port}" >> $TMP_ACL_PATH/$sid/dnsmasq.conf
							echo "conf-dir=${TMP_ACL_PATH}/${sid}/dnsmasq.d" >> $TMP_ACL_PATH/$sid/dnsmasq.conf
							d_server=127.0.0.1
							[ "$tcp_proxy_mode" = "global" ] && d_server=${d_server}#${_dns_port}
							echo "server=${d_server}" >> $TMP_ACL_PATH/$sid/dnsmasq.conf
							source $APP_PATH/helper_${DNS_N}.sh add DNS_MODE=$dns_mode TMP_DNSMASQ_PATH=$TMP_ACL_PATH/$sid/dnsmasq.d DNSMASQ_CONF_FILE=/dev/null LOCAL_DNS=$LOCAL_DNS TUN_DNS=127.0.0.1#${_dns_port} TCP_NODE=$tcp_node PROXY_MODE=${tcp_proxy_mode} NO_LOGIC_LOG=1
							ln_start_bin "$(first_type dnsmasq)" "dnsmasq_${sid}" "/dev/null" -C $TMP_ACL_PATH/$sid/dnsmasq.conf -x $TMP_ACL_PATH/$sid/dnsmasq.pid
							eval node_${tcp_node}_$(echo -n "${tcp_proxy_mode}${dns_forward}" | md5sum | cut -d " " -f1)=${dnsmasq_port}
						}
						if [ "$tcp_node" = "$TCP_NODE" ]; then
							tcp_port=$TCP_REDIR_PORT
						else
							_redir_port=$(eval echo \${node_${tcp_node}_redir_port})
							_socks_port=$(eval echo \${node_${tcp_node}_socks_port})
							if [ -n "${_socks_port}" ] && [ -n "${_redir_port}" ]; then
								socks_port=${_socks_port}
								tcp_port=${_redir_port}
								_dnsmasq_port=$(eval echo \${node_${tcp_node}_$(echo -n "${tcp_proxy_mode}${dns_forward}" | md5sum | cut -d " " -f1)})
								if [ -z "${_dnsmasq_port}" ]; then
									_dns_port=$(eval echo \${node_${tcp_node}_$(echo -n "${dns_forward}" | md5sum | cut -d " " -f1)})
									run_dns ${_dns_port}
								else
									redirect_dns_port=${_dnsmasq_port}
								fi
							else
								socks_port=$(get_new_port $(expr $socks_port + 1))
								eval node_${tcp_node}_socks_port=$socks_port
								redir_port=$(get_new_port $(expr $redir_port + 1))
								eval node_${tcp_node}_redir_port=$redir_port
								tcp_port=$redir_port
								config_file=$TMP_ACL_PATH/${tcp_node}_SOCKS_${socks_port}.json
								
								local type=$(echo $(config_n_get $tcp_node type) | tr 'A-Z' 'a-z')
								if [ -n "${type}" ] && ([ "${type}" = "v2ray" ] || [ "${type}" = "xray" ]); then
									config_file=$(echo $config_file | sed "s/SOCKS/TCP_UDP_SOCKS/g")
									run_v2ray flag=$tcp_node node=$tcp_node redir_port=$redir_port socks_address=127.0.0.1 socks_port=$socks_port config_file=$config_file
								else
									run_socks flag=$tcp_node node=$tcp_node bind=127.0.0.1 socks_port=$socks_port config_file=$config_file
									local log_file=$TMP_ACL_PATH/ipt2socks_${tcp_node}_${redir_port}.log
									log_file="/dev/null"
									run_ipt2socks flag=acl_${tcp_node} tcp_tproxy=${is_tproxy} local_port=$redir_port socks_address=127.0.0.1 socks_port=$socks_port log_file=$log_file
								fi
								run_dns
							fi
							filter_node $tcp_node TCP > /dev/null 2>&1 &
							filter_node $tcp_node UDP > /dev/null 2>&1 &
						fi
					}
				fi
				tcp_node_remark=$(config_n_get $tcp_node remarks)
			}
			[ "$udp_node" != "nil" ] && {
				[ "$udp_node" = "tcp" ] && udp_node=$tcp_node
				if [ "$udp_node" = "default" ]; then
					udp_node=$UDP_NODE
					[ "$TCP_UDP" = "1" ] && [ "$udp_node" = "nil" ] && udp_node=$TCP_NODE
					udp_port=$UDP_REDIR_PORT
				else
					[ "$(config_get_type $udp_node nil)" = "nodes" ] && {
						if [ "$udp_node" = "$UDP_NODE" ]; then
							udp_port=$UDP_REDIR_PORT
						else
							_redir_port=$(eval echo \${node_${udp_node}_redir_port})
							_socks_port=$(eval echo \${node_${udp_node}_socks_port})
							if [ -n "${_socks_port}" ] && [ -n "${_redir_port}" ]; then
								socks_port=${_socks_port}
								udp_port=${_redir_port}
							else
								socks_port=$(get_new_port $(expr $socks_port + 1))
								eval node_${udp_node}_socks_port=$socks_port
								redir_port=$(get_new_port $(expr $redir_port + 1))
								eval node_${udp_node}_redir_port=$redir_port
								udp_port=$redir_port
								config_file=$TMP_ACL_PATH/${udp_node}_SOCKS_${socks_port}.json
								
								local type=$(echo $(config_n_get $udp_node type) | tr 'A-Z' 'a-z')
								if [ -n "${type}" ] && ([ "${type}" = "v2ray" ] || [ "${type}" = "xray" ]); then
									config_file=$(echo $config_file | sed "s/SOCKS/TCP_UDP_SOCKS/g")
									run_v2ray flag=$udp_node node=$udp_node redir_port=$redir_port socks_address=127.0.0.1 socks_port=$socks_port config_file=$config_file
								else
									run_socks flag=$udp_node node=$udp_node bind=127.0.0.1 socks_port=$socks_port config_file=$config_file
									local log_file=$TMP_ACL_PATH/ipt2socks_${udp_node}_${redir_port}.log
									log_file="/dev/null"
									run_ipt2socks flag=acl_${udp_node} local_port=$redir_port socks_address=127.0.0.1 socks_port=$socks_port log_file=$log_file
								fi
							fi
							filter_node $udp_node TCP > /dev/null 2>&1 &
							filter_node $udp_node UDP > /dev/null 2>&1 &
						fi
					}
				fi
				udp_node_remark=$(config_n_get $udp_node remarks)
			}
			
			for i in ${ip_mac_list}; do
				ip_or_mac=$(lua_api "ip_or_mac(\"${i}\")")
				if [ "${ip_or_mac}" = "ip" ]; then
					ip=${i}
					unset mac
				elif [ "${ip_or_mac}" = "mac" ]; then
					mac=${i}
					unset ip
				fi
				[ -z "${ip}${mac}" ] && continue
				
				[ -n "$ip" ] && msg="备注：$remarks，IP：$ip，"
				[ -n "$mac" ] && msg="备注：$remarks，MAC：$mac，"
				ipt_tmp=$ipt_n
				[ -n "${is_tproxy}" ] && ipt_tmp=$ipt_m
				
				[ -n "$tcp_port" ] && {
					if [ "$tcp_proxy_mode" != "disable" ]; then
						[ -n "$redirect_dns_port" ] && $ipt_n -A PSW_REDIRECT $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") --dport 53 -j REDIRECT --to-ports $redirect_dns_port
						msg2="${msg}使用TCP节点[$tcp_node_remark] [$(get_action_chain_name $tcp_proxy_mode)]"
						if [ -n "${is_tproxy}" ]; then
							msg2="${msg2}(TPROXY:${tcp_port})代理"
							ipt_tmp=$ipt_m
						else
							msg2="${msg2}(REDIRECT:${tcp_port})代理"
						fi
						[ "$tcp_no_redir_ports" != "disable" ] && {
							$ipt_tmp -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -m multiport --dport $tcp_no_redir_ports -j RETURN
							$ip6t_m -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -m multiport --dport $tcp_no_redir_ports -j RETURN 2>/dev/null
							msg2="${msg2}[$?]除${tcp_no_redir_ports}外的"
						}
						msg2="${msg2}所有端口"
						$ipt_tmp -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -d $FAKE_IP $(REDIRECT $tcp_port $is_tproxy)
						$ipt_tmp -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(dst $IPSET_SHUNTLIST) $(REDIRECT $tcp_port $is_tproxy)
						$ipt_tmp -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $tcp_port $is_tproxy)
						$ipt_tmp -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(get_redirect_ipt $tcp_proxy_mode $tcp_port $is_tproxy)
						
						[ "$accept_icmp" = "1" ] && {
							$ipt_n -A PSW $(comment "$remarks") -p icmp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -d $FAKE_IP $(REDIRECT $tcp_port)
							$ipt_n -A PSW $(comment "$remarks") -p icmp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(dst $IPSET_SHUNTLIST) $(REDIRECT $tcp_port)
							$ipt_n -A PSW $(comment "$remarks") -p icmp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(dst $IPSET_BLACKLIST) $(REDIRECT $tcp_port)
							$ipt_n -A PSW $(comment "$remarks") -p icmp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(get_redirect_ipt $tcp_proxy_mode $tcp_port)
						}

						if [ "$PROXY_IPV6" == "1" ]; then
							$ip6t_m -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(dst $IPSET_SHUNTLIST6) $(REDIRECT $tcp_port TPROXY) 2>/dev/null
							$ip6t_m -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(dst $IPSET_BLACKLIST6) $(REDIRECT $tcp_port TPROXY) 2>/dev/null
							$ip6t_m -A PSW $(comment "$remarks") -p tcp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $tcp_redir_ports "-m multiport --dport") $(get_redirect_ip6t $tcp_proxy_mode $tcp_port TPROXY) 2>/dev/null
						fi
					else
						msg2="${msg}不代理TCP"
					fi
					echolog "  - ${msg2}"
				}
				
				$ipt_tmp -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -j RETURN
				$ip6t_m -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -j RETURN 2>/dev/null
				
				[ -n "$udp_port" ] && {
					if [ "$udp_proxy_mode" != "disable" ]; then
						msg2="${msg}使用UDP节点[$udp_node_remark] [$(get_action_chain_name $udp_proxy_mode)]"
						msg2="${msg2}(TPROXY:${udp_port})代理"
						[ "$udp_no_redir_ports" != "disable" ] && {
							$ipt_m -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -m multiport --dport $udp_no_redir_ports -j RETURN
							$ip6t_m -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -m multiport --dport $udp_no_redir_ports -j RETURN 2>/dev/null
							msg2="${msg2}[$?]除${udp_no_redir_ports}外的"
						}
						msg2="${msg2}所有端口"
						$ipt_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -d $FAKE_IP $(REDIRECT $udp_port TPROXY)
						$ipt_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $udp_redir_ports "-m multiport --dport") $(dst $IPSET_SHUNTLIST) $(REDIRECT $udp_port TPROXY)
						$ipt_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $udp_redir_ports "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $udp_port TPROXY)
						$ipt_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $udp_redir_ports "-m multiport --dport") $(get_redirect_ipt $udp_proxy_mode $udp_port TPROXY)

						if [ "$PROXY_IPV6" == "1" ]; then
							$ip6t_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $udp_redir_ports "-m multiport --dport") $(dst $IPSET_SHUNTLIST6) $(REDIRECT $udp_port TPROXY) 2>/dev/null
							$ip6t_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $udp_redir_ports "-m multiport --dport") $(dst $IPSET_BLACKLIST6) $(REDIRECT $udp_port TPROXY) 2>/dev/null
							$ip6t_m -A PSW $(comment "$remarks") -p udp $(factor $ip "-s") $(factor $mac "-m mac --mac-source") $(factor $udp_redir_ports "-m multiport --dport") $(get_redirect_ip6t $udp_proxy_mode $udp_port TPROXY) 2>/dev/null
						fi
					else
						msg2="${msg}不代理UDP"
					fi
					echolog "  - ${msg2}"
				}
				$ipt_m -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -j RETURN
				$ip6t_m -A PSW $(comment "$remarks") $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -j RETURN 2>/dev/null
			done
			unset enabled sid remarks ip_mac tcp_proxy_mode udp_proxy_mode tcp_no_redir_ports udp_no_redir_ports tcp_redir_ports udp_redir_ports tcp_node udp_node dns_mode dns_forward dns_doh
			unset ip mac ip_or_mac ip_mac_list tcp_port udp_port tcp_node_remark udp_node_remark config_file
			unset ipt_tmp msg msg2
			unset redirect_dns_port
		done
		unset socks_port redir_port dns_port dnsmasq_port
		unset ipt_tmp msg msg2
	}

	#  加载TCP默认代理模式
	local ipt_tmp=$ipt_n
	if [ "$TCP_PROXY_MODE" != "disable" ]; then
		[ "$TCP_NO_REDIR_PORTS" != "disable" ] && {
			$ipt_tmp -A PSW $(comment "默认") -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
			$ip6t_m -A PSW $(comment "默认") -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
		}

		ipt_tmp=$ipt_n
		[ "$TCP_NODE" != "nil" ] && {
			msg="TCP默认代理：使用TCP节点[$(config_n_get $TCP_NODE remarks)] [$(get_action_chain_name $TCP_PROXY_MODE)]"
			if [ -n "${is_tproxy}" ]; then
				ipt_tmp=$ipt_m
				msg="${msg}(TPROXY:${TCP_REDIR_PORT})代理"
			else
				msg="${msg}(REDIRECT:${TCP_REDIR_PORT})代理"
			fi
			[ "$TCP_NO_REDIR_PORTS" != "disable" ] && msg="${msg}除${TCP_NO_REDIR_PORTS}外的"
			msg="${msg}所有端口"
			$ipt_tmp -A PSW $(comment "默认") -p tcp -d $FAKE_IP $(REDIRECT $TCP_REDIR_PORT $is_tproxy)
			$ipt_tmp -A PSW $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST) $(REDIRECT $TCP_REDIR_PORT $is_tproxy)
			$ipt_tmp -A PSW $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $TCP_REDIR_PORT $is_tproxy)
			$ipt_tmp -A PSW $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $TCP_PROXY_MODE $TCP_REDIR_PORT $is_tproxy)
			
			[ "$accept_icmp" = "1" ] && {
				$ipt_n -A PSW $(comment "默认") -p icmp -d $FAKE_IP $(REDIRECT $TCP_REDIR_PORT)
				$ipt_n -A PSW $(comment "默认") -p icmp $(dst $IPSET_SHUNTLIST) $(REDIRECT $TCP_REDIR_PORT)
				$ipt_n -A PSW $(comment "默认") -p icmp $(dst $IPSET_BLACKLIST) $(REDIRECT $TCP_REDIR_PORT)
				$ipt_n -A PSW $(comment "默认") -p icmp $(get_redirect_ipt $TCP_PROXY_MODE $TCP_REDIR_PORT)
			}

			if [ "$PROXY_IPV6" == "1" ]; then
				$ip6t_m -A PSW $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST6) $(REDIRECT $TCP_REDIR_PORT TPROXY)
				$ip6t_m -A PSW $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST6) $(REDIRECT $TCP_REDIR_PORT TPROXY)
				$ip6t_m -A PSW $(comment "默认") -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ip6t $TCP_PROXY_MODE $TCP_REDIR_PORT TPROXY)
			fi

			echolog "${msg}"
		}
	fi
	$ipt_n -A PSW $(comment "默认") -p tcp -j RETURN
	$ipt_m -A PSW $(comment "默认") -p tcp -j RETURN
	$ip6t_m -A PSW $(comment "默认") -p tcp -j RETURN

	#  加载UDP默认代理模式
	if [ "$UDP_PROXY_MODE" != "disable" ]; then
		[ "$UDP_NO_REDIR_PORTS" != "disable" ] && {
			$ipt_m -A PSW $(comment "默认") -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN
			$ip6t_m -A PSW $(comment "默认") -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN
		}

		[ "$UDP_NODE" != "nil" -o "$TCP_UDP" = "1" ] && {
			[ "$TCP_UDP" = "1" ] && [ "$UDP_NODE" = "nil" ] && UDP_NODE=$TCP_NODE
			msg="UDP默认代理：使用UDP节点[$(config_n_get $UDP_NODE remarks)] [$(get_action_chain_name $UDP_PROXY_MODE)](TPROXY:${UDP_REDIR_PORT})代理"
			[ "$UDP_NO_REDIR_PORTS" != "disable" ] && msg="${msg}除${UDP_NO_REDIR_PORTS}外的"
			msg="${msg}所有端口"
			$ipt_m -A PSW $(comment "默认") -p udp -d $FAKE_IP $(REDIRECT $UDP_REDIR_PORT TPROXY)
			$ipt_m -A PSW $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST) $(REDIRECT $UDP_REDIR_PORT TPROXY)
			$ipt_m -A PSW $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $UDP_REDIR_PORT TPROXY)
			$ipt_m -A PSW $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $UDP_PROXY_MODE $UDP_REDIR_PORT TPROXY)

			if [ "$PROXY_IPV6" == "1" ]; then
				$ip6t_m -A PSW $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST6) $(REDIRECT $UDP_REDIR_PORT TPROXY)
				$ip6t_m -A PSW $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST6) $(REDIRECT $UDP_REDIR_PORT TPROXY)
				$ip6t_m -A PSW $(comment "默认") -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ip6t $UDP_PROXY_MODE $UDP_REDIR_PORT TPROXY)
			fi

			echolog "${msg}"
		}
	fi
	$ipt_m -A PSW $(comment "默认") -p udp -j RETURN
	$ip6t_m -A PSW $(comment "默认") -p udp -j RETURN
}

filter_haproxy() {
	for item in ${haproxy_items}; do
		local ip=$(get_host_ip ipv4 $(echo $item | awk -F ":" '{print $1}') 1)
		ipset -q add $IPSET_VPSIPLIST $ip
	done
	echolog "加入负载均衡的节点到ipset[$IPSET_VPSIPLIST]直连完成"
}

filter_vpsip() {
	uci show $CONFIG | grep ".address=" | cut -d "'" -f 2 | grep -E "([0-9]{1,3}[\.]){3}[0-9]{1,3}" | sed -e "/^$/d" | sed -e "s/^/add $IPSET_VPSIPLIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	uci show $CONFIG | grep ".address=" | cut -d "'" -f 2 | grep -E "([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}" | sed -e "/^$/d" | sed -e "s/^/add $IPSET_VPSIPLIST6 &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	echolog "加入所有节点到ipset[$IPSET_VPSIPLIST]直连完成"
}

filter_node() {
	local proxy_node=${1}
	local stream=$(echo ${2} | tr 'A-Z' 'a-z')
	local proxy_port=${3}

	filter_rules() {
		local node=${1}
		local stream=${2}
		local _proxy=${3}
		local _port=${4}
		local _is_tproxy ipt_tmp msg msg2

		if [ -n "$node" ] && [ "$node" != "nil" ]; then
			local type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
			local address=$(config_n_get $node address)
			local port=$(config_n_get $node port)
			ipt_tmp=$ipt_n
			_is_tproxy=${is_tproxy}
			[ "$stream" == "udp" ] && _is_tproxy="TPROXY"
			if [ -n "${_is_tproxy}" ]; then
				ipt_tmp=$ipt_m
				msg="TPROXY"
			else
				msg="REDIRECT"
			fi
		else
			echolog "  - 节点配置不正常，略过"
			return 0
		fi

		local ADD_INDEX=$FORCE_INDEX
		for _ipt in 4 6; do
			[ "$_ipt" == "4" ] && _ipt=$ipt_tmp
			[ "$_ipt" == "6" ] && _ipt=$ip6t_m
			$_ipt -n -L PSW_OUTPUT | grep -q "${address}:${port}"
			if [ $? -ne 0 ]; then
				unset dst_rule
				local dst_rule=$(REDIRECT 1 MARK)
				msg2="按规则路由(${msg})"
				[ "$_ipt" == "$ipt_m" -o "$_ipt" == "$ip6t_m" ] || {
					dst_rule=$(REDIRECT $_port)
					msg2="套娃使用(${msg}:${port} -> ${_port})"
				}
				[ -n "$_proxy" ] && [ "$_proxy" == "1" ] && [ -n "$_port" ] || {
					ADD_INDEX=$(RULE_LAST_INDEX "$_ipt" PSW_OUT_PUT "$IPSET_VPSIPLIST" $FORCE_INDEX)
					dst_rule=" -j RETURN"
					msg2="直连代理"
				}
				$_ipt -I PSW_OUTPUT $ADD_INDEX $(comment "${address}:${port}") -p $stream -d $address --dport $port $dst_rule 2>/dev/null
			else
				msg2="已配置过的节点，"
			fi
		done
		msg="[$?]$(echo ${2} | tr 'a-z' 'A-Z')${msg2}使用链${ADD_INDEX}，节点（${type}）：${address}:${port}"
		#echolog "  - ${msg}"
	}
	
	local proxy_protocol=$(config_n_get $proxy_node protocol)
	local proxy_type=$(echo $(config_n_get $proxy_node type nil) | tr 'A-Z' 'a-z')
	[ "$proxy_type" == "nil" ] && echolog "  - 节点配置不正常，略过！：${proxy_node}" && return 0
	if [ "$proxy_protocol" == "_balancing" ]; then
		#echolog "  - 多节点负载均衡（${proxy_type}）..."
		proxy_node=$(config_n_get $proxy_node balancing_node)
		for _node in $proxy_node; do
			filter_rules "$_node" "$stream"
		done
	elif [ "$proxy_protocol" == "_shunt" ]; then
		#echolog "  - 按请求目的地址分流（${proxy_type}）..."
		local default_node=$(config_n_get $proxy_node default_node _direct)
		local main_node=$(config_n_get $proxy_node main_node nil)
		if [ "$main_node" != "nil" ]; then
			filter_rules $main_node $stream
		else
			if [ "$default_node" != "_direct" ] && [ "$default_node" != "_blackhole" ]; then
				filter_rules $default_node $stream
			fi
		fi
:<<!
		local default_node_address=$(get_host_ip ipv4 $(config_n_get $default_node address) 1)
		local default_node_port=$(config_n_get $default_node port)
		
		local shunt_ids=$(uci show $CONFIG | grep "=shunt_rules" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')
		for shunt_id in $shunt_ids; do
			#local shunt_proxy=$(config_n_get $proxy_node "${shunt_id}_proxy" 0)
			local shunt_proxy=0
			local shunt_node=$(config_n_get $proxy_node "${shunt_id}" nil)
			[ "$shunt_node" != "nil" ] && {
				[ "$shunt_proxy" == 1 ] && {
					local shunt_node_address=$(get_host_ip ipv4 $(config_n_get $shunt_node address) 1)
					local shunt_node_port=$(config_n_get $shunt_node port)
					[ "$shunt_node_address" == "$default_node_address" ] && [ "$shunt_node_port" == "$default_node_port" ] && {
						shunt_proxy=0
					}
				}
				filter_rules "$(config_n_get $proxy_node $shunt_id)" "$stream" "$shunt_proxy" "$proxy_port"
			}
		done
!
	else
		#echolog "  - 普通节点（${proxy_type}）..."
		filter_rules "$proxy_node" "$stream"
	fi
}

dns_hijack() {
	$ipt_n -I PSW -p udp --dport 53 -j REDIRECT --to-ports 53
	echolog "强制转发本机DNS端口 UDP/53 的请求[$?]"
}

add_firewall_rule() {
	echolog "开始加载防火墙规则..."
	ipset -! create $IPSET_LANIPLIST nethash maxelem 1048576
	ipset -! create $IPSET_VPSIPLIST nethash maxelem 1048576
	ipset -! create $IPSET_SHUNTLIST nethash maxelem 1048576
	ipset -! create $IPSET_GFW nethash maxelem 1048576
	ipset -! create $IPSET_CHN nethash maxelem 1048576
	ipset -! create $IPSET_BLACKLIST nethash maxelem 1048576
	ipset -! create $IPSET_WHITELIST nethash maxelem 1048576
	ipset -! create $IPSET_BLOCKLIST nethash maxelem 1048576

	ipset -! create $IPSET_LANIPLIST6 nethash family inet6 maxelem 1048576
	ipset -! create $IPSET_VPSIPLIST6 nethash family inet6 maxelem 1048576
	ipset -! create $IPSET_SHUNTLIST6 nethash family inet6 maxelem 1048576
	ipset -! create $IPSET_GFW6 nethash family inet6 maxelem 1048576
	ipset -! create $IPSET_CHN6 nethash family inet6 maxelem 1048576
	ipset -! create $IPSET_BLACKLIST6 nethash family inet6 maxelem 1048576
	ipset -! create $IPSET_WHITELIST6 nethash family inet6 maxelem 1048576
	ipset -! create $IPSET_BLOCKLIST6 nethash family inet6 maxelem 1048576

	local shunt_ids=$(uci show $CONFIG | grep "=shunt_rules" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')

	for shunt_id in $shunt_ids; do
		config_n_get $shunt_id ip_list | tr -s "\r\n" "\n" | sed -e "/^$/d" | grep -E "(\.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}" | sed -e "s/^/add $IPSET_SHUNTLIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	done

	for shunt_id in $shunt_ids; do
		config_n_get $shunt_id ip_list | tr -s "\r\n" "\n" | sed -e "/^$/d" | grep -E "([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}" | sed -e "s/^/add $IPSET_SHUNTLIST6 &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	done

	cat $RULES_PATH/chnroute | sed -e "/^$/d" | sed -e "s/^/add $IPSET_CHN &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/proxy_ip | sed -e "/^$/d" | grep -E "(\.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}" | sed -e "s/^/add $IPSET_BLACKLIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/direct_ip | sed -e "/^$/d" | grep -E "(\.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}" | sed -e "s/^/add $IPSET_WHITELIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/block_ip | sed -e "/^$/d" | grep -E "(\.((2(5[0-5]|[0-4][0-9]))|[0-1]?[0-9]{1,2})){3}" | sed -e "s/^/add $IPSET_BLOCKLIST &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R

	cat $RULES_PATH/chnroute6 | sed -e "/^$/d" | sed -e "s/^/add $IPSET_CHN6 &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/proxy_ip | grep -E "([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}" | sed -e "/^$/d" | sed -e "s/^/add $IPSET_BLACKLIST6 &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/direct_ip | grep -E "([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}" | sed -e "/^$/d" | sed -e "s/^/add $IPSET_WHITELIST6 &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	cat $RULES_PATH/block_ip | grep -E "([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}" | sed -e "/^$/d" | sed -e "s/^/add $IPSET_BLOCKLIST6 &/g" | awk '{print $0} END{print "COMMIT"}' | ipset -! -R

	ipset -! -R <<-EOF
		$(gen_laniplist | sed -e "s/^/add $IPSET_LANIPLIST /")
	EOF

	ipset -! -R <<-EOF
		$(gen_laniplist_6 | sed -e "s/^/add $IPSET_LANIPLIST6 /")
	EOF
	
	# 忽略特殊IP段
	local lan_ifname lan_ip
	lan_ifname=$(uci -q -p /var/state get network.lan.ifname)
	[ -n "$lan_ifname" ] && {
		lan_ip=$(ip address show $lan_ifname | grep -w "inet" | awk '{print $2}')
		lan_ip6=$(ip address show $lan_ifname | grep -w "inet6" | awk '{print $2}')
		#echolog "本机IPv4网段互访直连：${lan_ip}"
		#echolog "本机IPv6网段互访直连：${lan_ip6}"

		[ -n "$lan_ip" ] && ipset -! -R <<-EOF
			$(echo $lan_ip | sed -e "s/ /\n/g" | sed -e "s/^/add $IPSET_LANIPLIST /")
		EOF

		[ -n "$lan_ip6" ] && ipset -! -R <<-EOF
			$(echo $lan_ip6 | sed -e "s/ /\n/g" | sed -e "s/^/add $IPSET_LANIPLIST6 /")
		EOF
	}

	local ISP_DNS=$(cat $RESOLVFILE 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | sort -u | grep -v 0.0.0.0 | grep -v 127.0.0.1)
	[ -n "$ISP_DNS" ] && {
		#echolog "处理 ISP DNS 例外..."
		for ispip in $ISP_DNS; do
			ipset -! add $IPSET_WHITELIST $ispip >/dev/null 2>&1 &
			#echolog "  - 追加到白名单：${ispip}"
		done
	}

	local ISP_DNS6=$(cat $RESOLVFILE 2>/dev/null | grep -E "([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}" | awk -F % '{print $1}' | awk -F " " '{print $2}'| sort -u | grep -v -Fx ::1 | grep -v -Fx ::)
	[ -n "$ISP_DNS" ] && {
		#echolog "处理 ISP IPv6 DNS 例外..."
		for ispip6 in $ISP_DNS; do
			ipset -! add $IPSET_WHITELIST6 $ispip6 >/dev/null 2>&1 &
			#echolog "  - 追加到白名单：${ispip6}"
		done
	} 

	#  过滤所有节点IP
	filter_vpsip > /dev/null 2>&1 &
	filter_haproxy > /dev/null 2>&1 &
	
	local accept_icmp=$(config_t_get global_forwarding accept_icmp 0)
	
	local tcp_proxy_way=$(config_t_get global_forwarding tcp_proxy_way redirect)
	if [ "$tcp_proxy_way" = "redirect" ]; then
		unset is_tproxy
	elif [ "$tcp_proxy_way" = "tproxy" ]; then
		is_tproxy="TPROXY"
	fi

	$ipt_n -N PSW
	$ipt_n -A PSW $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_n -A PSW $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_n -A PSW $(dst $IPSET_WHITELIST) -j RETURN
	$ipt_n -A PSW -m mark --mark 0xff -j RETURN
	PR_INDEX=$(RULE_LAST_INDEX "$ipt_n" PREROUTING prerouting_rule 1)
	PR_INDEX=$((PR_INDEX + 1))
	[ "$accept_icmp" = "1" ] && $ipt_n -I PREROUTING $PR_INDEX -p icmp -j PSW
	[ -z "${is_tproxy}" ] && $ipt_n -I PREROUTING $PR_INDEX -p tcp -j PSW
	unset PR_INDEX

	$ipt_n -N PSW_OUTPUT
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_n -A PSW_OUTPUT $(dst $IPSET_WHITELIST) -j RETURN
	$ipt_n -A PSW_OUTPUT -m mark --mark 0xff -j RETURN
	
	$ipt_n -N PSW_REDIRECT
	$ipt_n -I PREROUTING 1 -j PSW_REDIRECT
	
	# 据说能提升性能？
	PR_INDEX=$(RULE_LAST_INDEX "$ipt_m" PREROUTING mwan3 1)
	$ipt_m -N PSW_DIVERT
	$ipt_m -A PSW_DIVERT -j MARK --set-mark 1
	$ipt_m -A PSW_DIVERT -j ACCEPT
	$ipt_m -I PREROUTING $PR_INDEX -p tcp -m socket -j PSW_DIVERT

	$ipt_m -N PSW
	$ipt_m -A PSW $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_m -A PSW $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_m -A PSW $(dst $IPSET_WHITELIST) -j RETURN
	$ipt_m -A PSW -m mark --mark 0xff -j RETURN
	$ipt_m -A PSW $(dst $IPSET_BLOCKLIST) -j DROP
	PR_INDEX=$((PR_INDEX + 1))
	$ipt_m -I PREROUTING $PR_INDEX -j PSW
	unset PR_INDEX

	$ipt_m -N PSW_OUTPUT
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_WHITELIST) -j RETURN
	$ipt_m -A PSW_OUTPUT -m mark --mark 0xff -j RETURN
	$ipt_m -A PSW_OUTPUT $(dst $IPSET_BLOCKLIST) -j DROP

	ip rule add fwmark 1 lookup 100
	ip route add local 0.0.0.0/0 dev lo table 100
	
	# 据说能提升性能？
	$ip6t_m -N PSW_DIVERT
	$ip6t_m -A PSW_DIVERT -j MARK --set-mark 1
	$ip6t_m -A PSW_DIVERT -j ACCEPT
	$ip6t_m -A PREROUTING -p tcp -m socket -j PSW_DIVERT

	$ip6t_m -N PSW
	$ip6t_m -A PSW $(dst $IPSET_LANIPLIST6) -j RETURN
	$ip6t_m -A PSW $(dst $IPSET_VPSIPLIST6) -j RETURN
	$ip6t_m -A PSW $(dst $IPSET_WHITELIST6) -j RETURN
	$ip6t_m -A PSW -m mark --mark 0xff -j RETURN
	$ip6t_m -A PSW $(dst $IPSET_BLOCKLIST6) -j DROP
	$ip6t_m -A PREROUTING -j PSW

	$ip6t_m -N PSW_OUTPUT
	$ip6t_m -A PSW_OUTPUT $(dst $IPSET_LANIPLIST6) -j RETURN
	$ip6t_m -A PSW_OUTPUT $(dst $IPSET_VPSIPLIST6) -j RETURN
	$ip6t_m -A PSW_OUTPUT $(dst $IPSET_WHITELIST6) -j RETURN
	$ip6t_m -A PSW_OUTPUT -m mark --mark 0xff -j RETURN
	$ip6t_m -A PSW_OUTPUT $(dst $IPSET_BLOCKLIST6) -j DROP
	$ip6t_m -A OUTPUT -j PSW_OUTPUT

	ip -6 rule add fwmark 1 table 100
	ip -6 route add local ::/0 dev lo table 100

	# 加载路由器自身代理 TCP
	if [ "$TCP_NODE" != "nil" ]; then
		local ipt_tmp=$ipt_n
		local blist_r=$(REDIRECT $TCP_REDIR_PORT)
		local p_r=$(get_redirect_ipt $LOCALHOST_TCP_PROXY_MODE $TCP_REDIR_PORT)
		echolog "加载路由器自身 TCP 代理..."

		if [ -n "${is_tproxy}" ]; then
			echolog "  - 启用 TPROXY 模式"
			ipt_tmp=$ipt_m
			blist_r=$(REDIRECT 1 MARK)
			p_r=$(get_redirect_ipt $LOCALHOST_TCP_PROXY_MODE 1 MARK)
		fi
		
		[ "$accept_icmp" = "1" ] && {
			$ipt_n -A OUTPUT -p icmp -j PSW_OUTPUT
			$ipt_n -A PSW_OUTPUT -p icmp -d $FAKE_IP $(REDIRECT $TCP_REDIR_PORT)
			$ipt_n -A PSW_OUTPUT -p icmp $(dst $IPSET_SHUNTLIST) $(REDIRECT $TCP_REDIR_PORT)
			$ipt_n -A PSW_OUTPUT -p icmp $(dst $IPSET_BLACKLIST) $(REDIRECT $TCP_REDIR_PORT)
			$ipt_n -A PSW_OUTPUT -p icmp $(get_redirect_ipt $TCP_PROXY_MODE $TCP_REDIR_PORT)
		}
		
		_proxy_tcp_access() {
			[ -n "${2}" ] || return 0
			ipset -q test $IPSET_LANIPLIST ${2}
			[ $? -eq 0 ] && {
				echolog "  - 上游 DNS 服务器 ${2} 已在直接访问的列表中，不强制向 TCP 代理转发对该服务器 TCP/${3} 端口的访问"
				return 0
			}
			if [ -n "${is_tproxy}" ]; then
				$ipt_m -I PSW_OUTPUT -p tcp -d ${2} --dport ${3} $(REDIRECT 1 MARK)
				$ipt_m -I PSW $(comment "本机") -p tcp -i lo -d ${2} --dport ${3} $(REDIRECT $TCP_REDIR_PORT TPROXY)
			else
				$ipt_n -I PSW_OUTPUT -p tcp -d ${2} --dport ${3} $(REDIRECT $TCP_REDIR_PORT)
			fi
			echolog "  - [$?]将上游 DNS 服务器 ${2}:${3} 加入到路由器自身代理的 TCP 转发链"
		}
		
		[ "$use_tcp_node_resolve_dns" == 1 ] && hosts_foreach DNS_FORWARD _proxy_tcp_access 53
		$ipt_tmp -A OUTPUT -p tcp -j PSW_OUTPUT
		[ "$TCP_NO_REDIR_PORTS" != "disable" ] && {
			$ipt_tmp -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
			$ipt_tmp -A PSW_OUTPUT -p tcp -m multiport --sport $TCP_NO_REDIR_PORTS -j RETURN
			$ip6t_m -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
			$ip6t_m -A PSW_OUTPUT -p tcp -m multiport --sport $TCP_NO_REDIR_PORTS -j RETURN
			echolog "  - [$?]不代理TCP 端口：$TCP_NO_REDIR_PORTS"
		}
		
		$ipt_tmp -A PSW_OUTPUT -p tcp -d $FAKE_IP $blist_r
		$ipt_tmp -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST) $blist_r
		$ipt_tmp -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $blist_r
		$ipt_tmp -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $p_r
		
		if [ -n "${is_tproxy}" ]; then
			$ipt_m -A PSW $(comment "本机") -p tcp -i lo -d $FAKE_IP $(REDIRECT $TCP_REDIR_PORT TPROXY)
			$ipt_m -A PSW $(comment "本机") -p tcp -i lo $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST) $(REDIRECT $TCP_REDIR_PORT TPROXY)
			$ipt_m -A PSW $(comment "本机") -p tcp -i lo $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $TCP_REDIR_PORT TPROXY)
			$ipt_m -A PSW $(comment "本机") -p tcp -i lo $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $LOCALHOST_TCP_PROXY_MODE $TCP_REDIR_PORT TPROXY)
			$ipt_m -A PSW $(comment "本机") -p tcp -i lo -j RETURN
		fi

		if [ "$PROXY_IPV6" == "1" ]; then
			$ip6t_m -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST6) $(REDIRECT 1 MARK)
			$ip6t_m -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST6) $(REDIRECT 1 MARK)
			$ip6t_m -A PSW_OUTPUT -p tcp $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ip6t $LOCALHOST_TCP_PROXY_MODE 1 MARK)
			
			$ip6t_m -A PSW $(comment "本机") -p tcp -i lo $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST6) $(REDIRECT $TCP_REDIR_PORT $is_tproxy)
			$ip6t_m -A PSW $(comment "本机") -p tcp -i lo $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST6) $(REDIRECT $TCP_REDIR_PORT $is_tproxy)
			$ip6t_m -A PSW $(comment "本机") -p tcp -i lo $(factor $TCP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ip6t $LOCALHOST_TCP_PROXY_MODE $TCP_REDIR_PORT $is_tproxy)
			$ip6t_m -A PSW $(comment "本机") -p tcp -i lo -j RETURN
		fi
	fi

	# 过滤Socks节点
	[ "$SOCKS_ENABLED" = "1" ] && {
		local ids=$(uci show $CONFIG | grep "=socks" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')
		#echolog "分析 Socks 服务所使用节点..."
		local id enabled node port msg num
		for id in $ids; do
			enabled=$(config_n_get $id enabled 0)
			[ "$enabled" == "1" ] || continue
			node=$(config_n_get $id node nil)
			port=$(config_n_get $id port 0)
			msg="Socks 服务 [:${port}]"
			if [ "$node" == "nil" ] || [ "$port" == "0" ]; then
				msg="${msg} 未配置完全，略过"
			elif [ "$(echo $node | grep ^tcp)" ]; then
				#eval "node=\${TCP_NODE}"
				#msg="${msg} 使用与 TCP 代理自动切换${num} 相同的节点，延后处理"
				continue
			else
				filter_node $node TCP > /dev/null 2>&1 &
				filter_node $node UDP > /dev/null 2>&1 &
			fi
			#echolog "  - ${msg}"
		done
	}

	# 处理轮换节点的分流或套娃
	local node port stream switch
	for stream in TCP UDP; do
		eval "node=\${${stream}_NODE}"
		eval "port=\${${stream}_REDIR_PORT}"
		#echolog "分析 $stream 代理自动切换..."
		[ "$node" == "tcp" ] && [ "$stream" == "UDP" ] && {
			eval "node=\${TCP_NODE}"
			eval "port=\${TCP_REDIR_PORT}"
		}
		if [ "$node" != "nil" ]; then
			filter_node $node $stream $port > /dev/null 2>&1 &
		fi
	done

	# 加载路由器自身代理 UDP
	if [ "$UDP_NODE" != "nil" -o "$TCP_UDP" = "1" ]; then
		echolog "加载路由器自身 UDP 代理..."
		_proxy_udp_access() {
			[ -n "${2}" ] || return 0
			ipset -q test $IPSET_LANIPLIST ${2}
			[ $? == 0 ] && {
				echolog "  - 上游 DNS 服务器 ${2} 已在直接访问的列表中，不强制向 UDP 代理转发对该服务器 UDP/${3} 端口的访问"
				return 0
			}
			$ipt_m -I PSW_OUTPUT -p udp -d ${2} --dport ${3} $(REDIRECT 1 MARK)
			$ipt_m -I PSW $(comment "本机") -p udp -i lo -d ${2} --dport ${3} $(REDIRECT $UDP_REDIR_PORT TPROXY)
			echolog "  - [$?]将上游 DNS 服务器 ${2}:${3} 加入到路由器自身代理的 UDP 转发链"
		}
		[ "$use_udp_node_resolve_dns" == 1 ] && hosts_foreach DNS_FORWARD _proxy_udp_access 53
		$ipt_m -A OUTPUT -p udp -j PSW_OUTPUT
		[ "$UDP_NO_REDIR_PORTS" != "disable" ] && {
			$ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN
			$ipt_m -A PSW_OUTPUT -p udp -m multiport --sport $UDP_NO_REDIR_PORTS -j RETURN
			$ip6t_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN
			$ip6t_m -A PSW_OUTPUT -p udp -m multiport --sport $UDP_NO_REDIR_PORTS -j RETURN
			echolog "  - [$?]不代理 UDP 端口：$UDP_NO_REDIR_PORTS"
		}
		$ipt_m -A PSW_OUTPUT -p udp -d $FAKE_IP $(REDIRECT 1 MARK)
		$ipt_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST) $(REDIRECT 1 MARK)
		$ipt_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT 1 MARK)
		$ipt_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $LOCALHOST_UDP_PROXY_MODE 1 MARK)

		$ipt_m -A PSW $(comment "本机") -p udp -i lo -d $FAKE_IP $(REDIRECT $UDP_REDIR_PORT TPROXY)
		$ipt_m -A PSW $(comment "本机") -p udp -i lo $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST) $(REDIRECT $UDP_REDIR_PORT TPROXY)
		$ipt_m -A PSW $(comment "本机") -p udp -i lo $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST) $(REDIRECT $UDP_REDIR_PORT TPROXY)
		$ipt_m -A PSW $(comment "本机") -p udp -i lo $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ipt $LOCALHOST_UDP_PROXY_MODE $UDP_REDIR_PORT TPROXY)
		$ipt_m -A PSW $(comment "本机") -p udp -i lo -j RETURN

		if [ "$PROXY_IPV6" == "1" ]; then
			$ip6t_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST6) $(REDIRECT 1 MARK)
			$ip6t_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST6) $(REDIRECT 1 MARK)
			$ip6t_m -A PSW_OUTPUT -p udp $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ip6t $LOCALHOST_UDP_PROXY_MODE 1 MARK)
			
			$ip6t_m -A PSW $(comment "本机") -p udp -i lo $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_SHUNTLIST6) $(REDIRECT $UDP_REDIR_PORT $is_tproxy)
			$ip6t_m -A PSW $(comment "本机") -p udp -i lo $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(dst $IPSET_BLACKLIST6) $(REDIRECT $UDP_REDIR_PORT $is_tproxy)
			$ip6t_m -A PSW $(comment "本机") -p udp -i lo $(factor $UDP_REDIR_PORTS "-m multiport --dport") $(get_redirect_ip6t $LOCALHOST_UDP_PROXY_MODE $UDP_REDIR_PORT $is_tproxy)
			$ip6t_m -A PSW $(comment "本机") -p udp -i lo -j RETURN
		fi
	fi

	#  加载ACLS
	load_acl

	# dns_hijack "force"

	echolog "防火墙规则加载完成！"
}

del_firewall_rule() {
	for ipt in "$ipt_n" "$ipt_m" "$ip6t_m"; do
		for chain in "PREROUTING" "OUTPUT"; do
			for i in $(seq 1 $($ipt -nL $chain | grep -c PSW)); do
				local index=$($ipt --line-number -nL $chain | grep PSW | head -1 | awk '{print $1}')
				$ipt -D $chain $index 2>/dev/null
			done
		done
		for chain in "PSW" "PSW_OUTPUT" "PSW_DIVERT" "PSW_REDIRECT"; do
			$ipt -F $chain 2>/dev/null
			$ipt -X $chain 2>/dev/null
		done
	done
	
	ip rule del fwmark 1 lookup 100 2>/dev/null
	ip route del local 0.0.0.0/0 dev lo table 100 2>/dev/null

	ip -6 rule del fwmark 1 table 100 2>/dev/null
	ip -6 route del local ::/0 dev lo table 100 2>/dev/null
	
	destroy_ipset $IPSET_LANIPLIST
	destroy_ipset $IPSET_VPSIPLIST
	#destroy_ipset $IPSET_SHUNTLIST
	#destroy_ipset $IPSET_GFW
	#destroy_ipset $IPSET_CHN
	#destroy_ipset $IPSET_BLACKLIST
	destroy_ipset $IPSET_BLOCKLIST
	destroy_ipset $IPSET_WHITELIST
	
	destroy_ipset $IPSET_LANIPLIST6
	destroy_ipset $IPSET_VPSIPLIST6
	#destroy_ipset $IPSET_SHUNTLIST6
	#destroy_ipset $IPSET_GFW6
	#destroy_ipset $IPSET_CHN6
	#destroy_ipset $IPSET_BLACKLIST6
	destroy_ipset $IPSET_BLOCKLIST6
	destroy_ipset $IPSET_WHITELIST6
	
	echolog "删除相关防火墙规则完成。"
}

flush_ipset() {
	del_firewall_rule
	destroy_ipset $IPSET_VPSIPLIST $IPSET_SHUNTLIST $IPSET_GFW $IPSET_CHN $IPSET_BLACKLIST $IPSET_BLOCKLIST $IPSET_WHITELIST $IPSET_LANIPLIST
	destroy_ipset $IPSET_VPSIPLIST6 $IPSET_SHUNTLIST6 $IPSET_GFW6 $IPSET_CHN6 $IPSET_BLACKLIST6 $IPSET_BLOCKLIST6 $IPSET_WHITELIST6 $IPSET_LANIPLIST6 
	/etc/init.d/passwall reload
}

flush_include() {
	echo '#!/bin/sh' >$FWI
}

gen_include() {
	flush_include
	cat <<-EOF >>$FWI
		echo "" > $LOG_FILE
		/etc/init.d/passwall reload
	EOF
	return 0
}

start() {
	add_firewall_rule
	gen_include
}

stop() {
	del_firewall_rule
	flush_include
}

case $1 in
flush_ipset)
	flush_ipset
	;;
stop)
	stop
	;;
start)
	start
	;;
*) ;;
esac
