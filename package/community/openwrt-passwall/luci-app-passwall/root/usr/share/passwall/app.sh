#!/bin/sh
# Copyright (C) 2018-2020 L-WRT Team
# Copyright (C) 2021 xiaorouji

. $IPKG_INSTROOT/lib/functions.sh
. $IPKG_INSTROOT/lib/functions/service.sh

CONFIG=passwall
TMP_PATH=/var/etc/$CONFIG
TMP_BIN_PATH=$TMP_PATH/bin
TMP_ID_PATH=$TMP_PATH/id
TMP_PORT_PATH=$TMP_PATH/port
TMP_ROUTE_PATH=$TMP_PATH/route
TMP_ACL_PATH=$TMP_PATH/acl
DNSMASQ_PATH=/etc/dnsmasq.d
TMP_DNSMASQ_PATH=/var/etc/dnsmasq-passwall.d
LOG_FILE=/var/log/$CONFIG.log
APP_PATH=/usr/share/$CONFIG
RULES_PATH=/usr/share/${CONFIG}/rules
DNS_N=dnsmasq
DNS_PORT=7913
TUN_DNS="127.0.0.1#${DNS_PORT}"
LOCAL_DNS=119.29.29.29
DEFAULT_DNS=
NO_PROXY=0
PROXY_IPV6=0
use_tcp_node_resolve_dns=0
use_udp_node_resolve_dns=0
LUA_API_PATH=/usr/lib/lua/luci/model/cbi/$CONFIG/api
API_GEN_SS=$LUA_API_PATH/gen_shadowsocks.lua
API_GEN_V2RAY=$LUA_API_PATH/gen_v2ray.lua
API_GEN_V2RAY_PROTO=$LUA_API_PATH/gen_v2ray_proto.lua
API_GEN_TROJAN=$LUA_API_PATH/gen_trojan.lua
API_GEN_NAIVE=$LUA_API_PATH/gen_naiveproxy.lua
API_GEN_HYSTERIA=$LUA_API_PATH/gen_hysteria.lua

echolog() {
	local d="$(date "+%Y-%m-%d %H:%M:%S")"
	echo -e "$d: $*" >>$LOG_FILE
}

config_get_type() {
	local ret=$(uci -q get "${CONFIG}.${1}" 2>/dev/null)
	echo "${ret:=$2}"
}

config_n_get() {
	local ret=$(uci -q get "${CONFIG}.${1}.${2}" 2>/dev/null)
	echo "${ret:=$3}"
}

config_t_get() {
	local index=${4:-0}
	local ret=$(uci -q get "${CONFIG}.@${1}[${index}].${2}" 2>/dev/null)
	echo "${ret:=${3}}"
}

get_enabled_anonymous_secs() {
	uci -q show "${CONFIG}" | grep "${1}\[.*\.enabled='1'" | cut -d '.' -sf2
}

get_host_ip() {
	local host=$2
	local count=$3
	[ -z "$count" ] && count=3
	local isip=""
	local ip=$host
	if [ "$1" == "ipv6" ]; then
		isip=$(echo $host | grep -E "([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}")
		if [ -n "$isip" ]; then
			isip=$(echo $host | cut -d '[' -f2 | cut -d ']' -f1)
		else
			isip=$(echo $host | grep -E "([0-9]{1,3}[\.]){3}[0-9]{1,3}")
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

get_ip_port_from() {
	local __host=${1}; shift 1
	local __ipv=${1}; shift 1
	local __portv=${1}; shift 1
	local __ucipriority=${1}; shift 1

	local val1 val2
	if [ -n "${__ucipriority}" ]; then
		val2=$(config_n_get ${__host} port $(echo $__host | sed -n 's/^.*[:#]\([0-9]*\)$/\1/p'))
		val1=$(config_n_get ${__host} address "${__host%%${val2:+[:#]${val2}*}}")
	else
		val2=$(echo $__host | sed -n 's/^.*[:#]\([0-9]*\)$/\1/p')
		val1="${__host%%${val2:+[:#]${val2}*}}"
	fi
	eval "${__ipv}=\"$val1\"; ${__portv}=\"$val2\""
}

host_from_url(){
	local f=${1}

	## Remove protocol part of url  ##
	f="${f##http://}"
	f="${f##https://}"
	f="${f##ftp://}"
	f="${f##sftp://}"

	## Remove username and/or username:password part of URL  ##
	f="${f##*:*@}"
	f="${f##*@}"

	## Remove rest of urls ##
	f="${f%%/*}"
	echo "${f%%:*}"
}

hosts_foreach() {
	local __hosts
	eval "__hosts=\$${1}"; shift 1
	local __func=${1}; shift 1
	local __default_port=${1}; shift 1
	local __ret=1

	[ -z "${__hosts}" ] && return 0
	local __ip __port
	for __host in $(echo $__hosts | sed 's/[ ,]/\n/g'); do
		get_ip_port_from "$__host" "__ip" "__port"
		eval "$__func \"${__host}\" \"\${__ip}\" \"\${__port:-${__default_port}}\" \"$@\""
		__ret=$?
		[ ${__ret} -ge ${ERROR_NO_CATCH:-1} ] && return ${__ret}
	done
}

check_host() {
	local f=${1}
	a=$(echo $f | grep "\/")
	[ -n "$a" ] && return 1
	# 判断是否包含汉字~
	local tmp=$(echo -n $f | awk '{print gensub(/[!-~]/,"","g",$0)}')
	[ -n "$tmp" ] && return 1
	return 0
}

get_first_dns() {
	local __hosts_val=${1}; shift 1
	__first() {
		[ -z "${2}" ] && return 0
		echo "${2}#${3}"
		return 1
	}
	eval "hosts_foreach \"${__hosts_val}\" __first \"$@\""
}

get_last_dns() {
	local __hosts_val=${1}; shift 1
	local __first __last
	__every() {
		[ -z "${2}" ] && return 0
		__last="${2}#${3}"
		__first=${__first:-${__last}}
	}
	eval "hosts_foreach \"${__hosts_val}\" __every \"$@\""
	[ "${__first}" ==  "${__last}" ] || echo "${__last}"
}

check_port_exists() {
	port=$1
	protocol=$2
	[ -n "$protocol" ] || protocol="tcp,udp"
	result=
	if [ "$protocol" = "tcp" ]; then
		result=$(netstat -tln | grep -c ":$port ")
	elif [ "$protocol" = "udp" ]; then
		result=$(netstat -uln | grep -c ":$port ")
	elif [ "$protocol" = "tcp,udp" ]; then
		result=$(netstat -tuln | grep -c ":$port ")
	fi
	echo "${result}"
}

get_new_port() {
	port=$1
	[ "$port" == "auto" ] && port=2082
	protocol=$(echo $2 | tr 'A-Z' 'a-z')
	result=$(check_port_exists $port $protocol)
	if [ "$result" != 0 ]; then
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

first_type() {
	local path_name=${1}
	type -t -p "/bin/${path_name}" -p "${TMP_BIN_PATH}/${path_name}" -p "${path_name}" "$@" | head -n1
}

eval_set_val() {
	for i in $@; do
		for j in $i; do
			eval $j
		done
	done
}

eval_unset_val() {
	for i in $@; do
		for j in $i; do
			eval unset j
		done
	done
}

ln_start_bin() {
	local file_func=${1}
	local ln_name=${2}
	local output=${3}

	shift 3;
	if [  "${file_func%%/*}" != "${file_func}" ]; then
		[ ! -L "${file_func}" ] && {
			ln -s "${file_func}" "${TMP_BIN_PATH}/${ln_name}" >/dev/null 2>&1
			file_func="${TMP_BIN_PATH}/${ln_name}"
		}
		[ -x "${file_func}" ] || echolog "  - $(readlink ${file_func}) 没有执行权限，无法启动：${file_func} $*"
	fi
	#echo "${file_func} $*" >&2
	[ -n "${file_func}" ] || echolog "  - 找不到 ${ln_name}，无法启动..."
	${file_func:-echolog "  - ${ln_name}"} "$@" >${output} 2>&1 &
}

lua_api() {
	local func=${1}
	[ -z "${func}" ] && {
		echo "nil"
		return
	}
	echo $(lua -e "local api = require 'luci.model.cbi.passwall.api.api' print(api.${func})")
}

# 必用变量
ENABLED=$(config_t_get global enabled 0)
SOCKS_ENABLED=$(config_t_get global socks_enabled 0)
TCP_REDIR_PORT=1041
TCP_NODE=$(config_t_get global tcp_node nil)
UDP_REDIR_PORT=1051
UDP_NODE=$(config_t_get global udp_node nil)
TCP_UDP=0
[ "$UDP_NODE" == "tcp" ] && {
	UDP_NODE=$TCP_NODE
	TCP_UDP=1
}
[ "$ENABLED" != 1 ] && NO_PROXY=1
[ "$TCP_NODE" == "nil" -a "$UDP_NODE" == "nil" ] && NO_PROXY=1
[ "$(config_get_type $TCP_NODE nil)" == "nil" -a "$(config_get_type $UDP_NODE nil)" == "nil" ] && NO_PROXY=1
tcp_proxy_way=$(config_t_get global_forwarding tcp_proxy_way redirect)
REDIRECT_LIST="socks ss ss-rust ssr v2ray xray trojan-plus trojan-go naiveproxy"
TPROXY_LIST="socks ss ss-rust ssr v2ray xray trojan-plus brook trojan-go hysteria"
KCPTUN_REDIR_PORT=$(config_t_get global_forwarding kcptun_port 12948)
RESOLVFILE=/tmp/resolv.conf.d/resolv.conf.auto
[ -f "${RESOLVFILE}" ] && [ -s "${RESOLVFILE}" ] || RESOLVFILE=/tmp/resolv.conf.auto

load_config() {
	TCP_REDIR_PORTS=$(config_t_get global_forwarding tcp_redir_ports '80,443')
	UDP_REDIR_PORTS=$(config_t_get global_forwarding udp_redir_ports '1:65535')
	TCP_NO_REDIR_PORTS=$(config_t_get global_forwarding tcp_no_redir_ports 'disable')
	UDP_NO_REDIR_PORTS=$(config_t_get global_forwarding udp_no_redir_ports 'disable')
	TCP_PROXY_MODE=$(config_t_get global tcp_proxy_mode chnroute)
	UDP_PROXY_MODE=$(config_t_get global udp_proxy_mode chnroute)
	LOCALHOST_TCP_PROXY_MODE=$(config_t_get global localhost_tcp_proxy_mode default)
	LOCALHOST_UDP_PROXY_MODE=$(config_t_get global localhost_udp_proxy_mode default)
	[ "$LOCALHOST_TCP_PROXY_MODE" == "default" ] && LOCALHOST_TCP_PROXY_MODE=$TCP_PROXY_MODE
	[ "$LOCALHOST_UDP_PROXY_MODE" == "default" ] && LOCALHOST_UDP_PROXY_MODE=$UDP_PROXY_MODE
	global=$(echo "${TCP_PROXY_MODE}${LOCALHOST_TCP_PROXY_MODE}${UDP_PROXY_MODE}${LOCALHOST_UDP_PROXY_MODE}" | grep "global")
	returnhome=$(echo "${TCP_PROXY_MODE}${LOCALHOST_TCP_PROXY_MODE}${UDP_PROXY_MODE}${LOCALHOST_UDP_PROXY_MODE}" | grep "returnhome")
	chnlist=$(echo "${TCP_PROXY_MODE}${LOCALHOST_TCP_PROXY_MODE}${UDP_PROXY_MODE}${LOCALHOST_UDP_PROXY_MODE}" | grep "chnroute")
	gfwlist=$(echo "${TCP_PROXY_MODE}${LOCALHOST_TCP_PROXY_MODE}${UDP_PROXY_MODE}${LOCALHOST_UDP_PROXY_MODE}" | grep "gfwlist")
	DNS_MODE=$(config_t_get global dns_mode pdnsd)
	DNS_FORWARD=$(config_t_get global dns_forward 8.8.4.4:53 | sed 's/:/#/g')
	DNS_CACHE=$(config_t_get global dns_cache 0)
	CHINADNS_NG=$(config_t_get global chinadns_ng 1)
	
	DEFAULT_DNS=$(uci show dhcp | grep "@dnsmasq" | grep "\.server=" | awk -F '=' '{print $2}' | sed "s/'//g" | tr ' ' '\n' | grep -v "\/" | head -2 | sed ':label;N;s/\n/,/;b label')
	[ -z "${DEFAULT_DNS}" ] && DEFAULT_DNS=$(echo -n $(sed -n 's/^nameserver[ \t]*\([^ ]*\)$/\1/p' "${RESOLVFILE}" | grep -v -E "0.0.0.0|127.0.0.1|::" | head -2) | tr ' ' ',')
	LOCAL_DNS="${DEFAULT_DNS:-119.29.29.29}"
	
	export V2RAY_LOCATION_ASSET=$(config_t_get global_rules v2ray_location_asset "/usr/share/xray/")
	export XRAY_LOCATION_ASSET=$V2RAY_LOCATION_ASSET
	mkdir -p /var/etc $TMP_PATH $TMP_BIN_PATH $TMP_ID_PATH $TMP_PORT_PATH $TMP_ROUTE_PATH $TMP_ACL_PATH
}

run_ipt2socks() {
	local flag redir_type tcp_tproxy local_port socks_address socks_port socks_username socks_password log_file
	local _extra_param=""
	eval_set_val $@
	[ -n "$log_file" ] || log_file="/dev/null"
	[ -n "$socks_username" ] && [ -n "$socks_password" ] && _extra_param="${_extra_param} -a $socks_username -k $socks_password"
	[ -n "$tcp_tproxy" ] || _extra_param="${_extra_param} -R"
	case "$redir_type" in
	UDP)
		flag="${flag}_UDP"
		_extra_param="${_extra_param} -U"
	;;
	TCP)
		flag="${flag}_TCP"
		_extra_param="${_extra_param} -T"
	;;
	esac
	_extra_param="${_extra_param} -v"
	ln_start_bin "$(first_type ipt2socks)" "ipt2socks_${flag}" $log_file -l $local_port -b 0.0.0.0 -s $socks_address -p $socks_port ${_extra_param}
}

run_v2ray() {
	local flag node redir_type redir_port socks_address socks_port socks_username socks_password http_address http_port http_username http_password log_file config_file
	local _extra_param=""
	local proto="tcp,udp"
	eval_set_val $@
	local type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
	if [ "$type" != "v2ray" ] && [ "$type" != "xray" ]; then
		return 1
	fi
	[ -n "$log_file" ] || log_file="/dev/null"
	[ -n "$socks_username" ] && [ -n "$socks_password" ] && _extra_param="${_extra_param} -local_socks_username $socks_username -local_socks_password $socks_password"
	[ -n "$http_username" ] && [ -n "$http_password" ] && _extra_param="${_extra_param} -local_http_username $http_username -local_http_password $http_password"
	case "$redir_type" in
	UDP)
		flag="${flag}_UDP"
		proto="udp"
	;;
	TCP)
		flag="${flag}_TCP"
		proto="tcp"
	;;
	esac
	lua $API_GEN_V2RAY -node $node -proto $proto -redir_port $redir_port -local_socks_address $socks_address -local_socks_port $socks_port -local_http_address $http_address -local_http_port $http_port ${_extra_param} > $config_file
	ln_start_bin "$(first_type $(config_t_get global_app ${type}_file) ${type})" ${type} $log_file -config="$config_file"
}

run_dns2socks() {
	local flag socks socks_address socks_port socks_username socks_password listen_address listen_port dns cache log_file
	local _extra_param=""
	eval_set_val $@
	[ -n "$flag" ] && flag="_${flag}"
	[ -n "$log_file" ] || log_file="/dev/null"
	dns=$(get_first_dns dns 53 | sed 's/#/:/g')
	[ -n "$socks" ] && {
		socks=$(echo $socks | sed "s/#/:/g")
		socks_address=$(echo $socks | awk -F ':' '{print $1}')
		socks_port=$(echo $socks | awk -F ':' '{print $2}')
	}
	[ -n "$socks_username" ] && [ -n "$socks_password" ] && _extra_param="${_extra_param} /u $socks_username /p $socks_password"
	[ -z "$cache" ] && cache=1
	[ "$cache" = "0" ] && _extra_param="${_extra_param} /d"
	ln_start_bin "$(first_type dns2socks)" "dns2socks${flag}" $log_file ${_extra_param} "${socks_address}:${socks_port}" "${dns}" "${listen_address}:${listen_port}"
}

run_v2ray_doh_socks() {
	local bin=$(first_type $(config_t_get global_app v2ray_file) v2ray)
	if [ -n "$bin" ]; then
		type="v2ray"
	else
		bin=$(first_type $(config_t_get global_app xray_file) xray)
		[ -n "$bin" ] && type="xray"
	fi
	[ -z "$type" ] && return 1
	local flag socks_address socks_port socks_username socks_password listen_address listen_port doh_bootstrap doh_url doh_host log_file config_file
	eval_set_val $@
	[ -n "$log_file" ] || log_file="/dev/null"
	_doh_url=$(echo $doh | awk -F ',' '{print $1}')
	_doh_host_port=$(echo $_doh_url | sed "s/https:\/\///g" | awk -F '/' '{print $1}')
	_doh_host=$(echo $_doh_host_port | awk -F ':' '{print $1}')
	_doh_port=$(echo $_doh_host_port | awk -F ':' '{print $2}')
	_doh_bootstrap=$(echo $doh | cut -d ',' -sf 2-)
	lua $API_GEN_V2RAY -dns_listen_port "${listen_port}" -dns_server "${doh_bootstrap}" -doh_url "${doh_url}" -doh_host "${doh_host}" -dns_socks_address "${socks_address}" -dns_socks_port "${socks_port}" > $config_file
	ln_start_bin "$bin" $type $log_file -config="$config_file"
}

run_socks() {
	local flag node bind socks_port config_file http_port http_config_file relay_port log_file
	eval_set_val $@
	[ -n "$http_port" ] || http_port=0
	[ -n "$http_config_file" ] || http_config_file="nil"
	[ -n "$log_file" ] || log_file="/dev/null"
	local type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
	local remarks=$(config_n_get $node remarks)
	local server_host=$(config_n_get $node address)
	local port=$(config_n_get $node port)
	[ -n "$relay_port" ] && {
		server_host="127.0.0.1"
		port=$relay_port
	}
	local error_msg tmp

	if [ -n "$server_host" ] && [ -n "$port" ]; then
		check_host $server_host
		[ $? != 0 ] && {
			echolog "  - Socks节点：[$remarks]${server_host} 是非法的服务器地址，无法启动！"
			return 1
		}
		tmp="${server_host}:${port}"
	else
		error_msg="某种原因，此 Socks 服务的相关配置已失联，启动中止！"
	fi

	if ([ "$type" == "v2ray" ] || [ "$type" == "xray" ]) && ([ -n "$(config_n_get $node balancing_node)" ] || [ "$(config_n_get $node default_node)" != "_direct" -a "$(config_n_get $node default_node)" != "_blackhole" ]); then
		unset error_msg
	fi

	[ -n "${error_msg}" ] && {
		[ "$bind" != "127.0.0.1" ] && echolog "  - Socks节点：[$remarks]${tmp}，启动中止 ${bind}:${socks_port} ${error_msg}"
		return 1
	}
	[ "$bind" != "127.0.0.1" ] && echolog "  - Socks节点：[$remarks]${tmp}，启动 ${bind}:${socks_port}"

	case "$type" in
	socks)
		local bin=$(first_type $(config_t_get global_app v2ray_file) v2ray)
		if [ -n "$bin" ]; then
			type="v2ray"
		else
			bin=$(first_type $(config_t_get global_app xray_file) xray)
			[ -n "$bin" ] && type="xray"
		fi
		[ -z "$type" ] && return 1
		local _socks_address=$(config_n_get $node address)
		local _socks_port=$(config_n_get $node port)
		local _socks_username=$(config_n_get $node username)
		local _socks_password=$(config_n_get $node password)
		[ "$http_port" != "0" ] && {
			local _extra_param="-local_http_port $http_port"
			config_file=$(echo $config_file | sed "s/SOCKS/HTTP_SOCKS/g")
		}
		lua $API_GEN_V2RAY_PROTO -local_socks_port $socks_port ${_extra_param} -server_proto socks -server_address ${_socks_address} -server_port ${_socks_port} -server_username ${_socks_username} -server_password ${_socks_password} > $config_file
		ln_start_bin "$bin" $type $log_file -config="$config_file"
	;;
	v2ray|\
	xray)
		[ "$http_port" != "0" ] && {
			local _extra_param="-local_http_port $http_port"
			config_file=$(echo $config_file | sed "s/SOCKS/HTTP_SOCKS/g")
		}
		lua $API_GEN_V2RAY -node $node -local_socks_port $socks_port ${_extra_param} > $config_file
		ln_start_bin "$(first_type $(config_t_get global_app ${type}_file) ${type})" ${type} $log_file -config="$config_file"
	;;
	trojan-go)
		lua $API_GEN_TROJAN -node $node -run_type client -local_addr $bind -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_start_bin "$(first_type $(config_t_get global_app trojan_go_file) trojan-go)" trojan-go $log_file -config "$config_file"
	;;
	trojan*)
		lua $API_GEN_TROJAN -node $node -run_type client -local_addr $bind -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_start_bin "$(first_type ${type})" "${type}" $log_file -c "$config_file"
	;;
	naiveproxy)
		lua $API_GEN_NAIVE -node $node -run_type socks -local_addr $bind -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_start_bin "$(first_type naive)" naive $log_file "$config_file"
	;;
	brook)
		local protocol=$(config_n_get $node protocol client)
		local prefix=""
		[ "$protocol" == "wsclient" ] && {
			prefix="ws://"
			local brook_tls=$(config_n_get $node brook_tls 0)
			[ "$brook_tls" == "1" ] && {
				prefix="wss://"
				protocol="wssclient"
			}
			local ws_path=$(config_n_get $node ws_path "/ws")
		}
		server_host=${prefix}${server_host}
		ln_start_bin "$(first_type $(config_t_get global_app brook_file) brook)" "brook_SOCKS_${flag}" $log_file "$protocol" --socks5 "$bind:$socks_port" -s "${server_host}:${port}${ws_path}" -p "$(config_n_get $node password)"
	;;
	ssr)
		lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_start_bin "$(first_type ssr-local)" "ssr-local" $log_file -c "$config_file" -v
	;;
	ss)
		lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $socks_port -server_host $server_host -server_port $port -mode tcp_and_udp > $config_file
		ln_start_bin "$(first_type ss-local)" "ss-local" $log_file -c "$config_file" -v
	;;
	ss-rust)
		lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $socks_port -server_host $server_host -server_port $port -protocol socks -mode tcp_and_udp > $config_file
		ln_start_bin "$(first_type sslocal)" "sslocal" $log_file -c "$config_file" -v
	;;
	hysteria)
		lua $API_GEN_HYSTERIA -node $node -local_socks_port $socks_port > $config_file
		ln_start_bin "$(first_type $(config_t_get global_app hysteria_file))" "hysteria" $log_file -c "$config_file" client
	;;
	esac

	# http to socks
	[ "$type" != "v2ray" ] && [ "$type" != "xray" ] && [ "$type" != "socks" ] && [ "$http_port" != "0" ] && [ "$http_config_file" != "nil" ] && {
		local bin=$(first_type $(config_t_get global_app v2ray_file) v2ray)
		if [ -n "$bin" ]; then
			type="v2ray"
		else
			bin=$(first_type $(config_t_get global_app xray_file) xray)
			[ -n "$bin" ] && type="xray"
		fi
		[ -z "$type" ] && return 1
		lua $API_GEN_V2RAY_PROTO -local_http_port $http_port -server_proto socks -server_address "127.0.0.1" -server_port $socks_port -server_username $_username -server_password $_password > $http_config_file
		ln_start_bin "$bin" ${type} /dev/null -config="$http_config_file"
	}
}

run_redir() {
	local node bind local_port config_file REDIR_TYPE log_file
	eval_set_val $@
	[ -n "$log_file" ] || log_file="/dev/null"
	local redir_type=$(echo $REDIR_TYPE | tr 'A-Z' 'a-z')
	local type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
	local close_log=$(config_t_get global close_log_${redir_type} 1)
	[ "$close_log" = "1" ] && log_file="/dev/null"
	local remarks=$(config_n_get $node remarks)
	local server_host=$(config_n_get $node address)
	local port=$(config_n_get $node port)
	[ -n "$server_host" ] && [ -n "$port" ] && {
		check_host $server_host
		[ $? != 0 ] && {
			echolog "${REDIR_TYPE}节点：[$remarks]${server_host} 是非法的服务器地址，无法启动！"
			return 1
		}
		[ "$bind" != "127.0.0.1" ] && echolog "${REDIR_TYPE}节点：[$remarks]${server_host}:${port}，监听端口：$local_port"
	}
	eval ${REDIR_TYPE}_NODE_PORT=$port

	case "$REDIR_TYPE" in
	UDP)
		case "$type" in
		socks)
			local _socks_address=$(config_n_get $node address)
			local _socks_port=$(config_n_get $node port)
			local _socks_username=$(config_n_get $node username)
			local _socks_password=$(config_n_get $node password)
			[ -n "${_socks_username}" ] && [ -n "${_socks_password}" ] && local _extra_param="-a ${_socks_username} -k ${_socks_password}"
			ln_start_bin "$(first_type ipt2socks)" "ipt2socks_UDP" $log_file -l $local_port -b 127.0.0.1 -s ${_socks_address} -p ${_socks_port} ${_extra_param} -U -v
		;;
		v2ray|\
		xray)
			local loglevel=$(config_t_get global loglevel "warning")
			lua $API_GEN_V2RAY -node $node -proto udp -redir_port $local_port -loglevel $loglevel > $config_file
			ln_start_bin "$(first_type $(config_t_get global_app ${type}_file) ${type})" ${type} $log_file -config="$config_file"
		;;
		trojan-go)
			local loglevel=$(config_t_get global trojan_loglevel "2")
			lua $API_GEN_TROJAN -node $node -run_type nat -local_addr "0.0.0.0" -local_port $local_port -loglevel $loglevel > $config_file
			ln_start_bin "$(first_type $(config_t_get global_app trojan_go_file) trojan-go)" trojan-go $log_file -config "$config_file"
		;;
		trojan*)
			local loglevel=$(config_t_get global trojan_loglevel "2")
			lua $API_GEN_TROJAN -node $node -run_type nat -local_addr "0.0.0.0" -local_port $local_port -loglevel $loglevel > $config_file
			ln_start_bin "$(first_type ${type})" "${type}" $log_file -c "$config_file"
		;;
		naiveproxy)
			echolog "Naiveproxy不支持UDP转发！"
		;;
		brook)
			local protocol=$(config_n_get $node protocol client)
			if [ "$protocol" == "wsclient" ]; then
				echolog "Brook的WebSocket不支持UDP转发！"
			else
				ln_start_bin "$(first_type $(config_t_get global_app brook_file) brook)" "brook_UDP" $log_file tproxy -l ":$local_port" -s "$server_host:$port" -p "$(config_n_get $node password)" --doNotRunScripts
			fi
		;;
		ssr)
			lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port > $config_file
			ln_start_bin "$(first_type ssr-redir)" "ssr-redir" $log_file -c "$config_file" -v -U
		;;
		ss)
			lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port -mode udp_only > $config_file
			ln_start_bin "$(first_type ss-redir)" "ss-redir" $log_file -c "$config_file" -v
		;;
		ss-rust)
			lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port -protocol redir -mode udp_only > $config_file
			ln_start_bin "$(first_type sslocal)" "sslocal" $log_file -c "$config_file" -v
		;;
		hysteria)
			lua $API_GEN_HYSTERIA -node $node -local_udp_redir_port $local_port > $config_file
			ln_start_bin "$(first_type $(config_t_get global_app hysteria_file))" "hysteria" $log_file -c "$config_file" client
		;;
		esac
	;;
	TCP)
		local ipv6_tproxy=$(config_t_get global_other ipv6_tproxy 0)
		if [ $ipv6_tproxy == "1" ]; then
			if [ "$type" == "v2ray" ] || [ "$type" == "xray" ]; then
				PROXY_IPV6=1
				echolog "节点类型:$type，开启实验性IPv6透明代理(TProxy)..."
			else
				echolog "节点类型:$type，暂不支持IPv6透明代理(TProxy)..."
			fi
		fi
		local kcptun_use=$(config_n_get $node use_kcp 0)
		if [ "$kcptun_use" == "1" ]; then
			local kcptun_server_host=$(config_n_get $node kcp_server)
			local network_type="ipv4"
			local kcptun_port=$(config_n_get $node kcp_port)
			local kcptun_config="$(config_n_get $node kcp_opts)"
			if [ -z "$kcptun_port" -o -z "$kcptun_config" ]; then
				echolog "Kcptun未配置参数，错误！"
				return 1
			fi
			if [ -n "$kcptun_port" -a -n "$kcptun_config" ]; then
				local run_kcptun_ip=$server_host
				[ -n "$kcptun_server_host" ] && run_kcptun_ip=$(get_host_ip $network_type $kcptun_server_host)
				KCPTUN_REDIR_PORT=$(get_new_port $KCPTUN_REDIR_PORT tcp)
				kcptun_params="-l 0.0.0.0:$KCPTUN_REDIR_PORT -r $run_kcptun_ip:$kcptun_port $kcptun_config"
				ln_start_bin "$(first_type $(config_t_get global_app kcptun_client_file) kcptun-client)" "kcptun_TCP" $log_file $kcptun_params
			fi
		fi
		
		if [ "$tcp_proxy_way" = "redirect" ]; then
			can_ipt=$(echo "$REDIRECT_LIST" | grep "$type")
		elif [ "$tcp_proxy_way" = "tproxy" ]; then
			can_ipt=$(echo "$TPROXY_LIST" | grep "$type")
		fi
		[ -z "$can_ipt" ] && type="socks"
		
		case "$type" in
		socks)
			_socks_flag=1
			_socks_address=$(config_n_get $node address)
			_socks_port=$(config_n_get $node port)
			_socks_username=$(config_n_get $node username)
			_socks_password=$(config_n_get $node password)
			[ -z "$can_ipt" ] && {
				local _config_file=$config_file
				_config_file=$(echo ${_config_file} | sed "s/TCP/SOCKS_${node}/g")
				local _port=$(get_new_port 2080)
				run_socks flag="TCP" node=$node bind=127.0.0.1 socks_port=${_port} config_file=${_config_file}
				_socks_address=127.0.0.1
				_socks_port=${_port}
				unset _socks_username
				unset _socks_password
			}
		;;
		v2ray|\
		xray)
			local loglevel=$(config_t_get global loglevel "warning")
			local proto="-proto tcp"
			local _extra_param=""
			[ "$tcp_node_socks" = "1" ] && {
				local socks_param="-local_socks_port $tcp_node_socks_port"
				_extra_param="${_extra_param} ${socks_param}"
				config_file=$(echo $config_file | sed "s/TCP/TCP_SOCKS_$tcp_node_socks_id/g")
			}
			[ "$tcp_node_http" = "1" ] && {
				local http_param="-local_http_port $tcp_node_http_port"
				_extra_param="${_extra_param} ${http_param}"
				config_file=$(echo $config_file | sed "s/TCP/TCP_HTTP_$tcp_node_http_id/g")
			}
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				proto="-proto tcp,udp"
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
			}
			_extra_param="${_extra_param} ${proto}"
			lua $API_GEN_V2RAY -node $node -redir_port $local_port -loglevel $loglevel ${_extra_param} > $config_file
			ln_start_bin "$(first_type $(config_t_get global_app ${type}_file) ${type})" ${type} $log_file -config="$config_file"
		;;
		trojan-go)
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
			}
			local loglevel=$(config_t_get global trojan_loglevel "2")
			lua $API_GEN_TROJAN -node $node -run_type nat -local_addr "0.0.0.0" -local_port $local_port -loglevel $loglevel > $config_file
			ln_start_bin "$(first_type $(config_t_get global_app trojan_go_file) trojan-go)" trojan-go $log_file -config "$config_file"
		;;
		trojan*)
			[ "$tcp_proxy_way" = "tproxy" ] && lua_tproxy_arg="-use_tproxy true"
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
			}
			local loglevel=$(config_t_get global trojan_loglevel "2")
			lua $API_GEN_TROJAN -node $node -run_type nat -local_addr "0.0.0.0" -local_port $local_port -loglevel $loglevel $lua_tproxy_arg > $config_file
			ln_start_bin "$(first_type ${type})" "${type}" $log_file -c "$config_file"
		;;
		naiveproxy)
			lua $API_GEN_NAIVE -node $node -run_type redir -local_addr "0.0.0.0" -local_port $local_port > $config_file
			ln_start_bin "$(first_type naive)" naive $log_file "$config_file"
		;;
		brook)
			local server_ip=$server_host
			local protocol=$(config_n_get $node protocol client)
			local prefix=""
			[ "$protocol" == "wsclient" ] && {
				prefix="ws://"
				local brook_tls=$(config_n_get $node brook_tls 0)
				[ "$brook_tls" == "1" ] && prefix="wss://"
				local ws_path=$(config_n_get $node ws_path "/ws")
			}
			[ "$kcptun_use" == "1" ] && {
				server_ip=127.0.0.1
				port=$KCPTUN_REDIR_PORT
			}
			server_ip=${prefix}${server_ip}
			ln_start_bin "$(first_type $(config_t_get global_app brook_file) brook)" "brook_TCP" $log_file tproxy -l ":$local_port" -s "${server_ip}:${port}${ws_path}" -p "$(config_n_get $node password)" --doNotRunScripts
		;;
		ssr)
			[ "$tcp_proxy_way" = "tproxy" ] && lua_tproxy_arg="-tcp_tproxy true"
			if [ "$kcptun_use" == "1" ]; then
				lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port -server_host "127.0.0.1" -server_port $KCPTUN_REDIR_PORT $lua_tproxy_arg > $config_file
			else
				[ "$TCP_UDP" = "1" ] && {
					config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
					UDP_REDIR_PORT=$TCP_REDIR_PORT
					UDP_NODE="nil"
					_extra_param="-u"
				}
				lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port $lua_tproxy_arg > $config_file
			fi
			ln_start_bin "$(first_type ssr-redir)" "ssr-redir" $log_file -c "$config_file" -v ${_extra_param}
		;;
		ss)
			[ "$tcp_proxy_way" = "tproxy" ] && lua_tproxy_arg="-tcp_tproxy true"
			lua_mode_arg="-mode tcp_only"
			if [ "$kcptun_use" == "1" ]; then
				lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port -server_host "127.0.0.1" -server_port $KCPTUN_REDIR_PORT $lua_mode_arg $lua_tproxy_arg > $config_file
			else
				[ "$TCP_UDP" = "1" ] && {
					config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
					UDP_REDIR_PORT=$TCP_REDIR_PORT
					UDP_NODE="nil"
					lua_mode_arg="-mode tcp_and_udp"
				}
				lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port $lua_mode_arg $lua_tproxy_arg > $config_file
			fi
			ln_start_bin "$(first_type ss-redir)" "ss-redir" $log_file -c "$config_file" -v
		;;
		ss-rust)
			[ "$tcp_proxy_way" = "tproxy" ] && lua_tproxy_arg="-tcp_tproxy true"
			lua_mode_arg="-mode tcp_only"
			if [ "$kcptun_use" == "1" ]; then
				lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port -server_host "127.0.0.1" -server_port $KCPTUN_REDIR_PORT -protocol redir $lua_mode_arg $lua_tproxy_arg > $config_file
			else
				[ "$TCP_UDP" = "1" ] && {
					config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
					UDP_REDIR_PORT=$TCP_REDIR_PORT
					UDP_NODE="nil"
					lua_mode_arg="-mode tcp_and_udp"
				}
				lua $API_GEN_SS -node $node -local_addr "0.0.0.0" -local_port $local_port -protocol redir $lua_mode_arg $lua_tproxy_arg > $config_file
			fi
			ln_start_bin "$(first_type sslocal)" "sslocal" $log_file -c "$config_file" -v
		;;
		hysteria)
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
				_extra_param="-local_udp_redir_port $local_port"
			}
			lua $API_GEN_HYSTERIA -node $node -local_tcp_redir_port $local_port ${_extra_param} > $config_file
			ln_start_bin "$(first_type $(config_t_get global_app hysteria_file))" "hysteria" $log_file -c "$config_file" client
		;;
		esac
		if [ -n "${_socks_flag}" ]; then
			local _flag="TCP"
			local _extra_param="-T"
			[ "$TCP_UDP" = "1" ] && {
				_flag="TCP_UDP"
				_extra_param=""
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
			}
			local _socks_tproxy="-R"
			[ "$tcp_proxy_way" = "tproxy" ] && _socks_tproxy=""
			_extra_param="${_extra_param} ${_socks_tproxy}"
			[ -n "${_socks_username}" ] && [ -n "${_socks_password}" ] && _extra_param="-a ${_socks_username} -k ${_socks_password} ${_extra_param}"
			ln_start_bin "$(first_type ipt2socks)" "ipt2socks_${_flag}" $log_file -l $local_port -b 127.0.0.1 -s ${_socks_address} -p ${_socks_port} ${_extra_param} -v
		fi

		([ "$type" != "v2ray" ] && [ "$type" != "xray" ]) && {
			[ "$tcp_node_socks" = "1" ] && {
				local port=$tcp_node_socks_port
				local config_file=$TMP_PATH/SOCKS_$tcp_node_socks_id.json
				local log_file=$TMP_PATH/SOCKS_$tcp_node_socks_id.log
				local http_port=0
				local http_config_file=$TMP_PATH/HTTP2SOCKS_$tcp_node_http_id.json
				[ "$tcp_node_http" = "1" ] && {
					http_port=$tcp_node_http_port
				}
				run_socks flag=$tcp_node_socks_id node=$node bind=0.0.0.0 socks_port=$port config_file=$config_file http_port=$http_port http_config_file=$http_config_file
			}
		}
	;;
	esac
	return 0
}

node_switch() {
	[ -n "$1" -a -n "$2" ] && {
		[ -n "$4" ] && LOG_FILE="/dev/null"
		local node=$2
		pgrep -af "$TMP_PATH" | awk -v P1="$1" 'BEGIN{IGNORECASE=1}$0~P1 && !/acl\/|acl_/{print $1}' | xargs kill -9 >/dev/null 2>&1
		rm -rf $TMP_PATH/${1}*
		local config_file=$TMP_PATH/${1}.json
		local log_file=$TMP_PATH/${1}.log
		local port=$(cat $TMP_PORT_PATH/${1})

		[ "$SOCKS_ENABLED" = "1" ] && {
			local ids=$(uci show $CONFIG | grep "=socks" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')
			for id in $ids; do
				[ "$(config_n_get $id enabled 0)" == "0" ] && continue
				[ "$(config_n_get $id node nil)" != "tcp" ] && continue
				local socks_port=$(config_n_get $id port)
				local http_port=$(config_n_get $id http_port 0)
				pgrep -af "${TMP_PATH}.*${id}" | awk 'BEGIN{IGNORECASE=1}/SOCKS/{print $1}' | xargs kill -9 >/dev/null 2>&1
				tcp_node_socks=1
				tcp_node_socks_port=$socks_port
				tcp_node_socks_id=$id
				[ "$http_port" != "0" ] && {
					tcp_node_http=1
					tcp_node_http_port=$http_port
					tcp_node_http_id=$id
				}
				break
			done
		}
		
		[ "$3" != "0" ] && {
			local tcp_node=$(config_t_get global tcp_node nil)
			[ "$(config_n_get $tcp_node protocol nil)" = "_shunt" ] && {
				if [ "$3" == "1" ]; then
					uci set $CONFIG.$tcp_node.default_node="$node"
				elif [ "$3" == "2" ]; then
					uci set $CONFIG.$tcp_node.main_node="$node"
				fi
				uci commit $CONFIG
			}
			node=$tcp_node
		}

		run_redir node=$node bind=0.0.0.0 local_port=$port config_file=$config_file REDIR_TYPE=$1 log_file=$log_file
		echo $node > $TMP_ID_PATH/${1}
		
		[ "$3" != "0" ] && [ "$(config_n_get $node protocol nil)" = "_shunt" ] && {
			echo $(config_n_get $node default_node nil) > $TMP_ID_PATH/${1}_default
			echo $(config_n_get $node main_node nil) > $TMP_ID_PATH/${1}_main
			uci commit $CONFIG
		}

		[ "$1" = "TCP" ] && {
			[ "$(config_t_get global udp_node nil)" = "tcp" ] && [ "$UDP_REDIR_PORT" != "$TCP_REDIR_PORT" ] && {
				pgrep -af "$TMP_PATH" | awk 'BEGIN{IGNORECASE=1}/UDP/ && !/acl\/|acl_/{print $1}' | xargs kill -9 >/dev/null 2>&1
				UDP_NODE=$node
				start_redir UDP
			}
		}

		#local node_net=$(echo $1 | tr 'A-Z' 'a-z')
		#uci set $CONFIG.@global[0].${node_net}_node=$node
		#uci commit $CONFIG
		source $APP_PATH/helper_${DNS_N}.sh logic_restart no_log=1
	}
}

start_redir() {
	eval node=\$${1}_NODE
	if [ "$node" != "nil" ]; then
		TYPE=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
		local config_file=$TMP_PATH/${1}.json
		local log_file=$TMP_PATH/${1}.log
		eval current_port=\$${1}_REDIR_PORT
		local port=$(echo $(get_new_port $current_port $1))
		eval ${1}_REDIR=$port
		run_redir node=$node bind=0.0.0.0 local_port=$port config_file=$config_file REDIR_TYPE=$1 log_file=$log_file
		#eval ip=\$${1}_NODE_IP
		echo $port > $TMP_PORT_PATH/${1}
		echo $node > $TMP_ID_PATH/${1}
		[ "$(config_n_get $node protocol nil)" = "_shunt" ] && {
			local default_node=$(config_n_get $node default_node nil)
			local main_node=$(config_n_get $node main_node nil)
			echo $default_node > $TMP_ID_PATH/${1}_default
			echo $main_node > $TMP_ID_PATH/${1}_main
		}
	else
		[ "$1" = "UDP" ] && [ "$TCP_UDP" = "1" ] && return
		echolog "${1}节点没有选择或为空，不代理${1}。"
	fi
}

start_socks() {
	[ "$SOCKS_ENABLED" = "1" ] && {
		local ids=$(uci show $CONFIG | grep "=socks" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')
		[ -n "$ids" ] && echolog "分析 Socks 服务的节点配置..."
		for id in $ids; do
			local enabled=$(config_n_get $id enabled 0)
			[ "$enabled" == "0" ] && continue
			local node=$(config_n_get $id node nil)
			[ "$node" == "nil" ] && continue
			local port=$(config_n_get $id port)
			local config_file=$TMP_PATH/SOCKS_${id}.json
			local log_file=$TMP_PATH/SOCKS_${id}.log
			local http_port=$(config_n_get $id http_port 0)
			local http_config_file=$TMP_PATH/HTTP2SOCKS_${id}.json
			[ "$node" == "tcp" ] && {
				tcp_node_socks=1
				tcp_node_socks_port=$port
				tcp_node_socks_id=$id
				[ "$http_port" != "0" ] && {
					tcp_node_http=1
					tcp_node_http_port=$http_port
					tcp_node_http_id=$id
				}
				continue
			}
			run_socks flag=$id node=$node bind=0.0.0.0 socks_port=$port config_file=$config_file http_port=$http_port http_config_file=$http_config_file
		done
	}
}

clean_log() {
	logsnum=$(cat $LOG_FILE 2>/dev/null | wc -l)
	[ "$logsnum" -gt 1000 ] && {
		echo "" > $LOG_FILE
		echolog "日志文件过长，清空处理！"
	}
}

clean_crontab() {
	touch /etc/crontabs/root
	#sed -i "/${CONFIG}/d" /etc/crontabs/root >/dev/null 2>&1
	sed -i "/$(echo "/etc/init.d/${CONFIG}" | sed 's#\/#\\\/#g')/d" /etc/crontabs/root >/dev/null 2>&1
	sed -i "/$(echo "lua ${APP_PATH}/rule_update.lua log" | sed 's#\/#\\\/#g')/d" /etc/crontabs/root >/dev/null 2>&1
	sed -i "/$(echo "lua ${APP_PATH}/subscribe.lua start log" | sed 's#\/#\\\/#g')/d" /etc/crontabs/root >/dev/null 2>&1
}

start_crontab() {
	clean_crontab
	[ "$ENABLED" != 1 ] && {
		/etc/init.d/cron restart
		return
	}
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
		echo "$t lua $APP_PATH/rule_update.lua log > /dev/null 2>&1 &" >>/etc/crontabs/root
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

	if [ "$NO_PROXY" == 0 ]; then
		start_daemon=$(config_t_get global_delay start_daemon 0)
		[ "$start_daemon" = "1" ] && $APP_PATH/monitor.sh > /dev/null 2>&1 &

		AUTO_SWITCH_ENABLE=$(config_t_get auto_switch enable 0)
		[ "$AUTO_SWITCH_ENABLE" = "1" ] && $APP_PATH/test.sh > /dev/null 2>&1 &
	else
		echolog "运行于非代理模式，仅允许服务启停的定时任务。"
	fi

	/etc/init.d/cron restart
}

stop_crontab() {
	clean_crontab
	/etc/init.d/cron restart
	#echolog "清除定时执行命令。"
}

start_dns() {
	dns_listen_port=${DNS_PORT}
	TUN_DNS="127.0.0.1#${dns_listen_port}"

	echolog "过滤服务配置：准备接管域名解析..."

	case "$DNS_MODE" in
	nonuse)
		echolog "  - 不过滤DNS..."
		TUN_DNS=""
		use_chinadns_ng=$(config_t_get global always_use_chinadns_ng 0)
		[ "$use_chinadns_ng" == "0" ] && return
	;;
	dns2socks)
		local dns2socks_socks_server=$(echo $(config_t_get global socks_server 127.0.0.1:1080) | sed "s/#/:/g")
		local dns2socks_forward=$(get_first_dns DNS_FORWARD 53 | sed 's/#/:/g')
		run_dns2socks socks=$dns2socks_socks_server listen_address=127.0.0.1 listen_port=${dns_listen_port} dns=$dns2socks_forward cache=$DNS_CACHE
		echolog "  - 域名解析：dns2socks(127.0.0.1:${dns_listen_port})，${dns2socks_socks_server:-127.0.0.1:1080} -> ${dns2socks_forward-D8.8.8.8:53}"
	;;
	v2ray_tcp)
		local dns_forward=$(get_first_dns DNS_FORWARD 53 | sed 's/#/:/g')
		local dns_address=$(echo $dns_forward | awk -F ':' '{print $1}')
		local up_trust_tcp_dns=$(config_t_get global up_trust_tcp_dns "tcp")
		[ "${DNS_CACHE}" == "0" ] && local _extra_param="-dns_cache 0"
		if [ "$up_trust_tcp_dns" = "socks" ]; then
			use_tcp_node_resolve_dns=0
			local socks_server=$(echo $(config_t_get global socks_server 127.0.0.1:1080) | sed "s/#/:/g")
			local socks_address=$(echo $socks_server | awk -F ':' '{print $1}')
			local socks_port=$(echo $socks_server | awk -F ':' '{print $2}')
			lua $API_GEN_V2RAY -dns_listen_port "${dns_listen_port}" -dns_server "${dns_address}" -dns_tcp_server "tcp://${dns_forward}" -dns_socks_address "${socks_address}" -dns_socks_port "${socks_port}" ${_extra_param} > $TMP_PATH/DNS.json
		elif [ "${up_trust_tcp_dns}" = "tcp" ]; then
			use_tcp_node_resolve_dns=1
			lua $API_GEN_V2RAY -dns_listen_port "${dns_listen_port}" -dns_server "${dns_address}" -dns_tcp_server "tcp://${dns_forward}" ${_extra_param} > $TMP_PATH/DNS.json
		fi
		ln_start_bin "$(first_type $(config_t_get global_app v2ray_file) v2ray)" v2ray $TMP_PATH/DNS.log -config="$TMP_PATH/DNS.json"
		echolog "  - 域名解析 DNS Over TCP..."
	;;
	v2ray_doh|\
	xray_doh)
		local type=$(echo $DNS_MODE | awk -F '_' '{print $1}')
		up_trust_doh_dns=$(config_t_get global up_trust_doh_dns "tcp")
		up_trust_doh=$(config_t_get global up_trust_doh "https://dns.google/dns-query,8.8.4.4")
		_doh_url=$(echo $up_trust_doh | awk -F ',' '{print $1}')
		_doh_host_port=$(echo $_doh_url | sed "s/https:\/\///g" | awk -F '/' '{print $1}')
		_doh_host=$(echo $_doh_host_port | awk -F ':' '{print $1}')
		_doh_port=$(echo $_doh_host_port | awk -F ':' '{print $2}')
		_doh_bootstrap=$(echo $up_trust_doh | cut -d ',' -sf 2-)
		[ "${DNS_CACHE}" == "0" ] && local _extra_param="-dns_cache 0"
		
		if [ "$up_trust_doh_dns" = "socks" ]; then
			use_tcp_node_resolve_dns=0
			socks_server=$(echo $(config_t_get global socks_server 127.0.0.1:1080) | sed "s/#/:/g")
			socks_address=$(echo $socks_server | awk -F ':' '{print $1}')
			socks_port=$(echo $socks_server | awk -F ':' '{print $2}')
			lua $API_GEN_V2RAY -dns_listen_port "${dns_listen_port}" -dns_server "${_doh_bootstrap}" -doh_url "${_doh_url}" -doh_host "${_doh_host}" -dns_socks_address "${socks_address}" -dns_socks_port "${socks_port}" ${_extra_param} > $TMP_PATH/DNS.json
			ln_start_bin "$(first_type $(config_t_get global_app ${type}_file) ${type})" ${type} $TMP_PATH/DNS.log -config="$TMP_PATH/DNS.json"
		elif [ "${up_trust_doh_dns}" = "tcp" ]; then
			use_tcp_node_resolve_dns=1
			DNS_FORWARD=""
			_doh_bootstrap_dns=$(echo $_doh_bootstrap | sed "s/,/ /g")
			for _dns in $_doh_bootstrap_dns; do
				_dns=$(echo $_dns | awk -F ':' '{print $1}'):${_doh_port:-443}
				[ -n "$DNS_FORWARD" ] && DNS_FORWARD=${DNS_FORWARD},${_dns} || DNS_FORWARD=${_dns}
			done
			lua $API_GEN_V2RAY -dns_listen_port "${dns_listen_port}" -dns_server "${_doh_bootstrap}" -doh_url "${_doh_url}" -doh_host "${_doh_host}" ${_extra_param} > $TMP_PATH/DNS.json
			ln_start_bin "$(first_type $(config_t_get global_app ${type}_file) ${type})" ${type} $TMP_PATH/DNS.log -config="$TMP_PATH/DNS.json"
			unset _dns _doh_bootstrap_dns
		fi
		unset _doh_url _doh_port _doh_bootstrap
		echolog "  - 域名解析 DNS Over HTTPS..."
	;;
	pdnsd)
		use_tcp_node_resolve_dns=1
		gen_pdnsd_config "${dns_listen_port}" "${DNS_FORWARD}" "${DNS_CACHE}"
		ln_start_bin "$(first_type pdnsd)" pdnsd "/dev/null" --daemon -c "${TMP_PATH}/pdnsd/pdnsd.conf" -d
		echolog "  - 域名解析：pdnsd + 使用(TCP节点)解析域名..."
	;;
	udp)
		use_udp_node_resolve_dns=1
		TUN_DNS=${DNS_FORWARD}
		echolog "  - 域名解析：直接使用UDP节点请求DNS（$TUN_DNS）"
	;;
	custom)
		custom_dns=$(config_t_get global custom_dns)
		TUN_DNS="$(echo ${custom_dns} | sed 's/:/#/g')"
		echolog "  - 域名解析：使用UDP协议自定义DNS（$TUN_DNS）解析..."
	;;
	esac
	
	[ "${use_tcp_node_resolve_dns}" = "1" ] && echolog "  * 请确认上游 DNS 支持 TCP 查询，如非直连地址，确保 TCP 代理打开，并且已经正确转发！"
	[ "${use_udp_node_resolve_dns}" = "1" ] && echolog "  * 要求代理 DNS 请求，如上游 DNS 非直连地址，确保 UDP 代理打开，并且已经正确转发！"
	
	[ -n "$chnlist" ] && [ "$CHINADNS_NG" = "1" ] && [ -n "$(first_type chinadns-ng)" ] && [ -s "${RULES_PATH}/chnlist" ] && {
		china_ng_listen_port=$(expr $dns_listen_port + 1)
		china_ng_listen="127.0.0.1#${china_ng_listen_port}"
		china_ng_chn=$(echo -n $(echo "${LOCAL_DNS}" | sed "s/,/\n/g" | head -n2) | tr " " ",")
		china_ng_gfw="${TUN_DNS}"
		echolog "  | - (chinadns-ng) 最高支持4级域名过滤..."

		local gfwlist_param="${TMP_PATH}/chinadns_gfwlist"
		[ -s "${RULES_PATH}/gfwlist" ] && cp -a "${RULES_PATH}/gfwlist" "${gfwlist_param}"
		local chnlist_param="${TMP_PATH}/chinadns_chnlist"
		[ -s "${RULES_PATH}/chnlist" ] && cp -a "${RULES_PATH}/chnlist" "${chnlist_param}"

		[ -s "${RULES_PATH}/proxy_host" ] && {
			cat "${RULES_PATH}/proxy_host" >> "${gfwlist_param}"
			echolog "  | - [$?](chinadns-ng) 代理域名表合并到防火墙域名表"
		}
		[ -s "${RULES_PATH}/direct_host" ] && {
			cat "${RULES_PATH}/direct_host" >> "${chnlist_param}"
			echolog "  | - [$?](chinadns-ng) 域名白名单合并到中国域名表"
		}
		chnlist_param=${chnlist_param:+-m "${chnlist_param}" -M}
		local log_path="${TMP_PATH}/chinadns-ng.log"
		log_path="/dev/null"
		ln_start_bin "$(first_type chinadns-ng)" chinadns-ng "$log_path" -v -b 0.0.0.0 -l "${china_ng_listen_port}" ${china_ng_chn:+-c "${china_ng_chn}"} ${chnlist_param} ${china_ng_gfw:+-t "${china_ng_gfw}"} ${gfwlist_param:+-g "${gfwlist_param}"} -f
		echolog "  + 过滤服务：ChinaDNS-NG(:${china_ng_listen_port})：国内DNS：${china_ng_chn}，可信DNS：${china_ng_gfw}"
	}
	source $APP_PATH/helper_${DNS_N}.sh stretch
	source $APP_PATH/helper_${DNS_N}.sh add DNS_MODE=$DNS_MODE TMP_DNSMASQ_PATH=$TMP_DNSMASQ_PATH DNSMASQ_CONF_FILE=/var/dnsmasq.d/dnsmasq-passwall.conf DEFAULT_DNS=$DEFAULT_DNS LOCAL_DNS=$LOCAL_DNS TUN_DNS=$TUN_DNS CHINADNS_DNS=$china_ng_listen TCP_NODE=$TCP_NODE PROXY_MODE=${TCP_PROXY_MODE}${LOCALHOST_TCP_PROXY_MODE}
}

gen_pdnsd_config() {
	local listen_port=${1}
	local up_dns=${2}
	local cache=${3}
	local pdnsd_dir=${TMP_PATH}/pdnsd
	local perm_cache=2048
	local _cache="on"
	local query_method="tcp_only"
	local reject_ipv6_dns=
	[ "${cache}" = "0" ] && _cache="off" && perm_cache=0

	mkdir -p "${pdnsd_dir}"
	touch "${pdnsd_dir}/pdnsd.cache"
	chown -R root.nogroup "${pdnsd_dir}"
	if [ $PROXY_IPV6 == "0" ]; then
		reject_ipv6_dns=$(cat <<- 'EOF'

				reject = ::/0;
				reject_policy = negate;
		EOF
		)
	fi
	cat > "${pdnsd_dir}/pdnsd.conf" <<-EOF
		global {
			perm_cache = $perm_cache;
			cache_dir = "$pdnsd_dir";
			run_as = "root";
			server_ip = 127.0.0.1;
			server_port = ${listen_port};
			status_ctl = on;
			query_method = ${query_method};
			min_ttl = 1h;
			max_ttl = 1w;
			timeout = 10;
			par_queries = 2;
			neg_domain_pol = on;
			udpbufsize = 1024;
			proc_limit = 2;
			procq_limit = 8;
		}

	EOF
	echolog "  + [$?]Pdnsd (127.0.0.1:${listen_port})..."

	append_pdnsd_updns() {
		[ -z "${2}" ] && echolog "  | - 略过错误 : ${1}" && return 0
		cat >> $pdnsd_dir/pdnsd.conf <<-EOF
			server {
				label = "node-${2}_${3}";
				ip = ${2};
				edns_query = on;
				port = ${3};
				timeout = 4;
				interval = 10m;
				uptest = none;
				purge_cache = off;
				proxy_only = on;
				caching = $_cache;${reject_ipv6_dns}
			}
		EOF
		echolog "  | - [$?]上游DNS：${2}:${3}"
	}
	hosts_foreach up_dns append_pdnsd_updns 53
}

add_ip2route() {
	local ip=$(get_host_ip "ipv4" $1)
	[ -z "$ip" ] && {
		echolog "  - 无法解析${1}，路由表添加失败！"
		return 1
	}
	local remarks="${1}"
	[ "$remarks" != "$ip" ] && remarks="${1}(${ip})"
	local interface=$2
	local retries=5
	local failcount=0
	while [ "$failcount" -lt $retries ]; do
		unset msg
		ip route show dev ${interface} >/dev/null 2>&1
		if [ $? -ne 0 ]; then
			let "failcount++"
			echolog "  - 找不到出口接口：$interface，1分钟后再重试(${failcount}/${retries})，${ip}"
			[ "$failcount" -ge $retries ] && return 1
			sleep 1m
		else
			route add -host ${ip} dev ${interface} >/dev/null 2>&1
			echolog "  - ${remarks}添加路由表${interface}接口成功！"
			echo "$ip" >> $TMP_ROUTE_PATH/${interface}
			break
		fi
	done
}

delete_ip2route() {
	[ -d "${TMP_ROUTE_PATH}" ] && {
		for interface in $(ls ${TMP_ROUTE_PATH}); do
			for ip in $(cat ${TMP_ROUTE_PATH}/${interface}); do
				route del -host ${ip} dev ${interface} >/dev/null 2>&1
			done
		done
	}
}

start_haproxy() {
	local haproxy_path haproxy_file item items lport sort_items

	[ "$(config_t_get global_haproxy balancing_enable 0)" != "1" ] && return
	echolog "HAPROXY 负载均衡..."

	haproxy_path=${TMP_PATH}/haproxy
	mkdir -p "${haproxy_path}"
	haproxy_file=${haproxy_path}/config.cfg
	cat <<-EOF > "${haproxy_file}"
		global
		    log         127.0.0.1 local2
		    chroot      ${haproxy_path}
		    maxconn     60000
		    stats socket  ${haproxy_path}/haproxy.sock
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

	items=$(get_enabled_anonymous_secs "@haproxy_config")
	for item in $items; do
		lport=$(config_n_get ${item} haproxy_port 0)
		[ "${lport}" = "0" ] && echolog "  - 丢弃1个明显无效的节点" && continue
		sort_items="${sort_items}${IFS}${lport} ${item}"
	done

	items=$(echo "${sort_items}" | sort -n | cut -d ' ' -sf 2)

	unset lport
	local haproxy_port lbss lbweight export backup remark
	local msg bip bport hasvalid bbackup failcount interface
	for item in ${items}; do
		unset haproxy_port bbackup

		eval $(uci -q show "${CONFIG}.${item}" | cut -d '.' -sf 3-)
		get_ip_port_from "$lbss" bip bport 1

		[ -z "$haproxy_port" ] || [ -z "$bip" ] && echolog "  - 丢弃1个明显无效的节点" && continue
		[ "$backup" = "1" ] && bbackup="backup"
		remark=$(echo $bip | sed "s/\[//g" | sed "s/\]//g")

		[ "$lport" = "${haproxy_port}" ] || {
			hasvalid="1"
			lport=${haproxy_port}
			echolog "  + 入口 0.0.0.0:${lport}..."
			cat <<-EOF >> "${haproxy_file}"
				listen $lport
				    mode tcp
				    bind 0.0.0.0:$lport
			EOF
		}

		cat <<-EOF >> "${haproxy_file}"
			    server $remark:$bport $bip:$bport weight $lbweight check inter 1500 rise 1 fall 3 $bbackup
		EOF

		if [ "$export" != "0" ]; then
			add_ip2route ${bip} ${export} > /dev/null 2>&1 &
		fi
		
		haproxy_items="${haproxy_items}${IFS}${bip}:${bport}"
		echolog "  | - 出口节点：${bip}:${bport}，权重：${lbweight}"
	done

	# 控制台配置
	local console_port=$(config_t_get global_haproxy console_port)
	local console_user=$(config_t_get global_haproxy console_user)
	local console_password=$(config_t_get global_haproxy console_password)
	local auth=""
	[ -n "$console_user" ] && [ -n "$console_password" ] && auth="stats auth $console_user:$console_password"
	cat <<-EOF >> "${haproxy_file}"

		listen console
		    bind 0.0.0.0:$console_port
		    mode http
		    stats refresh 30s
		    stats uri /
		    stats admin if TRUE
		    $auth
	EOF

	[ "${hasvalid}" != "1" ] && echolog "  - 没有发现任何有效节点信息，不启动。" && return 0
	ln_start_bin "$(first_type haproxy)" haproxy "/dev/null" -f "${haproxy_file}"
	echolog "  * 控制台端口：${console_port}/，${auth:-公开}"
}

kill_all() {
	kill -9 $(pidof "$@") >/dev/null 2>&1
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
	ulimit -n 65535
	load_config
	start_haproxy
	start_socks
	
	[ "$NO_PROXY" == 1 ] || {
		if [ -z "$(command -v iptables)" ] && [ -z "$(command -v ipset)" ]; then
			echolog "系统未安装iptables或ipset，无法透明代理！"
		else
			start_redir TCP
			start_redir UDP
			start_dns
			source $APP_PATH/iptables.sh start
			source $APP_PATH/helper_${DNS_N}.sh logic_restart
		fi
	}
	start_crontab
	echolog "运行完成！\n"
}

stop() {
	clean_log
	source $APP_PATH/iptables.sh stop
	delete_ip2route
	kill_all v2ray-plugin obfs-local
	pgrep -f "sleep.*(9s|58s)" | xargs kill -9 >/dev/null 2>&1
	pgrep -af "${CONFIG}/" | awk '! /app\.sh|subscribe\.lua|rule_update\.lua/{print $1}' | xargs kill -9 >/dev/null 2>&1
	rm -rf $TMP_PATH
	unset V2RAY_LOCATION_ASSET
	unset XRAY_LOCATION_ASSET
	stop_crontab
	source $APP_PATH/helper_${DNS_N}.sh del
	source $APP_PATH/helper_${DNS_N}.sh restart no_log=1
	echolog "清空并关闭相关程序和缓存完成。"
}

arg1=$1
shift
case $arg1 in
get_new_port)
	get_new_port $@
	;;
run_socks)
	run_socks $@
	;;
run_redir)
	run_redir $@
	;;
node_switch)
	node_switch $@
	;;
stop)
	[ "$1" = "force" ] && force_stop
	stop
	;;
start)
	start
	;;
boot)
	boot
	;;
esac
