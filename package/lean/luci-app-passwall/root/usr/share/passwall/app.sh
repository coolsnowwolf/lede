#!/bin/sh
# Copyright (C) 2018-2020 Lienol <lawlienol@gmail.com>

. $IPKG_INSTROOT/lib/functions.sh
. $IPKG_INSTROOT/lib/functions/service.sh

CONFIG=passwall
TMP_PATH=/var/etc/$CONFIG
TMP_BIN_PATH=$TMP_PATH/bin
TMP_ID_PATH=$TMP_PATH/id
TMP_PORT_PATH=$TMP_PATH/port
LOG_FILE=/var/log/$CONFIG.log
APP_PATH=/usr/share/$CONFIG
RULES_PATH=/usr/share/${CONFIG}/rules
TMP_DNSMASQ_PATH=/var/etc/dnsmasq-passwall.d
DNSMASQ_PATH=/etc/dnsmasq.d
RESOLVFILE=/tmp/resolv.conf.d/resolv.conf.auto
DNS_PORT=7913
LUA_API_PATH=/usr/lib/lua/luci/model/cbi/$CONFIG/api
API_GEN_TROJAN=$LUA_API_PATH/gen_trojan.lua
API_GEN_V2RAY=$LUA_API_PATH/gen_v2ray.lua
API_GEN_V2RAY_BALANCING=$LUA_API_PATH/gen_v2ray_balancing.lua
API_GEN_V2RAY_SHUNT=$LUA_API_PATH/gen_v2ray_shunt.lua

echolog() {
	local d="$(date "+%Y-%m-%d %H:%M:%S")"
	echo -e "$d: $1" >>$LOG_FILE
}

find_bin() {
	bin_name=$1
	result=$(find /usr/*bin -iname "$bin_name" -type f)
	if [ -z "$result" ]; then
		echo "null"
	else
		echo "$result"
	fi
}

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

get_host_ip() {
	local host=$2
	local count=$3
	[ -z "$count" ] && count=3
	local isip=""
	local ip=$host
	if [ "$1" == "ipv6" ]; then
		isip=$(echo $host | grep -E "([[a-f0-9]{1,4}(:[a-f0-9]{1,4}){7}|[a-f0-9]{1,4}(:[a-f0-9]{1,4}){0,7}::[a-f0-9]{0,4}(:[a-f0-9]{1,4}){0,7}])")
		if [ -n "$isip" ]; then
			isip=$(echo $host | cut -d '[' -f2 | cut -d ']' -f1)
		else
			isip=$(echo $host | grep -E "([a-f0-9]{1,4}(:[a-f0-9]{1,4}){7}|[a-f0-9]{1,4}(:[a-f0-9]{1,4}){0,7}::[a-f0-9]{0,4}(:[a-f0-9]{1,4}){0,7})")
		fi
	else
		isip=$(echo $host | grep -E "([0-9]{1,3}[\.]){3}[0-9]{1,3}")
	fi
	[ -z "$isip" ] && {
		local t=4
		[ "$1" == "ipv6" ] && t=6
		local vpsrip=$(resolveip -$t -t $count $host | awk 'NR==1{print}')
		ip=$vpsrip
	}
	echo $ip
}

get_node_host_ip() {
	local ip
	local address=$(config_n_get $1 address)
	[ -n "$address" ] && {
		local use_ipv6=$(config_n_get $1 use_ipv6)
		local network_type="ipv4"
		[ "$use_ipv6" == "1" ] && network_type="ipv6"
		ip=$(get_host_ip $network_type $address)
	}
	echo $ip
}

check_port_exists() {
	port=$1
	protocol=$2
	result=
	if [ "$protocol" = "tcp" ]; then
		result=$(netstat -tln | grep -c ":$port")
	elif [ "$protocol" = "udp" ]; then
		result=$(netstat -uln | grep -c ":$port")
	fi
	if [ "$result" = 1 ]; then
		echo 1
	else
		echo 0
	fi
}

get_new_port() {
	port=$1
	[ "$port" == "auto" ] && port=$SOCKS_REDIR_PORT3
	protocol=$2
	result=$(check_port_exists $port $protocol)
	if [ "$result" = 1 ]; then
		temp=
		if [ "$port" -lt 65535 ]; then
			temp=$(expr $port + 1)
		elif [ "$port" -gt 1 ]; then
			temp=$(expr $port - 1)
		fi
		get_new_port $temp $protocol
	else
		echo $port
	fi
}

ln_start_bin() {
	local file=$1
	[ "$file" != "null" ] && {
		local bin=$2
		local cmd=$3
		if [ -n "${TMP_BIN_PATH}/$bin" -a -f "${TMP_BIN_PATH}/$bin" ];then
			${TMP_BIN_PATH}/$bin $cmd >/dev/null 2>&1 &
		else
			if [ -n "$file" -a -f "$file" ];then
				ln -s $file ${TMP_BIN_PATH}/$bin
				${TMP_BIN_PATH}/$bin $cmd >/dev/null 2>&1 &
			else
				echolog "找不到$bin主程序，无法启动！"
			fi
		fi
	}
}

ENABLED=$(config_t_get global enabled 0)

TCP_NODE_NUM=$(config_t_get global_other tcp_node_num 1)
for i in $(seq 1 $TCP_NODE_NUM); do
	eval TCP_NODE$i=$(config_t_get global tcp_node$i nil)
done
TCP_REDIR_PORT1=$(config_t_get global_forwarding tcp_redir_port 1041)
TCP_REDIR_PORT2=$(expr $TCP_REDIR_PORT1 + 1)
TCP_REDIR_PORT3=$(expr $TCP_REDIR_PORT2 + 1)

UDP_NODE_NUM=$(config_t_get global_other udp_node_num 1)
for i in $(seq 1 $UDP_NODE_NUM); do
	eval UDP_NODE$i=$(config_t_get global udp_node$i nil)
done
UDP_REDIR_PORT1=$(config_t_get global_forwarding udp_redir_port 1051)
UDP_REDIR_PORT2=$(expr $UDP_REDIR_PORT1 + 1)
UDP_REDIR_PORT3=$(expr $UDP_REDIR_PORT2 + 1)

SOCKS_NODE_NUM=$(config_t_get global_other socks_node_num 1)
for i in $(seq 1 $SOCKS_NODE_NUM); do
	eval SOCKS_NODE$i=$(config_t_get global socks_node$i nil)
done
SOCKS_REDIR_PORT1=$(config_t_get global_forwarding socks_proxy_port1 1081)
SOCKS_REDIR_PORT2=$(config_t_get global_forwarding socks_proxy_port2 1082)
SOCKS_REDIR_PORT3=$(config_t_get global_forwarding socks_proxy_port3 1083)

[ "$UDP_NODE1" == "tcp" ] && UDP_NODE1=$TCP_NODE1
[ "$SOCKS_NODE1" == "tcp" ] && SOCKS_NODE1=$TCP_NODE1

# Dynamic variables (Used to record)
# TCP_NODE1_IP="" UDP_NODE1_IP="" SOCKS_NODE1_IP="" TCP_NODE1_PORT="" UDP_NODE1_PORT="" SOCKS_NODE1_PORT="" TCP_NODE1_TYPE="" UDP_NODE1_TYPE="" SOCKS_NODE1_TYPE=""

TCP_REDIR_PORTS=$(config_t_get global_forwarding tcp_redir_ports '80,443')
UDP_REDIR_PORTS=$(config_t_get global_forwarding udp_redir_ports '1:65535')
TCP_NO_REDIR_PORTS=$(config_t_get global_forwarding tcp_no_redir_ports 'disable')
UDP_NO_REDIR_PORTS=$(config_t_get global_forwarding udp_no_redir_ports 'disable')
KCPTUN_REDIR_PORT=$(config_t_get global_forwarding kcptun_port 12948)
TCP_PROXY_MODE=$(config_t_get global tcp_proxy_mode chnroute)
UDP_PROXY_MODE=$(config_t_get global udp_proxy_mode chnroute)
LOCALHOST_TCP_PROXY_MODE=$(config_t_get global localhost_tcp_proxy_mode default)
LOCALHOST_UDP_PROXY_MODE=$(config_t_get global localhost_udp_proxy_mode default)
[ "$LOCALHOST_TCP_PROXY_MODE" == "default" ] && LOCALHOST_TCP_PROXY_MODE=$TCP_PROXY_MODE
[ "$LOCALHOST_UDP_PROXY_MODE" == "default" ] && LOCALHOST_UDP_PROXY_MODE=$UDP_PROXY_MODE

load_config() {
	[ "$ENABLED" != 1 ] && return 1
	[ "$TCP_NODE1" == "nil" -a "$UDP_NODE1" == "nil" -a "$SOCKS_NODE1" == "nil" ] && {
		echolog "没有选择节点！"
		return 1
	}
	
	DNS_MODE=$(config_t_get global dns_mode pdnsd)
	DNS_FORWARD=$(config_t_get global dns_forward 8.8.4.4)
	use_tcp_node_resolve_dns=0
	use_udp_node_resolve_dns=0
	process=1
	if [ "$(config_t_get global_forwarding process 0)" = "0" ]; then
		process=$(cat /proc/cpuinfo | grep 'processor' | wc -l)
	else
		process=$(config_t_get global_forwarding process)
	fi
	UP_CHINA_DNS=$(config_t_get global up_china_dns dnsbyisp)
	[ "$UP_CHINA_DNS" == "default" ] && IS_DEFAULT_CHINA_DNS=1
	[ ! -f "$RESOLVFILE" -o ! -s "$RESOLVFILE" ] && RESOLVFILE=/tmp/resolv.conf.auto
	if [ "$UP_CHINA_DNS" == "dnsbyisp" -o "$UP_CHINA_DNS" == "default" ]; then
		UP_CHINA_DNS1=$(cat $RESOLVFILE 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | grep -v 0.0.0.0 | grep -v 127.0.0.1 | sed -n '1P')
		DEFAULT_DNS1="$UP_CHINA_DNS1"
		[ -z "$UP_CHINA_DNS1" ] && UP_CHINA_DNS1="119.29.29.29"
		UP_CHINA_DNS="$UP_CHINA_DNS1"
		UP_CHINA_DNS2=$(cat $RESOLVFILE 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | grep -v 0.0.0.0 | grep -v 127.0.0.1 | sed -n '2P')
		[ -n "$UP_CHINA_DNS1" -a -n "$UP_CHINA_DNS2" ] && UP_CHINA_DNS="$UP_CHINA_DNS1,$UP_CHINA_DNS2"
	else
		UP_CHINA_DNS1=$(echo $UP_CHINA_DNS | sed "s/:/#/g" | awk -F ',' '{print $1}')
		if [ -n "$UP_CHINA_DNS1" ]; then
			UP_CHINA_DNS2=$(echo $UP_CHINA_DNS | sed "s/:/#/g" | awk -F ',' '{print $2}')
			[ -n "$UP_CHINA_DNS2" ] && UP_CHINA_DNS="${UP_CHINA_DNS1},${UP_CHINA_DNS2}"
		else
			UP_CHINA_DNS1="119.29.29.29"
			UP_CHINA_DNS=$UP_CHINA_DNS1
		fi
	fi
	PROXY_IPV6=$(config_t_get global_forwarding proxy_ipv6 0)
	mkdir -p /var/etc $TMP_PATH $TMP_BIN_PATH $TMP_ID_PATH $TMP_PORT_PATH
	return 0
}

gen_ss_ssr_config_file() {
	local type local_port kcptun node configfile
	type=$1
	local_port=$2
	kcptun=$3
	node=$4
	configfile=$5
	local port encrypt_method
	port=$(config_n_get $node port)
	encrypt_method=$(config_n_get $node ss_encrypt_method)
	[ "$type" == "ssr" ] && encrypt_method=$(config_n_get $node ssr_encrypt_method)
	[ "$kcptun" == "1" ] && {
		server_host=127.0.0.1
		port=$KCPTUN_REDIR_PORT
	}
	cat <<-EOF >$configfile
		{
		    "server": "$server_host",
		    "server_port": $port,
		    "local_address": "0.0.0.0",
		    "local_port": $local_port,
		    "password": "$(config_n_get $node password)",
		    "timeout": $(config_n_get $node timeout),
		    "method": "$encrypt_method",
		    "fast_open": $(config_n_get $node tcp_fast_open false),
		    "reuse_port": true,
	EOF
	[ "$1" == "ssr" ] && {
		cat <<-EOF >>$configfile
		    "protocol": "$(config_n_get $node protocol)",
		    "protocol_param": "$(config_n_get $node protocol_param)",
		    "obfs": "$(config_n_get $node obfs)",
		    "obfs_param": "$(config_n_get $node obfs_param)"
		EOF
	}
	echo -e "}" >>$configfile
}

gen_start_config() {
	local node local_port redir_type config_file bind server_host port type
	node=$1
	local_port=$2
	redir_type=$3
	config_file=$4
	bind="0.0.0.0"
	[ -n "$6" ] && bind="$6"
	type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
	remarks=$(config_n_get $node remarks)
	server_host=$(config_n_get $node address)
	port=$(config_n_get $node port)
	[ -n "$server_host" -a -n "$port" ] && {
		# 判断节点服务器地址是否URL并去掉~
		server_host=$(echo $server_host | sed 's/^\(https:\/\/\|http:\/\/\)//g' | awk -F '/' '{print $1}')
		# 判断节点服务器地址是否包含汉字~
		local tmp=$(echo -n $server_host | awk '{print gensub(/[!-~]/,"","g",$0)}')
		[ -n "$tmp" ] && {
			echolog "${redir_type}_${5}节点，非法的服务器地址，无法启动！"
			return 1
		}
		[ "$bind" == "0.0.0.0" ] && echolog "${redir_type}_${5}节点：$remarks，节点：${server_host}:${port}，监听端口：$local_port"
	}

	if [ "$redir_type" == "SOCKS" ]; then
		eval SOCKS_NODE${5}_PORT=$port
		if [ "$type" == "socks5" ]; then
			echolog "Socks节点不能使用Socks代理节点！"
		elif [ "$type" == "v2ray" ]; then
			lua $API_GEN_V2RAY $node nil nil $local_port >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_socks_$5 "-config=$config_file"
		elif [ "$type" == "v2ray_balancing" ]; then
			lua $API_GEN_V2RAY_BALANCING $node nil nil $local_port >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_socks_$5 "-config=$config_file"
		elif [ "$type" == "v2ray_shunt" ]; then
			lua $API_GEN_V2RAY_SHUNT $node nil nil $local_port >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_socks_$5 "-config=$config_file"
		elif [ "$type" == "trojan" ]; then
			lua $API_GEN_TROJAN $node client $bind $local_port >$config_file
			ln_start_bin $(find_bin trojan) trojan_socks_$5 "-c $config_file"
		elif [ "$type" == "brook" ]; then
			local protocol=$(config_n_get $node brook_protocol client)
			local brook_tls=$(config_n_get $node brook_tls 0)
			[ "$protocol" == "wsclient" ] && {
				[ "$brook_tls" == "1" ] && server_host="wss://${server_host}" || server_host="ws://${server_host}" 
			}
			ln_start_bin $(config_t_get global_app brook_file $(find_bin brook)) brook_socks_$5 "$protocol -l $bind:$local_port -i $$bind -s $server_host:$port -p $(config_n_get $node password)"
		elif [ "$type" == "ssr" ]; then
			gen_ss_ssr_config_file ssr $local_port 0 $node $config_file
			ln_start_bin $(find_bin ssr-local) ssr-local_socks_$5 "-c $config_file -b $bind -u"
		elif [ "$type" == "ss" ]; then
			gen_ss_ssr_config_file ss $local_port 0 $node $config_file
			local plugin_params=""
			local plugin=$(config_n_get $node ss_plugin)
			if [ "$plugin" != "none" ]; then
				[ "$plugin" == "v2ray-plugin" -o "$plugin" == "obfs-local" ] && {
					local plugin_opts=""
					local opts=$(config_n_get $node ss_plugin_opts)
					[ -n "$opts" ] && plugin_opts="--plugin-opts $opts"
					plugin_params="--plugin $plugin $plugin_opts"
				}
			fi
			ln_start_bin $(find_bin ss-local) ss-local_socks_$5 "-c $config_file -b $bind -u $plugin_params" 
		fi
	fi

	if [ "$redir_type" == "UDP" ]; then
		eval UDP_NODE${5}_PORT=$port
		
		if [ "$type" == "socks5" ]; then
			local node_address=$(config_n_get $node address)
			local node_port=$(config_n_get $node port)
			local server_username=$(config_n_get $node username)
			local server_password=$(config_n_get $node password)
			eval port=\$UDP_REDIR_PORT$5
			ln_start_bin $(find_bin ipt2socks) ipt2socks_udp_$5 "-4 -U -l $port -b 0.0.0.0 -s $node_address -p $node_port -R"
		elif [ "$type" == "v2ray" ]; then
			lua $API_GEN_V2RAY $node udp $local_port nil >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_udp_$5 "-config=$config_file"
		elif [ "$type" == "v2ray_balancing" ]; then
			lua $API_GEN_V2RAY_BALANCING $node udp $local_port nil >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_udp_$5 "-config=$config_file"
		elif [ "$type" == "v2ray_shunt" ]; then
			lua $API_GEN_V2RAY_SHUNT $node udp $local_port nil >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_udp_$5 "-config=$config_file"
		elif [ "$type" == "trojan" ]; then
			SOCKS_REDIR_PORT4=$(expr $SOCKS_REDIR_PORT3 + 1)
			local_port=$(get_new_port $SOCKS_REDIR_PORT4 tcp)
			socks_port=$local_port
			lua $API_GEN_TROJAN $node client "127.0.0.1" $socks_port >$config_file
			ln_start_bin $(find_bin trojan) trojan_udp_$5 "-c $config_file"
			
			local node_address=$(config_n_get $node address)
			local node_port=$(config_n_get $node port)
			local server_username=$(config_n_get $node username)
			local server_password=$(config_n_get $node password)
			eval port=\$UDP_REDIR_PORT$5
			ln_start_bin $(find_bin ipt2socks) ipt2socks_udp_$5 "-4 -U -l $port -b 0.0.0.0 -s 127.0.0.1 -p $socks_port -R"
		elif [ "$type" == "brook" ]; then
			local protocol=$(config_n_get $node brook_protocol client)
			if [ "$protocol" == "wsclient" ]; then
				echolog "Brook的WebSocket不支持UDP转发！"
			else
				ln_start_bin $(config_t_get global_app brook_file $(find_bin brook)) brook_udp_$5 "tproxy -l 0.0.0.0:$local_port -s $server_host:$port -p $(config_n_get $node password)"
			fi
		elif [ "$type" == "ssr" ]; then
			gen_ss_ssr_config_file ssr $local_port 0 $node $config_file
			ln_start_bin $(find_bin ssr-redir) ssr-redir_udp_$5 "-c $config_file -U"
		elif [ "$type" == "ss" ]; then
			gen_ss_ssr_config_file ss $local_port 0 $node $config_file
			local plugin_params=""
			local plugin=$(config_n_get $node ss_plugin)
			if [ "$plugin" != "none" ]; then
				[ "$plugin" == "v2ray-plugin" -o "$plugin" == "obfs-local" ] && {
					local plugin_opts=""
					local opts=$(config_n_get $node ss_plugin_opts)
					[ -n "$opts" ] && plugin_opts="--plugin-opts $opts"
					plugin_params="--plugin $plugin $plugin_opts"
				}
			fi
			ln_start_bin $(find_bin ss-redir) ss-redir_udp_$5 "-c $config_file -U $plugin_params"
		fi
	fi

	if [ "$redir_type" == "TCP" ]; then
		eval TCP_NODE${5}_PORT=$port
		
		if [ "$type" == "socks5" ]; then
			local node_address=$(config_n_get $node address)
			local node_port=$(config_n_get $node port)
			local server_username=$(config_n_get $node username)
			local server_password=$(config_n_get $node password)
			eval port=\$TCP_REDIR_PORT$5
			ln_start_bin $(find_bin ipt2socks) ipt2socks_tcp_$5 "-4 -T -l $port -b 0.0.0.0 -s $node_address -p $node_port -R"
		elif [ "$type" == "v2ray" ]; then
			lua $API_GEN_V2RAY $node tcp $local_port nil >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_tcp_$5 "-config=$config_file"
		elif [ "$type" == "v2ray_balancing" ]; then
			lua $API_GEN_V2RAY_BALANCING $node tcp $local_port nil >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_tcp_$5 "-config=$config_file"
		elif [ "$type" == "v2ray_shunt" ]; then
			lua $API_GEN_V2RAY_SHUNT $node tcp $local_port nil >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray_tcp_$5 "-config=$config_file"
		elif [ "$type" == "trojan" ]; then
			lua $API_GEN_TROJAN $node nat "0.0.0.0" $local_port >$config_file
			for k in $(seq 1 $process); do
				ln_start_bin $(find_bin trojan) trojan_tcp_$5 "-c $config_file"
			done
		else
			local kcptun_use=$(config_n_get $node use_kcp 0)
			if [ "$kcptun_use" == "1" ]; then
				local kcptun_server_host=$(config_n_get $node kcp_server)
				local kcptun_port=$(config_n_get $node kcp_port)
				local kcptun_config="$(config_n_get $node kcp_opts)"
				if [ -z "$kcptun_port" -o -z "$kcptun_config" ]; then
					echolog "Kcptun未配置参数，错误！"
					force_stop
				fi
				if [ -n "$kcptun_port" -a -n "$kcptun_config" ]; then
					local run_kcptun_ip=$server_host
					[ -n "$kcptun_server_host" ] && run_kcptun_ip=$(get_host_ip $network_type $kcptun_server_host)
					KCPTUN_REDIR_PORT=$(get_new_port $KCPTUN_REDIR_PORT tcp)
					ln_start_bin $(config_t_get global_app kcptun_client_file $(find_bin kcptun-client)) kcptun_tcp_$5 "-l 0.0.0.0:$KCPTUN_REDIR_PORT -r $run_kcptun_ip:$kcptun_port $kcptun_config"
				fi
			fi
			if [ "$type" == "ssr" ]; then
				gen_ss_ssr_config_file ssr $local_port $kcptun_use $node $config_file
				for k in $(seq 1 $process); do
					ln_start_bin $(find_bin ssr-redir) ssr-redir_tcp_$5 "-c $config_file"
				done
			elif [ "$type" == "ss" ]; then
				gen_ss_ssr_config_file ss $local_port $kcptun_use $node $config_file
				local plugin_params=""
				local plugin=$(config_n_get $node ss_plugin)
				if [ "$plugin" != "none" ]; then
					[ "$plugin" == "v2ray-plugin" -o "$plugin" == "obfs-local" ] && {
						local plugin_opts=""
						local opts=$(config_n_get $node ss_plugin_opts)
						[ -n "$opts" ] && plugin_opts="--plugin-opts $opts"
						plugin_params="--plugin $plugin $plugin_opts"
					}
				fi
				for k in $(seq 1 $process); do
					ln_start_bin $(find_bin ss-redir) ss-redir_tcp_$5 "-c $config_file $plugin_params"
				done
			elif [ "$type" == "brook" ]; then
				local server_ip=$server_host
				local protocol=$(config_n_get $node brook_protocol client)
				local brook_tls=$(config_n_get $node brook_tls 0)
				if [ "$protocol" == "wsclient" ]; then
					[ "$brook_tls" == "1" ] && server_ip="wss://${server_ip}" || server_ip="ws://${server_ip}" 
					socks_port=$(get_new_port $(expr $SOCKS_REDIR_PORT3 + 3) tcp)
					ln_start_bin $(config_t_get global_app brook_file $(find_bin brook)) brook_tcp_$5 "wsclient -l 127.0.0.1:$socks_port -i 127.0.0.1 -s $server_ip:$port -p $(config_n_get $node password)"
					eval port=\$TCP_REDIR_PORT$5
					ln_start_bin $(find_bin ipt2socks) ipt2socks_tcp_$5 "-4 -T -l $port -b 0.0.0.0 -s 127.0.0.1 -p $socks5_port -R"
					echolog "Brook的WebSocket不支持透明代理，将使用ipt2socks转换透明代理！"
				else
					[ "$kcptun_use" == "1" ] && {
						server_ip=127.0.0.1
						port=$KCPTUN_REDIR_PORT
					}
					ln_start_bin $(config_t_get global_app brook_file $(find_bin brook)) brook_tcp_$5 "tproxy -l 0.0.0.0:$local_port -s $server_ip:$port -p $(config_n_get $node password)"
				fi
			fi
		fi
	fi
	return 0
}

node_switch() {
	local i=$3
	local node=$4
	[ -n "$1" -a -n "$2" -a -n "$3" -a -n "$4" ] && {
		ps -w | grep -E "$TMP_PATH" | grep -i "${1}_${i}" | grep -v "grep" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &
		local config_file=$TMP_PATH/${1}_${i}.json
		eval current_port=\$${1}_REDIR_PORT${i}
		local port=$(cat $TMP_PORT_PATH/${1}_${i})
		gen_start_config $node $port $1 $config_file $i
		echo $node > $TMP_ID_PATH/${1}_${i}
		local node_net=$(echo $1 | tr 'A-Z' 'a-z')
		uci set $CONFIG.@global[0].${node_net}_node${i}=$node
		uci commit $CONFIG
		/etc/init.d/dnsmasq restart >/dev/null 2>&1
	}
}

start_redir() {
	eval num=\$${1}_NODE_NUM
	for i in $(seq 1 $num); do
		eval node=\$${1}_NODE$i
		[ "$node" != "nil" ] && {
			TYPE=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
			local config_file=$TMP_PATH/${1}_${i}.json
			eval current_port=\$${1}_REDIR_PORT$i
			local port=$(echo $(get_new_port $current_port $2))
			eval ${1}_REDIR${i}=$port
			gen_start_config $node $port $1 $config_file $i
			#eval ip=\$${1}_NODE${i}_IP
			echo $node > $TMP_ID_PATH/${1}_${i}
			echo $port > $TMP_PORT_PATH/${1}_${i}
		}
	done
}

clean_log() {
	logsnum=$(cat $LOG_FILE 2>/dev/null | wc -l)
	[ "$logsnum" -gt 300 ] && {
		echo "" > $LOG_FILE
		echolog "日志文件过长，清空处理！"
	}
}

start_crontab() {
	sed -i '/$CONFIG/d' /etc/crontabs/root >/dev/null 2>&1 &
	auto_on=$(config_t_get global_delay auto_on 0)
	if [ "$auto_on" = "1" ]; then
		time_off=$(config_t_get global_delay time_off)
		time_on=$(config_t_get global_delay time_on)
		time_restart=$(config_t_get global_delay time_restart)
		[ -z "$time_off" -o "$time_off" != "nil" ] && {
			echo "0 $time_off * * * /etc/init.d/$CONFIG stop" >>/etc/crontabs/root
			echolog "配置定时任务：每天 $time_off 点关闭服务。"
		}
		[ -z "$time_on" -o "$time_on" != "nil" ] && {
			echo "0 $time_on * * * /etc/init.d/$CONFIG start" >>/etc/crontabs/root
			echolog "配置定时任务：每天 $time_on 点开启服务。"
		}
		[ -z "$time_restart" -o "$time_restart" != "nil" ] && {
			echo "0 $time_restart * * * /etc/init.d/$CONFIG restart" >>/etc/crontabs/root
			echolog "配置定时任务：每天 $time_restart 点重启服务。"
		}
	fi

	autoupdate=$(config_t_get global_rules auto_update)
	weekupdate=$(config_t_get global_rules week_update)
	dayupdate=$(config_t_get global_rules time_update)
	if [ "$autoupdate" = "1" ]; then
		local t="0 $dayupdate * * $weekupdate"
		[ "$weekupdate" = "7" ] && t="0 $dayupdate * * *"
		echo "$t lua $APP_PATH/rule_update.lua nil log > /dev/null 2>&1 &" >>/etc/crontabs/root
		echolog "配置定时任务：自动更新规则。"
	fi

	autoupdatesubscribe=$(config_t_get global_subscribe auto_update_subscribe)
	weekupdatesubscribe=$(config_t_get global_subscribe week_update_subscribe)
	dayupdatesubscribe=$(config_t_get global_subscribe time_update_subscribe)
	if [ "$autoupdatesubscribe" = "1" ]; then
		local t="0 $dayupdatesubscribe * * $weekupdatesubscribe"
		[ "$weekupdatesubscribe" = "7" ] && t="0 $dayupdatesubscribe * * *"
		echo "$t lua $APP_PATH/subscribe.lua start log > /dev/null 2>&1 &" >>/etc/crontabs/root
		echolog "配置定时任务：自动更新节点订阅。"
	fi
	
	start_daemon=$(config_t_get global_delay start_daemon 0)
	[ "$start_daemon" = "1" ] && $APP_PATH/monitor.sh > /dev/null 2>&1 &
	
	AUTO_SWITCH_ENABLE=$(config_t_get auto_switch enable 0)
	[ "$AUTO_SWITCH_ENABLE" = "1" ] && $APP_PATH/test.sh > /dev/null 2>&1 &
	
	/etc/init.d/cron restart
}

stop_crontab() {
	sed -i "/$CONFIG/d" /etc/crontabs/root >/dev/null 2>&1 &
	ps | grep "$APP_PATH/test.sh" | grep -v "grep" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &
	/etc/init.d/cron restart
	#echolog "清除定时执行命令。"
}

start_dns() {
	case "$DNS_MODE" in
	nonuse)
		echolog "DNS：不使用，将会直接使用上级DNS！"
	;;
	local_7913)
		echolog "DNS：使用本机7913端口DNS服务器解析域名..."
	;;
	dns2socks)
		if [ -n "$SOCKS_NODE1" -a "$SOCKS_NODE1" != "nil" ]; then
			DNS2SOCKS_FORWARD=$(config_t_get global dns2socks_forward 8.8.4.4)
			ln_start_bin $(find_bin dns2socks) dns2socks "127.0.0.1:$SOCKS_REDIR_PORT1 $DNS2SOCKS_FORWARD 127.0.0.1:$DNS_PORT"
			echolog "DNS：dns2socks..."
		else
			echolog "DNS：dns2socks模式需要使用Socks代理节点，请开启！"
			force_stop
		fi
	;;
	pdnsd)
		if [ -z "$TCP_NODE1" -o "$TCP_NODE1" == "nil" ]; then
			echolog "DNS：pdnsd 模式需要启用TCP节点！"
			force_stop
		else
			gen_pdnsd_config $DNS_PORT 2048
			DNS_FORWARD=$(echo $DNS_FORWARD | sed 's/,/ /g')
			ln_start_bin $(find_bin pdnsd) pdnsd "--daemon -c $pdnsd_dir/pdnsd.conf -d"
			echolog "DNS：pdnsd + 使用TCP节点解析DNS（$DNS_FORWARD）..."
		fi
	;;
	chinadns-ng)
		other_port=$(expr $DNS_PORT + 1)
		[ -f "$RULES_PATH/gfwlist.conf" ] && cat $RULES_PATH/gfwlist.conf | sort | uniq | sed -e '/127.0.0.1/d' | sed 's/ipset=\/.//g' | sed 's/\/gfwlist//g' > $TMP_PATH/gfwlist.txt
		[ -f "$TMP_PATH/gfwlist.txt" ] && {
			[ -f "$RULES_PATH/blacklist_host" -a -s "$RULES_PATH/blacklist_host" ] && cat $RULES_PATH/blacklist_host >> $TMP_PATH/gfwlist.txt
			local gfwlist_param="-g $TMP_PATH/gfwlist.txt"
		}
		[ -f "$RULES_PATH/chnlist" ] && cp -a $RULES_PATH/chnlist $TMP_PATH/chnlist
		[ -f "$TMP_PATH/chnlist" ] && {
			[ -f "$RULES_PATH/whitelist_host" -a -s "$RULES_PATH/whitelist_host" ] && cat $RULES_PATH/whitelist_host >> $TMP_PATH/chnlist
			local chnlist_param="-m $TMP_PATH/chnlist -M -f"
		}
		
		up_trust_chinadns_ng_dns=$(config_t_get global up_trust_chinadns_ng_dns "pdnsd")
		if [ "$up_trust_chinadns_ng_dns" == "pdnsd" ]; then
			if [ -z "$TCP_NODE1" -o "$TCP_NODE1" == "nil" ]; then
				echolog "DNS：ChinaDNS-NG + pdnsd 模式需要启用TCP节点！"
				force_stop
			else
				gen_pdnsd_config $other_port 0
				DNS_FORWARD=$(echo $DNS_FORWARD | sed 's/,/ /g')
				ln_start_bin $(find_bin pdnsd) pdnsd "--daemon -c $pdnsd_dir/pdnsd.conf -d"
				ln_start_bin $(find_bin chinadns-ng) chinadns-ng "-l $DNS_PORT -c $UP_CHINA_DNS -t 127.0.0.1#$other_port $gfwlist_param $chnlist_param"
				echolog "DNS：ChinaDNS-NG + pdnsd($DNS_FORWARD)，国内DNS：$UP_CHINA_DNS"
			fi
		elif [ "$up_trust_chinadns_ng_dns" == "dns2socks" ]; then
			if [ -n "$SOCKS_NODE1" -a "$SOCKS_NODE1" != "nil" ]; then
				DNS2SOCKS_FORWARD=$(config_t_get global dns2socks_forward 8.8.4.4)
				ln_start_bin $(find_bin dns2socks) dns2socks "127.0.0.1:$SOCKS_REDIR_PORT1 $DNS2SOCKS_FORWARD 127.0.0.1:$other_port"
				ln_start_bin $(find_bin chinadns-ng) chinadns-ng "-l $DNS_PORT -c $UP_CHINA_DNS -t 127.0.0.1#$other_port $gfwlist_param $chnlist_param"
				echolog "DNS：ChinaDNS-NG + dns2socks($DNS2SOCKS_FORWARD)，国内DNS：$UP_CHINA_DNS"
			else
				echolog "DNS：dns2socks模式需要使用Socks代理节点，请开启！"
				force_stop
			fi
		else
			use_udp_node_resolve_dns=1
			DNS_FORWARD=$(echo $up_trust_chinadns_ng_dns | sed 's/,/ /g')
			ln_start_bin $(find_bin chinadns-ng) chinadns-ng "-l $DNS_PORT -c $UP_CHINA_DNS -t $up_trust_chinadns_ng_dns $gfwlist_param $chnlist_param"
			echolog "DNS：ChinaDNS-NG，国内DNS：$UP_CHINA_DNS，可信DNS：$up_trust_chinadns_ng_dns，如果不能使用，请确保UDP节点已打开并且支持UDP转发。"
		fi
	;;
	esac
}

add_dnsmasq() {
	mkdir -p $TMP_DNSMASQ_PATH $DNSMASQ_PATH /var/dnsmasq.d
	local adblock=$(config_t_get global_rules adblock 0)
	local chinadns_mode=0
	[ "$DNS_MODE" == "chinadns-ng" ] && [ "$IS_DEFAULT_CHINA_DNS" != 1 ] && chinadns_mode=1
	[ "$adblock" == "1" ] && {
		[ -f "$RULES_PATH/adblock.conf" -a -s "$RULES_PATH/adblock.conf" ] && ln -s $RULES_PATH/adblock.conf $TMP_DNSMASQ_PATH/adblock.conf
	}
	
	[ "$DNS_MODE" != "nonuse" ] && {
		[ -f "$RULES_PATH/whitelist_host" -a -s "$RULES_PATH/whitelist_host" ] && cat $RULES_PATH/whitelist_host | sed -e "/^$/d" | sort -u | awk '{if (mode == 0 && dns1 != "") print "server=/."$1"/'$UP_CHINA_DNS1'"; if (mode == 0 && dns2 != "") print "server=/."$1"/'$UP_CHINA_DNS2'"; print "ipset=/."$1"/whitelist"}' mode=$chinadns_mode dns1=$UP_CHINA_DNS1 dns2=$UP_CHINA_DNS2 > $TMP_DNSMASQ_PATH/whitelist_host.conf
		uci show $CONFIG | grep "@nodes" | grep "address" | cut -d "'" -f 2 | sed 's/^\(https:\/\/\|http:\/\/\)//g' | awk -F '/' '{print $1}' | grep -v "google.c" | grep -E '.*\..*$' | grep '[a-zA-Z]$' | sort -u | awk '{if (dns1 != "") print "server=/."$1"/'$UP_CHINA_DNS1'"; if (dns2 != "") print "server=/."$1"/'$UP_CHINA_DNS2'"; print "ipset=/."$1"/vpsiplist"}' dns1=$UP_CHINA_DNS1 dns2=$UP_CHINA_DNS2 > $TMP_DNSMASQ_PATH/vpsiplist_host.conf
		[ -f "$RULES_PATH/blacklist_host" -a -s "$RULES_PATH/blacklist_host" ] && cat $RULES_PATH/blacklist_host | sed -e "/^$/d" | sort -u | awk '{if (mode == 0) print "server=/."$1"/127.0.0.1#'$DNS_PORT'"; print "ipset=/."$1"/blacklist"}' mode=$chinadns_mode > $TMP_DNSMASQ_PATH/blacklist_host.conf
		if [ "$chinadns_mode" == 0 ]; then
			[ -f "$RULES_PATH/gfwlist.conf" -a -s "$RULES_PATH/gfwlist.conf" ] && ln -s $RULES_PATH/gfwlist.conf $TMP_DNSMASQ_PATH/gfwlist.conf
		else
			cat $TMP_PATH/gfwlist.txt | sed -e "/^$/d" | sort -u | awk '{print "ipset=/."$1"/gfwlist"}' > $TMP_DNSMASQ_PATH/gfwlist.conf
		fi
		
		subscribe_proxy=$(config_t_get global_subscribe subscribe_proxy 0)
		[ "$subscribe_proxy" -eq 1 ] && {
			local count=$(uci show $CONFIG | grep "@subscribe_list" | sed -n '$p' | cut -d '[' -f 2 | cut -d ']' -f 1)
			[ -n "$count" ] && [ "$count" -ge 0 ] && {
				u_get() {
					local ret=$(uci -q get $CONFIG.@subscribe_list[$1].$2)
					echo ${ret:=$3}
				}
				for i in $(seq 0 $count); do
					local enabled=$(u_get $i enabled 0)
					[ "$enabled" == "0" ] && continue
					local url=$(u_get $i url)
					[ -n "$url" -a "$url" != "" ] && {
						if [ -n "$(echo -n "$url" | grep "//")" ]; then
							[ "$chinadns_mode" == 0 ] && echo -n "$url" | awk -F '/' '{print $3}' | sed "s/^/server=&\/./g" | sed "s/$/\/127.0.0.1#$DNS_PORT/g" >>$TMP_DNSMASQ_PATH/subscribe.conf
							echo -n "$url" | awk -F '/' '{print $3}' | sed "s/^/ipset=&\/./g" | sed "s/$/\/blacklist/g" >>$TMP_DNSMASQ_PATH/subscribe.conf
						else
							[ "$chinadns_mode" == 0 ] && echo -n "$url" | awk -F '/' '{print $1}' | sed "s/^/server=&\/./g" | sed "s/$/\/127.0.0.1#$DNS_PORT/g" >>$TMP_DNSMASQ_PATH/subscribe.conf
							echo -n "$url" | awk -F '/' '{print $1}' | sed "s/^/ipset=&\/./g" | sed "s/$/\/blacklist/g" >>$TMP_DNSMASQ_PATH/subscribe.conf
						fi
					}
				done
			}
		}
	}
	
	if [ -z "$IS_DEFAULT_CHINA_DNS" -o "$IS_DEFAULT_CHINA_DNS" == 0 ]; then
		server="server=127.0.0.1#$DNS_PORT"
		[ "$DNS_MODE" != "chinadns-ng" ] && {
			[ -n "$UP_CHINA_DNS1" ] && server="server=$UP_CHINA_DNS1"
			[ -n "$UP_CHINA_DNS2" ] && server="${server}\nserver=${UP_CHINA_DNS2}"
		}
		cat <<-EOF > /var/dnsmasq.d/dnsmasq-$CONFIG.conf
			$(echo -e $server)
			all-servers
			no-poll
			no-resolv
		EOF
	else
		# 防呆
		[ -z "$DEFAULT_DNS1" ] && {
			local tmp=$(get_host_ip ipv4 www.baidu.com 1)
			[ -z "$tmp" ] && {
				cat <<-EOF > /var/dnsmasq.d/dnsmasq-$CONFIG.conf
					server=$UP_CHINA_DNS1
					no-poll
					no-resolv
				EOF
				echolog "你没有设置接口DNS，请前往设置！"
				/etc/init.d/dnsmasq restart >/dev/null 2>&1
			}
		}
	fi
	
	echo "conf-dir=$TMP_DNSMASQ_PATH" >> /var/dnsmasq.d/dnsmasq-$CONFIG.conf
	cp -rf /var/dnsmasq.d/dnsmasq-$CONFIG.conf $DNSMASQ_PATH/dnsmasq-$CONFIG.conf
	echolog "dnsmasq：生成配置文件。"
}

gen_pdnsd_config() {
	pdnsd_dir=$TMP_PATH/pdnsd
	mkdir -p $pdnsd_dir
	touch $pdnsd_dir/pdnsd.cache
	chown -R root.nogroup $pdnsd_dir
	cat > $pdnsd_dir/pdnsd.conf <<-EOF
		global {
			perm_cache = $2;
			cache_dir = "$pdnsd_dir";
			run_as = "root";
			server_ip = 127.0.0.1;
			server_port = $1;
			status_ctl = on;
			query_method = tcp_only;
			min_ttl = 1h;
			max_ttl = 1w;
			timeout = 10;
			par_queries = 1;
			neg_domain_pol = on;
			udpbufsize = 1024;
			proc_limit = 2;
			procq_limit = 8;
		}
		
	EOF
			
	cat >> $pdnsd_dir/pdnsd.conf <<-EOF
		server {
			label = "node";
			ip = $DNS_FORWARD;
			edns_query = on;
			port = 53;
			timeout = 4;
			interval = 10m;
			uptest = none;
			purge_cache = off;
		}
		
	EOF
	
	use_tcp_node_resolve_dns=1
}

del_dnsmasq() {
	rm -rf /var/dnsmasq.d/dnsmasq-$CONFIG.conf
	rm -rf $DNSMASQ_PATH/dnsmasq-$CONFIG.conf
	rm -rf $TMP_DNSMASQ_PATH
}

start_haproxy() {
	enabled=$(config_t_get global_haproxy balancing_enable 0)
	[ "$enabled" = "1" ] && {
		haproxy_bin=$(find_bin haproxy)
		[ -f "$haproxy_bin" ] && {
			local HAPROXY_PATH=$TMP_PATH/haproxy
			mkdir -p $HAPROXY_PATH
			local HAPROXY_FILE=$HAPROXY_PATH/config.cfg
			bport=$(config_t_get global_haproxy haproxy_port)
			cat <<-EOF > $HAPROXY_FILE
				global
				    log         127.0.0.1 local2
				    chroot      /usr/bin
				    maxconn     60000
				    stats socket  $HAPROXY_PATH/haproxy.sock
				    user        root
				    daemon
					
				defaults
				    mode                    tcp
				    log                     global
				    option                  tcplog
				    option                  dontlognull
				    option http-server-close
				    #option forwardfor       except 127.0.0.0/8
				    option                  redispatch
				    retries                 2
				    timeout http-request    10s
				    timeout queue           1m
				    timeout connect         10s
				    timeout client          1m
				    timeout server          1m
				    timeout http-keep-alive 10s
				    timeout check           10s
				    maxconn                 3000
					
			EOF
			
			local ports=$(uci show $CONFIG | grep "@haproxy_config" | grep haproxy_port | cut -d "'" -f 2 | sort -u)
			for p in $ports; do
				cat <<-EOF >> $HAPROXY_FILE
					listen $p
					    mode tcp
					    bind 0.0.0.0:$p
						
				EOF
			done
			
			local count=$(uci show $CONFIG | grep "@haproxy_config" | sed -n '$p' | cut -d '[' -f 2 | cut -d ']' -f 1)
			[ -n "$count" ] && [ "$count" -ge 0 ] && {
				u_get() {
					local ret=$(uci -q get $CONFIG.@haproxy_config[$1].$2)
					echo ${ret:=$3}
				}
				for i in $(seq 0 $count); do
					local enabled=$(u_get $i enabled 0)
					[ -z "$enabled" -o "$enabled" == "0" ] && continue
					
					local haproxy_port=$(u_get $i haproxy_port)
					[ -z "$haproxy_port" ] && continue
					
					local bips=$(u_get $i lbss)
					local bports=$(u_get $i lbort)
					if [ -z "$bips" ] || [ -z "$bports" ]; then
						continue
					fi
					
					local bip=$(echo $bips | awk -F ":" '{print $1}')
					local bport=$(echo $bips | awk -F ":" '{print $2}')
					[ "$bports" != "default" ] && bport=$bports
					[ -z "$bport" ] && continue
					
					local line=$(cat $HAPROXY_FILE | grep -n "bind 0.0.0.0:$haproxy_port" | awk -F ":" '{print $1}')
					[ -z "$line" ] && continue
					
					local bweight=$(u_get $i lbweight)
					local exports=$(u_get $i export)
					local backup=$(u_get $i backup)
					local bbackup=""
					[ "$backup" = "1" ] && bbackup="backup"
					sed -i "${line}i \ \ \ \ server $bip:$bport $bip:$bport weight $bweight check inter 1500 rise 1 fall 3 $bbackup" $HAPROXY_FILE
					if [ "$exports" != "0" ]; then
						failcount=0
						while [ "$failcount" -lt "3" ]; do
							interface=$(ifconfig | grep "$exports" | awk '{print $1}')
							if [ -z "$interface" ]; then
								echolog "找不到出口接口：$exports，1分钟后再重试"
								let "failcount++"
								[ "$failcount" -ge 3 ] && exit 0
								sleep 1m
							else
								route add -host ${bip} dev ${exports}
								echo "$bip" >>/tmp/balancing_ip
								break
							fi
						done
					fi
				done
			}
			
			# 控制台配置
			local console_port=$(config_t_get global_haproxy console_port)
			local console_user=$(config_t_get global_haproxy console_user)
			local console_password=$(config_t_get global_haproxy console_password)
			local auth=""
			[ -n "$console_user" -a -n "console_password" ] && auth="stats auth $console_user:$console_password"
			cat <<-EOF >> $HAPROXY_FILE
				listen console
				    bind 0.0.0.0:$console_port
				    mode http                   
				    stats refresh 30s
				    stats uri /
				    stats admin if TRUE
				    $auth
			EOF
			
			ln_start_bin $haproxy_bin haproxy "-f $HAPROXY_FILE"
		}
	}
}

kill_all() {
	kill -9 $(pidof $@) >/dev/null 2>&1 &
}

force_stop() {
	stop
	exit 0
}

boot() {
	[ "$ENABLED" == 1 ] && {
		local delay=$(config_t_get global_delay start_delay 1)
		if [ "$delay" -gt 0 ]; then
			echolog "执行启动延时 $delay 秒后再启动!"
			sleep $delay && start >/dev/null 2>&1 &
		else
			start
		fi
	}
	return 0
}

start() {
	! load_config && return 1
	start_haproxy
	start_redir SOCKS tcp
	start_redir TCP tcp
	start_redir UDP udp
	start_dns
	add_dnsmasq
	source $APP_PATH/iptables.sh start
	/etc/init.d/dnsmasq restart >/dev/null 2>&1
	start_crontab
	echolog "运行完成！\n"
}

stop() {
	clean_log
	source $APP_PATH/iptables.sh stop
	kill_all v2ray-plugin obfs-local
	ps -w | grep -v "grep" | grep $CONFIG/test.sh | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &
	ps -w | grep -v "grep" | grep $CONFIG/monitor.sh | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &
	ps -w | grep -v "grep" | grep -E "$TMP_PATH" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &
	ps -w | grep -v "grep" | grep "sleep 1m" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &
	rm -rf $TMP_DNSMASQ_PATH $TMP_PATH
	stop_crontab
	del_dnsmasq
	/etc/init.d/dnsmasq restart >/dev/null 2>&1
	echolog "清空并关闭相关程序和缓存完成。"
}

case $1 in
get_new_port)
	get_new_port $2 $3
	;;
gen_start_config)
	gen_start_config $2 $3 $4 $5 $6 $7
	;;
node_switch)
	node_switch $2 $3 $4 $5
	;;
stop)
	[ -n "$2" -a "$2" == "force" ] && force_stop
	stop
	;;
start)
	start
	;;
boot)
	boot
	;;
esac