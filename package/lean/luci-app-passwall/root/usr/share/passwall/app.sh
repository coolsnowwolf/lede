#!/bin/sh
# Copyright (C) 2018-2020 L-WRT Team
# Copyright (C) 2021-2023 xiaorouji

. $IPKG_INSTROOT/lib/functions.sh
. $IPKG_INSTROOT/lib/functions/service.sh

CONFIG=passwall
TMP_PATH=/tmp/etc/$CONFIG
TMP_BIN_PATH=$TMP_PATH/bin
TMP_SCRIPT_FUNC_PATH=$TMP_PATH/script_func
TMP_ID_PATH=$TMP_PATH/id
TMP_PORT_PATH=$TMP_PATH/port
TMP_ROUTE_PATH=$TMP_PATH/route
TMP_ACL_PATH=$TMP_PATH/acl
TMP_IFACE_PATH=$TMP_PATH/iface
TMP_PATH2=/tmp/etc/${CONFIG}_tmp
DNSMASQ_PATH=/etc/dnsmasq.d
TMP_DNSMASQ_PATH=/tmp/dnsmasq.d/passwall
LOG_FILE=/tmp/log/$CONFIG.log
APP_PATH=/usr/share/$CONFIG
RULES_PATH=/usr/share/${CONFIG}/rules
DNS_N=dnsmasq
DNS_PORT=15353
TUN_DNS="127.0.0.1#${DNS_PORT}"
LOCAL_DNS=119.29.29.29,223.5.5.5
DEFAULT_DNS=
ENABLED_DEFAULT_ACL=0
PROXY_IPV6=0
PROXY_IPV6_UDP=0
resolve_dns=0
use_tcp_node_resolve_dns=0
use_udp_node_resolve_dns=0
LUA_UTIL_PATH=/usr/lib/lua/luci/passwall
UTIL_SINGBOX=$LUA_UTIL_PATH/util_sing-box.lua
UTIL_SS=$LUA_UTIL_PATH/util_shadowsocks.lua
UTIL_XRAY=$LUA_UTIL_PATH/util_xray.lua
UTIL_TROJAN=$LUA_UTIL_PATH/util_trojan.lua
UTIL_NAIVE=$LUA_UTIL_PATH/util_naiveproxy.lua
UTIL_HYSTERIA2=$LUA_UTIL_PATH/util_hysteria2.lua
UTIL_TUIC=$LUA_UTIL_PATH/util_tuic.lua

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

config_t_set() {
	local index=${4:-0}
	local ret=$(uci -q set "${CONFIG}.@${1}[${index}].${2}=${3}" 2>/dev/null)
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

check_depends() {
	local tables=${1}
	if [ "$tables" == "iptables" ]; then
		for depends in "iptables-mod-tproxy" "iptables-mod-socket" "iptables-mod-iprange" "iptables-mod-conntrack-extra" "kmod-ipt-nat"; do
			[ -z "$(opkg status ${depends} 2>/dev/null | grep 'Status' | awk -F ': ' '{print $2}' 2>/dev/null)" ] && echolog "$tables透明代理基础依赖 $depends 未安装..."
		done
	else
		for depends in "kmod-nft-socket" "kmod-nft-tproxy" "kmod-nft-nat"; do
			[ -z "$(opkg status ${depends} 2>/dev/null | grep 'Status' | awk -F ': ' '{print $2}' 2>/dev/null)" ] && echolog "$tables透明代理基础依赖 $depends 未安装..."
		done
	fi
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

ln_run() {
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
	[ "${output}" != "/dev/null" ] && local persist_log_path=$(config_t_get global persist_log_path) && local sys_log=$(config_t_get global sys_log "0")
	if [ -z "$persist_log_path" ] && [ "$sys_log" != "1" ]; then
		${file_func:-echolog " - ${ln_name}"} "$@" >${output} 2>&1 &
	else
		[ "${output: -1, -7}" == "TCP.log" ] && local protocol="TCP"
		[ "${output: -1, -7}" == "UDP.log" ] && local protocol="UDP"
		if [ -n "${persist_log_path}" ]; then
			mkdir -p ${persist_log_path}
			local log_file=${persist_log_path}/passwall_${protocol}_${ln_name}_$(date '+%F').log
			echolog "记录到持久性日志文件：${log_file}"
			${file_func:-echolog " - ${ln_name}"} "$@" >> ${log_file} 2>&1 &
			sys_log=0
		fi
		if [ "${sys_log}" == "1" ]; then
			echolog "记录 ${ln_name}_${protocol} 到系统日志"
			${file_func:-echolog " - ${ln_name}"} "$@" 2>&1 | logger -t PASSWALL_${protocol}_${ln_name} &
		fi
	fi
	process_count=$(ls $TMP_SCRIPT_FUNC_PATH | wc -l)
	process_count=$((process_count + 1))
	echo "${file_func:-echolog "  - ${ln_name}"} $@ >${output}" > $TMP_SCRIPT_FUNC_PATH/$process_count
}

lua_api() {
	local func=${1}
	[ -z "${func}" ] && {
		echo "nil"
		return
	}
	echo $(lua -e "local api = require 'luci.passwall.api' print(api.${func})")
}

run_ipt2socks() {
	local flag proto tcp_tproxy local_port socks_address socks_port socks_username socks_password log_file
	local _extra_param=""
	eval_set_val $@
	[ -n "$log_file" ] || log_file="/dev/null"
	socks_address=$(get_host_ip "ipv4" ${socks_address})
	[ -n "$socks_username" ] && [ -n "$socks_password" ] && _extra_param="${_extra_param} -a $socks_username -k $socks_password"
	[ -n "$tcp_tproxy" ] || _extra_param="${_extra_param} -R"
	case "$proto" in
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
	ln_run "$(first_type ipt2socks)" "ipt2socks_${flag}" $log_file -l $local_port -b 0.0.0.0 -s $socks_address -p $socks_port ${_extra_param}
}

run_singbox() {
	local flag type node tcp_redir_port udp_redir_port socks_address socks_port socks_username socks_password http_address http_port http_username http_password
	local dns_listen_port direct_dns_port direct_dns_udp_server remote_dns_protocol remote_dns_udp_server remote_dns_tcp_server remote_dns_doh remote_fakedns remote_dns_query_strategy dns_cache dns_socks_address dns_socks_port
	local loglevel log_file config_file
	local _extra_param=""
	eval_set_val $@
	[ -z "$type" ] && {
		local type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
		if [ "$type" != "sing-box" ]; then
			bin=$(first_type $(config_t_get global_app singbox_file) sing-box)
			[ -n "$bin" ] && type="sing-box"
		fi
	}
	[ -z "$type" ] && return 1
	[ -n "$log_file" ] || local log_file="/dev/null"
	_extra_param="${_extra_param} -log 1 -logfile ${log_file}"
	if [ "$log_file" = "/dev/null" ]; then
		_extra_param="${_extra_param} -log 0"
	else
		_extra_param="${_extra_param} -log 1 -logfile ${log_file}"
	fi
	[ -z "$loglevel" ] && local loglevel=$(config_t_get global loglevel "warn")
	[ "$loglevel" = "warning" ] && loglevel="warn"
	_extra_param="${_extra_param} -loglevel $loglevel"

	_extra_param="${_extra_param} -tags $($(first_type $(config_t_get global_app singbox_file) sing-box) version | grep 'Tags:' | awk '{print $2}')"

	[ -n "$flag" ] && _extra_param="${_extra_param} -flag $flag"
	[ -n "$node" ] && _extra_param="${_extra_param} -node $node"
	[ -n "$tcp_redir_port" ] && _extra_param="${_extra_param} -tcp_redir_port $tcp_redir_port"
	[ -n "$udp_redir_port" ] && _extra_param="${_extra_param} -udp_redir_port $udp_redir_port"
	[ -n "$socks_address" ] && _extra_param="${_extra_param} -local_socks_address $socks_address"
	[ -n "$socks_port" ] && _extra_param="${_extra_param} -local_socks_port $socks_port"
	[ -n "$socks_username" ] && [ -n "$socks_password" ] && _extra_param="${_extra_param} -local_socks_username $socks_username -local_socks_password $socks_password"
	[ -n "$http_address" ] && _extra_param="${_extra_param} -local_http_address $http_address"
	[ -n "$http_port" ] && _extra_param="${_extra_param} -local_http_port $http_port"
	[ -n "$http_username" ] && [ -n "$http_password" ] && _extra_param="${_extra_param} -local_http_username $http_username -local_http_password $http_password"
	[ -n "$dns_socks_address" ] && [ -n "$dns_socks_port" ] && _extra_param="${_extra_param} -dns_socks_address ${dns_socks_address} -dns_socks_port ${dns_socks_port}"
	[ -n "$dns_listen_port" ] && _extra_param="${_extra_param} -dns_listen_port ${dns_listen_port}"
	[ -n "$dns_cache" ] && _extra_param="${_extra_param} -dns_cache ${dns_cache}"
	
	local local_dns=$(echo -n $(echo "${LOCAL_DNS}" | sed "s/,/\n/g" | head -n1) | tr " " ",")
	[ -z "$direct_dns_udp_server" ] && direct_dns_udp_server=$(echo ${local_dns} | awk -F '#' '{print $1}')
	[ -z "$direct_dns_port" ] && direct_dns_port=$(echo ${local_dns} | awk -F '#' '{print $2}')
	[ -z "$direct_dns_port" ] && direct_dns_port=53
	[ -n "$direct_dns_udp_server" ] && _extra_param="${_extra_param} -direct_dns_udp_server ${direct_dns_udp_server}"
	[ -n "$direct_dns_port" ] && _extra_param="${_extra_param} -direct_dns_port ${direct_dns_port}"
	_extra_param="${_extra_param} -direct_dns_query_strategy UseIP"

	[ -n "$remote_dns_query_strategy" ] && _extra_param="${_extra_param} -remote_dns_query_strategy ${remote_dns_query_strategy}"
	case "$remote_dns_protocol" in
		tcp)
			local _dns=$(get_first_dns remote_dns_tcp_server 53 | sed 's/#/:/g')
			local _dns_address=$(echo ${_dns} | awk -F ':' '{print $1}')
			local _dns_port=$(echo ${_dns} | awk -F ':' '{print $2}')
			_extra_param="${_extra_param} -remote_dns_server ${_dns_address} -remote_dns_port ${_dns_port} -remote_dns_tcp_server tcp://${_dns}"
		;;
		doh)
			local _doh_url=$(echo $remote_dns_doh | awk -F ',' '{print $1}')
			local _doh_host_port=$(lua_api "get_domain_from_url(\"${_doh_url}\")")
			#local _doh_host_port=$(echo $_doh_url | sed "s/https:\/\///g" | awk -F '/' '{print $1}')
			local _doh_host=$(echo $_doh_host_port | awk -F ':' '{print $1}')
			local is_ip=$(lua_api "is_ip(\"${_doh_host}\")")
			local _doh_port=$(echo $_doh_host_port | awk -F ':' '{print $2}')
			[ -z "${_doh_port}" ] && _doh_port=443
			local _doh_bootstrap=$(echo $remote_dns_doh | cut -d ',' -sf 2-)
			[ "${is_ip}" = "true" ] && _doh_bootstrap=${_doh_host}
			[ -n "$_doh_bootstrap" ] && _extra_param="${_extra_param} -remote_dns_server ${_doh_bootstrap}"
			_extra_param="${_extra_param} -remote_dns_port ${_doh_port} -remote_dns_doh_url ${_doh_url} -remote_dns_doh_host ${_doh_host}"
		;;
	esac
	[ "$remote_fakedns" = "1" ] && _extra_param="${_extra_param} -remote_dns_fake 1"
	_extra_param="${_extra_param} -tcp_proxy_way $tcp_proxy_way"
	lua $UTIL_SINGBOX gen_config ${_extra_param} > $config_file
	ln_run "$(first_type $(config_t_get global_app singbox_file) sing-box)" "sing-box" $log_file run -c "$config_file"
}

run_xray() {
	local flag type node tcp_redir_port udp_redir_port socks_address socks_port socks_username socks_password http_address http_port http_username http_password
	local dns_listen_port remote_dns_udp_server remote_dns_tcp_server remote_dns_doh dns_client_ip dns_query_strategy dns_cache dns_socks_address dns_socks_port
	local loglevel log_file config_file
	local _extra_param=""
	eval_set_val $@
	[ -z "$type" ] && {
		local type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
		if [ "$type" != "xray" ]; then
			bin=$(first_type $(config_t_get global_app xray_file) xray)
			[ -n "$bin" ] && type="xray"
		fi
	}
	[ -z "$type" ] && return 1
	[ -n "$log_file" ] || local log_file="/dev/null"
	[ -z "$loglevel" ] && local loglevel=$(config_t_get global loglevel "warning")
	[ -n "$flag" ] && _extra_param="${_extra_param} -flag $flag"
	[ -n "$node" ] && _extra_param="${_extra_param} -node $node"
	[ -n "$tcp_redir_port" ] && _extra_param="${_extra_param} -tcp_redir_port $tcp_redir_port"
	[ -n "$udp_redir_port" ] && _extra_param="${_extra_param} -udp_redir_port $udp_redir_port"
	[ -n "$socks_address" ] && _extra_param="${_extra_param} -local_socks_address $socks_address"
	[ -n "$socks_port" ] && _extra_param="${_extra_param} -local_socks_port $socks_port"
	[ -n "$socks_username" ] && [ -n "$socks_password" ] && _extra_param="${_extra_param} -local_socks_username $socks_username -local_socks_password $socks_password"
	[ -n "$http_address" ] && _extra_param="${_extra_param} -local_http_address $http_address"
	[ -n "$http_port" ] && _extra_param="${_extra_param} -local_http_port $http_port"
	[ -n "$http_username" ] && [ -n "$http_password" ] && _extra_param="${_extra_param} -local_http_username $http_username -local_http_password $http_password"
	[ -n "$dns_socks_address" ] && [ -n "$dns_socks_port" ] && _extra_param="${_extra_param} -dns_socks_address ${dns_socks_address} -dns_socks_port ${dns_socks_port}"
	[ -n "$dns_listen_port" ] && _extra_param="${_extra_param} -dns_listen_port ${dns_listen_port}"
	[ -n "$dns_query_strategy" ] && _extra_param="${_extra_param} -dns_query_strategy ${dns_query_strategy}"
	[ -n "$dns_client_ip" ] && _extra_param="${_extra_param} -dns_client_ip ${dns_client_ip}"
	[ -n "$dns_cache" ] && _extra_param="${_extra_param} -dns_cache ${dns_cache}"
	[ -n "${remote_dns_tcp_server}" ] && {
		local _dns=$(get_first_dns remote_dns_tcp_server 53 | sed 's/#/:/g')
		local _dns_address=$(echo ${_dns} | awk -F ':' '{print $1}')
		local _dns_port=$(echo ${_dns} | awk -F ':' '{print $2}')
		_extra_param="${_extra_param} -remote_dns_tcp_server ${_dns_address} -remote_dns_tcp_port ${_dns_port}"
	}
	[ -n "${remote_dns_doh}" ] && {
		local _doh_url=$(echo $remote_dns_doh | awk -F ',' '{print $1}')
		local _doh_host_port=$(lua_api "get_domain_from_url(\"${_doh_url}\")")
		#local _doh_host_port=$(echo $_doh_url | sed "s/https:\/\///g" | awk -F '/' '{print $1}')
		local _doh_host=$(echo $_doh_host_port | awk -F ':' '{print $1}')
		local is_ip=$(lua_api "is_ip(\"${_doh_host}\")")
		local _doh_port=$(echo $_doh_host_port | awk -F ':' '{print $2}')
		[ -z "${_doh_port}" ] && _doh_port=443
		local _doh_bootstrap=$(echo $remote_dns_doh | cut -d ',' -sf 2-)
		[ "${is_ip}" = "true" ] && _doh_bootstrap=${_doh_host}
		[ -n "$_doh_bootstrap" ] && _extra_param="${_extra_param} -remote_dns_doh_ip ${_doh_bootstrap}"
		_extra_param="${_extra_param} -remote_dns_doh_port ${_doh_port} -remote_dns_doh_url ${_doh_url} -remote_dns_doh_host ${_doh_host}"
	}
	_extra_param="${_extra_param} -tcp_proxy_way $tcp_proxy_way"
	_extra_param="${_extra_param} -loglevel $loglevel"
	lua $UTIL_XRAY gen_config ${_extra_param} > $config_file
	ln_run "$(first_type $(config_t_get global_app ${type}_file) ${type})" ${type} $log_file run -c "$config_file"
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
	ln_run "$(first_type dns2socks)" "dns2socks${flag}" $log_file ${_extra_param} "${socks_address}:${socks_port}" "${dns}" "${listen_address}:${listen_port}"
}

run_chinadns_ng() {
	local _listen_port _dns_china _dns_trust _chnlist _gfwlist _no_ipv6_rules _log_path _no_logic_log
	eval_set_val $@
	
	local _LOG_FILE=$LOG_FILE
	[ -n "$_no_logic_log" ] && LOG_FILE="/dev/null"

	echolog "  | - (chinadns-ng) 最高支持4级域名过滤..."

	local _default_tag=$(config_t_get global chinadns_ng_default_tag smart)
	local _extra_param=""
	[ -n "$_chnlist" ] && {
		[ -s "${RULES_PATH}/chnlist" ] && {
			local _chnlist_file="${TMP_PATH}/chinadns_chnlist"
			cp -a "${RULES_PATH}/chnlist" "${_chnlist_file}"
			local chnroute4_set="passwall_chnroute"
			local chnroute6_set="passwall_chnroute6"
			[ "$nftflag" = "1" ] && {
				chnroute4_set="inet@fw4@passwall_chnroute"
				chnroute6_set="inet@fw4@passwall_chnroute6"
			}
			_extra_param="${_extra_param} -4 ${chnroute4_set} -6 ${chnroute6_set} -m ${_chnlist_file} -M -a"
		}
	}
	
	([ -n "$_chnlist" ] || [ -n "$_gfwlist" ]) && [ -s "${RULES_PATH}/gfwlist" ] && {
		local _gfwlist_file="${TMP_PATH}/chinadns_gfwlist"
		cp -a "${RULES_PATH}/gfwlist" "${_gfwlist_file}"
		local gfwlist_set="passwall_gfwlist,passwall_gfwlist6"
		[ "$nftflag" = "1" ] && gfwlist_set="inet@fw4@passwall_gfwlist,inet@fw4@passwall_gfwlist6"
		_extra_param="${_extra_param} -g ${_gfwlist_file} -A ${gfwlist_set}"
		#当只有使用gfwlist模式时设置默认DNS为本地直连
		[ -n "$_gfwlist" ] && [ -z "$_chnlist" ] && _default_tag="chn"
	}
	[ -n "$_default_tag" ] && [ "$_default_tag" != "smart" ] && _extra_param="${_extra_param} -d ${_default_tag}"

	_log_path="/dev/null"
	ln_run "$(first_type chinadns-ng)" chinadns-ng "$_log_path" -v -b 127.0.0.1 -l "${_listen_port}" ${_dns_china:+-c "${_dns_china}"} ${_dns_trust:+-t "${_dns_trust}"} ${_extra_param} -f ${_no_ipv6_rules:+-N=${_no_ipv6_rules}}
	echolog "  + 过滤服务：ChinaDNS-NG(:${_listen_port})：国内DNS：${_dns_china}，可信DNS：${_dns_trust}"
	LOG_FILE=${_LOG_FILE}
}

run_socks() {
	local flag node bind socks_port config_file http_port http_config_file relay_port log_file
	eval_set_val $@
	[ -n "$config_file" ] && [ -z "$(echo ${config_file} | grep $TMP_PATH)" ] && config_file=$TMP_PATH/$config_file
	[ -n "$http_port" ] || http_port=0
	[ -n "$http_config_file" ] && [ -z "$(echo ${http_config_file} | grep $TMP_PATH)" ] && http_config_file=$TMP_PATH/$http_config_file
	if [ -n "$log_file" ] && [ -z "$(echo ${log_file} | grep $TMP_PATH)" ]; then
		log_file=$TMP_PATH/$log_file
	else
		log_file="/dev/null"
	fi
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

	if [ "$type" == "sing-box" ] || [ "$type" == "xray" ]; then
		local protocol=$(config_n_get $node protocol)
		if [ "$protocol" == "_balancing" ] || [ "$protocol" == "_shunt" ] || [ "$protocol" == "_iface" ]; then
			unset error_msg
		fi
	fi

	[ -n "${error_msg}" ] && {
		[ "$bind" != "127.0.0.1" ] && echolog "  - Socks节点：[$remarks]${tmp}，启动中止 ${bind}:${socks_port} ${error_msg}"
		return 1
	}
	[ "$bind" != "127.0.0.1" ] && echolog "  - Socks节点：[$remarks]${tmp}，启动 ${bind}:${socks_port}"

	case "$type" in
	socks)
		local _socks_address=$(config_n_get $node address)
		local _socks_port=$(config_n_get $node port)
		local _socks_username=$(config_n_get $node username)
		local _socks_password=$(config_n_get $node password)
		[ "$http_port" != "0" ] && {
			http_flag=1
			config_file=$(echo $config_file | sed "s/SOCKS/HTTP_SOCKS/g")
			local _extra_param="-local_http_port $http_port"
		}
		local bin=$(first_type $(config_t_get global_app singbox_file) sing-box)
		if [ -n "$bin" ]; then
			type="sing-box"
			lua $UTIL_SINGBOX gen_proto_config -local_socks_port $socks_port ${_extra_param} -server_proto socks -server_address ${_socks_address} -server_port ${_socks_port} -server_username ${_socks_username} -server_password ${_socks_password} > $config_file
			ln_run "$bin" ${type} $log_file run -c "$config_file"
		else
			bin=$(first_type $(config_t_get global_app xray_file) xray)
			[ -n "$bin" ] && {
				type="xray"
				lua $UTIL_XRAY gen_proto_config -local_socks_port $socks_port ${_extra_param} -server_proto socks -server_address ${_socks_address} -server_port ${_socks_port} -server_username ${_socks_username} -server_password ${_socks_password} > $config_file
				ln_run "$bin" ${type} $log_file run -c "$config_file"
			}
		fi
	;;
	sing-box)
		[ "$http_port" != "0" ] && {
			http_flag=1
			config_file=$(echo $config_file | sed "s/SOCKS/HTTP_SOCKS/g")
			local _args="http_port=$http_port"
		}
		run_singbox flag=$flag node=$node socks_port=$socks_port config_file=$config_file log_file=$log_file ${_args}
	;;
	xray)
		[ "$http_port" != "0" ] && {
			http_flag=1
			config_file=$(echo $config_file | sed "s/SOCKS/HTTP_SOCKS/g")
			local _args="http_port=$http_port"
		}
		run_xray flag=$flag node=$node socks_port=$socks_port config_file=$config_file log_file=$log_file ${_args}
	;;
	trojan-go)
		lua $UTIL_TROJAN gen_config -node $node -run_type client -local_addr $bind -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_run "$(first_type $(config_t_get global_app trojan_go_file) trojan-go)" trojan-go $log_file -config "$config_file"
	;;
	trojan*)
		lua $UTIL_TROJAN gen_config -node $node -run_type client -local_addr $bind -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_run "$(first_type ${type})" "${type}" $log_file -c "$config_file"
	;;
	naiveproxy)
		lua $UTIL_NAIVE gen_config -node $node -run_type socks -local_addr $bind -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_run "$(first_type naive)" naive $log_file "$config_file"
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
		ln_run "$(first_type $(config_t_get global_app brook_file) brook)" "brook_SOCKS_${flag}" $log_file "$protocol" --socks5 "$bind:$socks_port" -s "${server_host}:${port}${ws_path}" -p "$(config_n_get $node password)"
	;;
	ssr)
		lua $UTIL_SS gen_config -node $node -local_addr "0.0.0.0" -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_run "$(first_type ssr-local)" "ssr-local" $log_file -c "$config_file" -v -u
	;;
	ss)
		lua $UTIL_SS gen_config -node $node -local_addr "0.0.0.0" -local_port $socks_port -server_host $server_host -server_port $port -mode tcp_and_udp > $config_file
		ln_run "$(first_type ss-local)" "ss-local" $log_file -c "$config_file" -v
	;;
	ss-rust)
		[ "$http_port" != "0" ] && {
			http_flag=1
			config_file=$(echo $config_file | sed "s/SOCKS/HTTP_SOCKS/g")
			local _extra_param="-local_http_port $http_port"
		}
		lua $UTIL_SS gen_config -node $node -local_socks_port $socks_port -server_host $server_host -server_port $port ${_extra_param} > $config_file
		ln_run "$(first_type sslocal)" "sslocal" $log_file -c "$config_file" -v
	;;
	hysteria2)
		[ "$http_port" != "0" ] && {
			http_flag=1
			config_file=$(echo $config_file | sed "s/SOCKS/HTTP_SOCKS/g")
			local _extra_param="-local_http_port $http_port"
		}
		lua $UTIL_HYSTERIA2 gen_config -node $node -local_socks_port $socks_port -server_host $server_host -server_port $port ${_extra_param} > $config_file
		ln_run "$(first_type $(config_t_get global_app hysteria_file))" "hysteria" $log_file -c "$config_file" client
	;;
	tuic)
		lua $UTIL_TUIC gen_config -node $node -local_addr $bind -local_port $socks_port -server_host $server_host -server_port $port > $config_file
		ln_run "$(first_type tuic-client)" "tuic-client" $log_file -c "$config_file"
	;;
	esac

	# http to socks
	[ -z "$http_flag" ] && [ "$http_port" != "0" ] && [ -n "$http_config_file" ] && [ "$type" != "sing-box" ] && [ "$type" != "xray" ] && [ "$type" != "socks" ] && {
		local bin=$(first_type $(config_t_get global_app singbox_file) sing-box)
		if [ -n "$bin" ]; then
			type="sing-box"
			lua $UTIL_SINGBOX gen_proto_config -local_http_port $http_port -server_proto socks -server_address "127.0.0.1" -server_port $socks_port -server_username $_username -server_password $_password > $http_config_file
			ln_run "$bin" ${type} /dev/null run -c "$http_config_file"
		else
			bin=$(first_type $(config_t_get global_app xray_file) xray)
			[ -n "$bin" ] && type="xray"
			[ -z "$type" ] && return 1
			lua $UTIL_XRAY gen_proto_config -local_http_port $http_port -server_proto socks -server_address "127.0.0.1" -server_port $socks_port -server_username $_username -server_password $_password > $http_config_file
			ln_run "$bin" ${type} /dev/null run -c "$http_config_file"
		fi
	}
	unset http_flag
}

run_redir() {
	local node proto bind local_port config_file log_file
	eval_set_val $@
	local tcp_node_socks_flag tcp_node_http_flag
	[ -n "$config_file" ] && [ -z "$(echo ${config_file} | grep $TMP_PATH)" ] && config_file=$TMP_PATH/$config_file
	if [ -n "$log_file" ] && [ -z "$(echo ${log_file} | grep $TMP_PATH)" ]; then
		log_file=$TMP_PATH/$log_file
	else
		log_file="/dev/null"
	fi
	local proto=$(echo $proto | tr 'A-Z' 'a-z')
	local PROTO=$(echo $proto | tr 'a-z' 'A-Z')
	local type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
	local close_log=$(config_t_get global close_log_${proto} 1)
	[ "$close_log" = "1" ] && log_file="/dev/null"
	local remarks=$(config_n_get $node remarks)
	local server_host=$(config_n_get $node address)
	local port=$(config_n_get $node port)
	[ -n "$server_host" ] && [ -n "$port" ] && {
		check_host $server_host
		[ $? != 0 ] && {
			echolog "${PROTO}节点：[$remarks]${server_host} 是非法的服务器地址，无法启动！"
			return 1
		}
	}
	[ "$bind" != "127.0.0.1" ] && echolog "${PROTO}节点：[$remarks]，监听端口：$local_port"
	eval ${PROTO}_NODE_PORT=$port

	case "$PROTO" in
	UDP)
		case "$type" in
		socks)
			local _socks_address=$(config_n_get $node address)
			_socks_address=$(get_host_ip "ipv4" ${_socks_address})
			local _socks_port=$(config_n_get $node port)
			local _socks_username=$(config_n_get $node username)
			local _socks_password=$(config_n_get $node password)
			[ -n "${_socks_username}" ] && [ -n "${_socks_password}" ] && local _extra_param="-a ${_socks_username} -k ${_socks_password}"
			ln_run "$(first_type ipt2socks)" "ipt2socks_UDP" $log_file -l $local_port -b 0.0.0.0 -s ${_socks_address} -p ${_socks_port} ${_extra_param} -U -v
		;;
		sing-box)
			run_singbox flag=UDP node=$node udp_redir_port=$local_port config_file=$config_file log_file=$log_file
		;;
		xray)
			run_xray flag=UDP node=$node udp_redir_port=$local_port config_file=$config_file log_file=$log_file
		;;
		trojan-go)
			local loglevel=$(config_t_get global trojan_loglevel "2")
			lua $UTIL_TROJAN gen_config -node $node -run_type nat -local_addr "0.0.0.0" -local_port $local_port -loglevel $loglevel > $config_file
			ln_run "$(first_type $(config_t_get global_app trojan_go_file) trojan-go)" trojan-go $log_file -config "$config_file"
		;;
		trojan*)
			local loglevel=$(config_t_get global trojan_loglevel "2")
			lua $UTIL_TROJAN gen_config -node $node -run_type nat -local_addr "0.0.0.0" -local_port $local_port -loglevel $loglevel > $config_file
			ln_run "$(first_type ${type})" "${type}" $log_file -c "$config_file"
		;;
		naiveproxy)
			echolog "Naiveproxy不支持UDP转发！"
		;;
		brook)
			local protocol=$(config_n_get $node protocol client)
			if [ "$protocol" == "wsclient" ]; then
				echolog "Brook的WebSocket不支持UDP转发！"
			else
				ln_run "$(first_type $(config_t_get global_app brook_file) brook)" "brook_UDP" $log_file tproxy -l ":$local_port" -s "$server_host:$port" -p "$(config_n_get $node password)" --doNotRunScripts
			fi
		;;
		ssr)
			lua $UTIL_SS gen_config -node $node -local_addr "0.0.0.0" -local_port $local_port > $config_file
			ln_run "$(first_type ssr-redir)" "ssr-redir" $log_file -c "$config_file" -v -U
		;;
		ss)
			lua $UTIL_SS gen_config -node $node -local_addr "0.0.0.0" -local_port $local_port -mode udp_only > $config_file
			ln_run "$(first_type ss-redir)" "ss-redir" $log_file -c "$config_file" -v
		;;
		ss-rust)
			lua $UTIL_SS gen_config -node $node -local_udp_redir_port $local_port > $config_file
			ln_run "$(first_type sslocal)" "sslocal" $log_file -c "$config_file" -v
		;;
		hysteria2)
			lua $UTIL_HYSTERIA2 gen_config -node $node -local_udp_redir_port $local_port > $config_file
			ln_run "$(first_type $(config_t_get global_app hysteria_file))" "hysteria" $log_file -c "$config_file" client
		;;
		tuic)
			echolog "TUIC不支持UDP转发！"
		;;
		esac
	;;
	TCP)
		tcp_node_socks=1
		tcp_node_socks_port=$(get_new_port $(config_t_get global tcp_node_socks_port 1070))
		tcp_node_http_port=$(config_t_get global tcp_node_http_port 0)
		[ "$tcp_node_http_port" != "0" ] && tcp_node_http=1
		if [ $PROXY_IPV6 == "1" ]; then
			echolog "开启实验性IPv6透明代理(TProxy)，请确认您的节点及类型支持IPv6！"
			PROXY_IPV6_UDP=1
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
			_socks_address=$(get_host_ip "ipv4" ${_socks_address})
			_socks_port=$(config_n_get $node port)
			_socks_username=$(config_n_get $node username)
			_socks_password=$(config_n_get $node password)
			[ -z "$can_ipt" ] && {
				local _config_file=$config_file
				_config_file="TCP_SOCKS_${node}.json"
				local _port=$(get_new_port 2080)
				run_socks flag="TCP" node=$node bind=127.0.0.1 socks_port=${_port} config_file=${_config_file}
				_socks_address=127.0.0.1
				_socks_port=${_port}
				unset _socks_username
				unset _socks_password
			}
		;;
		sing-box)
			local _flag="TCP"
			local _args=""
			[ "$tcp_node_socks" = "1" ] && {
				tcp_node_socks_flag=1
				_args="${_args} socks_port=${tcp_node_socks_port}"
				config_file=$(echo $config_file | sed "s/TCP/TCP_SOCKS/g")
			}
			[ "$tcp_node_http" = "1" ] && {
				tcp_node_http_flag=1
				_args="${_args} http_port=${tcp_node_http_port}"
				config_file=$(echo $config_file | sed "s/TCP/TCP_HTTP/g")
			}
			[ "$TCP_UDP" = "1" ] && {
				UDP_REDIR_PORT=$local_port
				UDP_NODE="nil"
				_flag="TCP_UDP"
				_args="${_args} udp_redir_port=${UDP_REDIR_PORT}"
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
			}
			[ "${DNS_MODE}" = "sing-box" ] && {
				resolve_dns=1
				config_file=$(echo $config_file | sed "s/.json/_DNS.json/g")
				_args="${_args} remote_dns_query_strategy=${DNS_QUERY_STRATEGY}"
				[ "${DNS_CACHE}" == "0" ] && _args="${_args} dns_cache=0"
				local v2ray_dns_mode=$(config_t_get global v2ray_dns_mode tcp)
				_args="${_args} remote_dns_protocol=${v2ray_dns_mode}"
				_args="${_args} dns_listen_port=${dns_listen_port}"
				local logout=""
				case "$v2ray_dns_mode" in
					tcp)
						_args="${_args} remote_dns_tcp_server=${REMOTE_DNS}"
						logout="  - 域名解析 DNS Over TCP"
					;;
					doh)
						remote_dns_doh=$(config_t_get global remote_dns_doh "https://1.1.1.1/dns-query")
						_args="${_args} remote_dns_doh=${remote_dns_doh}"
						logout="  - 域名解析 DNS Over HTTPS"
					;;
				esac
				local remote_fakedns=$(config_t_get global remote_fakedns 0)
				[ "${remote_fakedns}" = "1" ] && {
					fakedns=1
					_args="${_args} remote_fakedns=1"
					logout="${logout} + FakeDNS"
				}
				echolog ${logout}
			}
			run_singbox flag=$_flag node=$node tcp_redir_port=$local_port config_file=$config_file log_file=$log_file ${_args}
		;;
		xray)
			local _flag="TCP"
			local _args=""
			[ "$tcp_node_socks" = "1" ] && {
				tcp_node_socks_flag=1
				_args="${_args} socks_port=${tcp_node_socks_port}"
				config_file=$(echo $config_file | sed "s/TCP/TCP_SOCKS/g")
			}
			[ "$tcp_node_http" = "1" ] && {
				tcp_node_http_flag=1
				_args="${_args} http_port=${tcp_node_http_port}"
				config_file=$(echo $config_file | sed "s/TCP/TCP_HTTP/g")
			}
			[ "$TCP_UDP" = "1" ] && {
				UDP_REDIR_PORT=$local_port
				UDP_NODE="nil"
				_flag="TCP_UDP"
				_args="${_args} udp_redir_port=${UDP_REDIR_PORT}"
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
			}
			[ "${DNS_MODE}" = "xray" ] && {
				resolve_dns=1
				config_file=$(echo $config_file | sed "s/.json/_DNS.json/g")
				_args="${_args} dns_query_strategy=${DNS_QUERY_STRATEGY}"
				local _dns_client_ip=$(config_t_get global dns_client_ip)
				[ -n "${_dns_client_ip}" ] && _args="${_args} dns_client_ip=${_dns_client_ip}"
				[ "${DNS_CACHE}" == "0" ] && _args="${_args} dns_cache=0"
				_args="${_args} dns_listen_port=${dns_listen_port}"
				_args="${_args} remote_dns_tcp_server=${REMOTE_DNS}"
				local v2ray_dns_mode=$(config_t_get global v2ray_dns_mode tcp)
				[ "$v2ray_dns_mode" = "tcp+doh" ] && {
					remote_dns_doh=$(config_t_get global remote_dns_doh "https://1.1.1.1/dns-query")
					_args="${_args} remote_dns_doh=${remote_dns_doh}"
				}
			}
			run_xray flag=$_flag node=$node tcp_redir_port=$local_port config_file=$config_file log_file=$log_file ${_args}
		;;
		trojan-go)
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
			}
			local loglevel=$(config_t_get global trojan_loglevel "2")
			lua $UTIL_TROJAN gen_config -node $node -run_type nat -local_addr "0.0.0.0" -local_port $local_port -loglevel $loglevel > $config_file
			ln_run "$(first_type $(config_t_get global_app trojan_go_file) trojan-go)" trojan-go $log_file -config "$config_file"
		;;
		trojan*)
			[ "$tcp_proxy_way" = "tproxy" ] && lua_tproxy_arg="-use_tproxy true"
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
			}
			local loglevel=$(config_t_get global trojan_loglevel "2")
			lua $UTIL_TROJAN gen_config -node $node -run_type nat -local_addr "0.0.0.0" -local_port $local_port -loglevel $loglevel $lua_tproxy_arg > $config_file
			ln_run "$(first_type ${type})" "${type}" $log_file -c "$config_file"
		;;
		naiveproxy)
			lua $UTIL_NAIVE gen_config -node $node -run_type redir -local_addr "0.0.0.0" -local_port $local_port > $config_file
			ln_run "$(first_type naive)" naive $log_file "$config_file"
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
			server_ip=${prefix}${server_ip}
			ln_run "$(first_type $(config_t_get global_app brook_file) brook)" "brook_TCP" $log_file tproxy -l ":$local_port" -s "${server_ip}:${port}${ws_path}" -p "$(config_n_get $node password)" --doNotRunScripts
		;;
		ssr)
			[ "$tcp_proxy_way" = "tproxy" ] && lua_tproxy_arg="-tcp_tproxy true"
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
				_extra_param="-u"
			}
			lua $UTIL_SS gen_config -node $node -local_addr "0.0.0.0" -local_port $local_port $lua_tproxy_arg > $config_file
			ln_run "$(first_type ssr-redir)" "ssr-redir" $log_file -c "$config_file" -v ${_extra_param}
		;;
		ss)
			[ "$tcp_proxy_way" = "tproxy" ] && lua_tproxy_arg="-tcp_tproxy true"
			lua_mode_arg="-mode tcp_only"
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
				lua_mode_arg="-mode tcp_and_udp"
			}
			lua $UTIL_SS gen_config -node $node -local_addr "0.0.0.0" -local_port $local_port $lua_mode_arg $lua_tproxy_arg > $config_file
			ln_run "$(first_type ss-redir)" "ss-redir" $log_file -c "$config_file" -v
		;;
		ss-rust)
			local _extra_param="-local_tcp_redir_port $local_port"
			[ "$tcp_proxy_way" = "tproxy" ] && _extra_param="${_extra_param} -tcp_tproxy true"
			[ "$tcp_node_socks" = "1" ] && {
				tcp_node_socks_flag=1
				config_file=$(echo $config_file | sed "s/TCP/TCP_SOCKS/g")
				_extra_param="${_extra_param} -local_socks_port ${tcp_node_socks_port}"
			}
			[ "$tcp_node_http" = "1" ] && {
				tcp_node_http_flag=1
				config_file=$(echo $config_file | sed "s/TCP/TCP_HTTP/g")
				_extra_param="${_extra_param} -local_http_port ${tcp_node_http_port}"
			}
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
				_extra_param="${_extra_param} -local_udp_redir_port $local_port"
			}
			lua $UTIL_SS gen_config -node $node ${_extra_param} > $config_file
			ln_run "$(first_type sslocal)" "sslocal" $log_file -c "$config_file" -v
		;;
		hysteria2)
			local _extra_param="-local_tcp_redir_port $local_port"
			[ "$tcp_node_socks" = "1" ] && {
				tcp_node_socks_flag=1
				config_file=$(echo $config_file | sed "s/TCP/TCP_SOCKS/g")
				_extra_param="${_extra_param} -local_socks_port ${tcp_node_socks_port}"
			}
			[ "$tcp_node_http" = "1" ] && {
				tcp_node_http_flag=1
				config_file=$(echo $config_file | sed "s/TCP/TCP_HTTP/g")
				_extra_param="${_extra_param} -local_http_port ${tcp_node_http_port}"
			}
			[ "$TCP_UDP" = "1" ] && {
				config_file=$(echo $config_file | sed "s/TCP/TCP_UDP/g")
				UDP_REDIR_PORT=$TCP_REDIR_PORT
				UDP_NODE="nil"
				_extra_param="${_extra_param} -local_udp_redir_port $local_port"
			}
			_extra_param="${_extra_param} -tcp_proxy_way $tcp_proxy_way"
			lua $UTIL_HYSTERIA2 gen_config -node $node ${_extra_param} > $config_file
			ln_run "$(first_type $(config_t_get global_app hysteria_file))" "hysteria" $log_file -c "$config_file" client
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
			ln_run "$(first_type ipt2socks)" "ipt2socks_${_flag}" $log_file -l $local_port -b 0.0.0.0 -s ${_socks_address} -p ${_socks_port} ${_extra_param} -v
		fi

		[ -z "$tcp_node_socks_flag" ] && {
			[ "$tcp_node_socks" = "1" ] && {
				local port=$tcp_node_socks_port
				local config_file="SOCKS_TCP.json"
				local log_file="SOCKS_TCP.log"
				local http_port=0
				local http_config_file="HTTP2SOCKS_TCP.json"
				[ "$tcp_node_http" = "1" ] && [ -z "$tcp_node_http_flag" ] && {
					http_port=$tcp_node_http_port
				}
				run_socks flag=TCP node=$node bind=0.0.0.0 socks_port=$port config_file=$config_file http_port=$http_port http_config_file=$http_config_file
			}
		}

		[ "$tcp_node_socks" = "1" ] && {
			echo "127.0.0.1:$tcp_node_socks_port" > $TMP_PATH/TCP_SOCKS_server
		}
	;;
	esac
	unset tcp_node_socks_flag tcp_node_http_flag
	return 0
}

start_redir() {
	local proto=${1}
	eval node=\$${proto}_NODE
	if [ "$node" != "nil" ]; then
		TYPE=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
		local config_file="${proto}.json"
		local log_file="${proto}.log"
		eval current_port=\$${proto}_REDIR_PORT
		local port=$(echo $(get_new_port $current_port $proto))
		eval ${proto}_REDIR=$port
		run_redir node=$node proto=${proto} bind=0.0.0.0 local_port=$port config_file=$config_file log_file=$log_file
		#eval ip=\$${proto}_NODE_IP
		echo $port > $TMP_PORT_PATH/${proto}
		echo $node > $TMP_ID_PATH/${proto}
		[ "$(config_n_get $node protocol nil)" = "_shunt" ] && {
			local default_node=$(config_n_get $node default_node nil)
			local main_node=$(config_n_get $node main_node nil)
			echo $default_node > $TMP_ID_PATH/${proto}_default
			echo $main_node > $TMP_ID_PATH/${proto}_main
		}
	else
		[ "${proto}" = "UDP" ] && [ "$TCP_UDP" = "1" ] && return
		echolog "${proto}节点没有选择或为空，不代理${proto}。"
	fi
}

start_socks() {
	[ "$SOCKS_ENABLED" = "1" ] && {
		local ids=$(uci show $CONFIG | grep "=socks" | awk -F '.' '{print $2}' | awk -F '=' '{print $1}')
		[ -n "$ids" ] && {
			echolog "分析 Socks 服务的节点配置..."
			for id in $ids; do
				local enabled=$(config_n_get $id enabled 0)
				[ "$enabled" == "0" ] && continue
				local node=$(config_n_get $id node nil)
				[ "$node" == "nil" ] && continue
				local port=$(config_n_get $id port)
				local config_file="SOCKS_${id}.json"
				local log_file="SOCKS_${id}.log"
				local http_port=$(config_n_get $id http_port 0)
				local http_config_file="HTTP2SOCKS_${id}.json"
				run_socks flag=$id node=$node bind=0.0.0.0 socks_port=$port config_file=$config_file http_port=$http_port http_config_file=$http_config_file
				echo $node > $TMP_ID_PATH/socks_${id}

				#自动切换逻辑
				local enable_autoswitch=$(config_n_get $id enable_autoswitch 0)
				[ "$enable_autoswitch" = "1" ] && $APP_PATH/socks_auto_switch.sh ${id} > /dev/null 2>&1 &
			done
		}
	}
}

socks_node_switch() {
	local flag new_node
	eval_set_val $@
	[ -n "$flag" ] && [ -n "$new_node" ] && {
		pgrep -af "$TMP_BIN_PATH" | awk -v P1="${flag}" 'BEGIN{IGNORECASE=1}$0~P1 && !/acl\/|acl_/{print $1}' | xargs kill -9 >/dev/null 2>&1
		rm -rf $TMP_PATH/SOCKS_${flag}*
		rm -rf $TMP_PATH/HTTP2SOCKS_${flag}*

		for filename in $(ls ${TMP_SCRIPT_FUNC_PATH}); do
			cmd=$(cat ${TMP_SCRIPT_FUNC_PATH}/${filename})
			[ -n "$(echo $cmd | grep "${flag}")" ] && rm -f ${TMP_SCRIPT_FUNC_PATH}/${filename}
		done
		local port=$(config_n_get $flag port)
		local config_file="SOCKS_${flag}.json"
		local log_file="SOCKS_${flag}.log"
		local http_port=$(config_n_get $flag http_port 0)
		local http_config_file="HTTP2SOCKS_${flag}.json"
		LOG_FILE="/dev/null"
		run_socks flag=$flag node=$new_node bind=0.0.0.0 socks_port=$port config_file=$config_file http_port=$http_port http_config_file=$http_config_file
		echo $new_node > $TMP_ID_PATH/socks_${flag}
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
	sed -i "/$(echo "lua ${APP_PATH}/subscribe.lua start" | sed 's#\/#\\\/#g')/d" /etc/crontabs/root >/dev/null 2>&1
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

	TMP_SUB_PATH=$TMP_PATH/sub_crontabs
	mkdir -p $TMP_SUB_PATH
	for item in $(uci show ${CONFIG} | grep "=subscribe_list" | cut -d '.' -sf 2 | cut -d '=' -sf 1); do
		if [ "$(config_n_get $item auto_update 0)" = "1" ]; then
			cfgid=$(uci show ${CONFIG}.$item | head -n 1 | cut -d '.' -sf 2 | cut -d '=' -sf 1)
			remark=$(config_n_get $item remark)
			week_update=$(config_n_get $item week_update)
			time_update=$(config_n_get $item time_update)
			echo "$cfgid" >> $TMP_SUB_PATH/${week_update}_${time_update}
			echolog "配置定时任务：自动更新【$remark】订阅。"
		fi
	done

	[ -d "${TMP_SUB_PATH}" ] && {
		for name in $(ls ${TMP_SUB_PATH}); do
			week_update=$(echo $name | awk -F '_' '{print $1}')
			time_update=$(echo $name | awk -F '_' '{print $2}')
			local t="0 $time_update * * $week_update"
			[ "$week_update" = "7" ] && t="0 $time_update * * *"
			cfgids=$(echo -n $(cat ${TMP_SUB_PATH}/${name}) | sed 's# #,#g')
			echo "$t lua $APP_PATH/subscribe.lua start $cfgids > /dev/null 2>&1 &" >>/etc/crontabs/root
		done
		rm -rf $TMP_SUB_PATH
	}

	if [ "$ENABLED_DEFAULT_ACL" == 1 ] || [ "$ENABLED_ACLS" == 1 ]; then
		start_daemon=$(config_t_get global_delay start_daemon 0)
		[ "$start_daemon" = "1" ] && $APP_PATH/monitor.sh > /dev/null 2>&1 &
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
	TUN_DNS="127.0.0.1#${dns_listen_port}"

	echolog "过滤服务配置：准备接管域名解析..."
	[ "$ENABLED_ACLS" == 1 ] && {
		local items=$(uci show ${CONFIG} | grep "=acl_rule" | cut -d '.' -sf 2 | cut -d '=' -sf 1)
		[ -n "$items" ] && {
			for item in $items; do
				[ "$(config_n_get $item enabled)" = "1" ] || continue
				[ "$(config_n_get $item tcp_node)" = "default" ] && [ "$TCP_NODE" != "nil" ] && {
					local item_tcp_proxy_mode=$(config_n_get $item tcp_proxy_mode default)
					[ "$item_tcp_proxy_mode" = "default" ] && item_tcp_proxy_mode=$TCP_PROXY_MODE
					global=$(echo "${global}${item_tcp_proxy_mode}" | grep "global")
					returnhome=$(echo "${returnhome}${item_tcp_proxy_mode}" | grep "returnhome")
					chnlist=$(echo "${chnlist}${item_tcp_proxy_mode}" | grep "chnroute")
					gfwlist=$(echo "${gfwlist}${item_tcp_proxy_mode}" | grep "gfwlist")
					ACL_TCP_PROXY_MODE=${ACL_TCP_PROXY_MODE}${item_tcp_proxy_mode}
				}
				[ "$(config_n_get $item udp_node)" = "default" ] && [ "$UDP_NODE" != "nil" ] && {
					local item_udp_proxy_mode=$(config_n_get $item udp_proxy_mode default)
					[ "$item_udp_proxy_mode" = "default" ] && item_udp_proxy_mode=$UDP_PROXY_MODE
					global=$(echo "${global}${item_udp_proxy_mode}" | grep "global")
					returnhome=$(echo "${returnhome}${item_udp_proxy_mode}" | grep "returnhome")
					chnlist=$(echo "${chnlist}${item_udp_proxy_mode}" | grep "chnroute")
					gfwlist=$(echo "${gfwlist}${item_udp_proxy_mode}" | grep "gfwlist")
					ACL_UDP_PROXY_MODE=${ACL_UDP_PROXY_MODE}${item_udp_proxy_mode}
				}
			done
		}
	}

	case "$DNS_MODE" in
	dns2socks)
		local dns2socks_socks_server=$(echo $(config_t_get global socks_server 127.0.0.1:1080) | sed "s/#/:/g")
		local dns2socks_forward=$(get_first_dns REMOTE_DNS 53 | sed 's/#/:/g')
		run_dns2socks socks=$dns2socks_socks_server listen_address=127.0.0.1 listen_port=${dns_listen_port} dns=$dns2socks_forward cache=$DNS_CACHE
		echolog "  - 域名解析：dns2socks(127.0.0.1:${dns_listen_port})，${dns2socks_socks_server} -> ${dns2socks_forward}"
	;;
	sing-box)
		[ "${resolve_dns}" == "0" ] && {
			local config_file=$TMP_PATH/DNS.json
			local log_file=$TMP_PATH/DNS.log
			local log_file=/dev/null
			local _args="type=$DNS_MODE config_file=$config_file log_file=$log_file"
			[ "${DNS_CACHE}" == "0" ] && _args="${_args} dns_cache=0"
			_args="${_args} remote_dns_query_strategy=${DNS_QUERY_STRATEGY}"
			use_tcp_node_resolve_dns=1
			local v2ray_dns_mode=$(config_t_get global v2ray_dns_mode tcp)
			_args="${_args} dns_listen_port=${dns_listen_port}"
			_args="${_args} remote_dns_protocol=${v2ray_dns_mode}"
			case "$v2ray_dns_mode" in
				tcp)
					_args="${_args} remote_dns_tcp_server=${REMOTE_DNS}"
					echolog "  - 域名解析 DNS Over TCP..."
				;;
				doh)
					remote_dns_doh=$(config_t_get global remote_dns_doh "https://1.1.1.1/dns-query")
					_args="${_args} remote_dns_doh=${remote_dns_doh}"

					local _doh_url=$(echo $remote_dns_doh | awk -F ',' '{print $1}')
					local _doh_host_port=$(lua_api "get_domain_from_url(\"${_doh_url}\")")
					local _doh_host=$(echo $_doh_host_port | awk -F ':' '{print $1}')
					local _is_ip=$(lua_api "is_ip(\"${_doh_host}\")")
					local _doh_port=$(echo $_doh_host_port | awk -F ':' '{print $2}')
					[ -z "${_doh_port}" ] && _doh_port=443
					local _doh_bootstrap=$(echo $remote_dns_doh | cut -d ',' -sf 2-)
					[ "${_is_ip}" = "true" ] && _doh_bootstrap=${_doh_host}
					[ -n "${_doh_bootstrap}" ] && REMOTE_DNS=${_doh_bootstrap}:${_doh_port}
					unset _doh_url _doh_host_port _doh_host _is_ip _doh_port _doh_bootstrap
					echolog "  - 域名解析 DNS Over HTTPS..."
				;;
			esac
			_args="${_args} dns_socks_address=127.0.0.1 dns_socks_port=${tcp_node_socks_port}"
			run_singbox ${_args}
		}
	;;
	xray)
		[ "${resolve_dns}" == "0" ] && {
			local config_file=$TMP_PATH/DNS.json
			local log_file=$TMP_PATH/DNS.log
			local log_file=/dev/null
			local _args="type=$DNS_MODE config_file=$config_file log_file=$log_file"
			[ "${DNS_CACHE}" == "0" ] && _args="${_args} dns_cache=0"
			_args="${_args} dns_query_strategy=${DNS_QUERY_STRATEGY}"
			local _dns_client_ip=$(config_t_get global dns_client_ip)
			[ -n "${_dns_client_ip}" ] && _args="${_args} dns_client_ip=${_dns_client_ip}"
			use_tcp_node_resolve_dns=1
			_args="${_args} dns_listen_port=${dns_listen_port}"
			_args="${_args} remote_dns_tcp_server=${REMOTE_DNS}"
			local v2ray_dns_mode=$(config_t_get global v2ray_dns_mode tcp)
			[ "$v2ray_dns_mode" = "tcp+doh" ] && {
				remote_dns_doh=$(config_t_get global remote_dns_doh "https://1.1.1.1/dns-query")
				_args="${_args} remote_dns_doh=${remote_dns_doh}"
			}
			_args="${_args} dns_socks_address=127.0.0.1 dns_socks_port=${tcp_node_socks_port}"
			run_xray ${_args}
		}
	;;
	dns2tcp)
		use_tcp_node_resolve_dns=1
		ln_run "$(first_type dns2tcp)" dns2tcp "/dev/null" -L "${TUN_DNS}" -R "$(get_first_dns REMOTE_DNS 53)" -v
		echolog "  - 域名解析：dns2tcp + 使用(TCP节点)解析域名..."
	;;
	udp)
		use_udp_node_resolve_dns=1
		TUN_DNS="$(echo ${REMOTE_DNS} | sed 's/#/:/g' | sed -E 's/\:([^:]+)$/#\1/g')"
		echolog "  - 域名解析：使用UDP协议请求DNS（$TUN_DNS）..."
	;;
	esac

	[ "${use_tcp_node_resolve_dns}" = "1" ] && echolog "  * 请确认上游 DNS 支持 TCP 查询，如非直连地址，确保 TCP 代理打开，并且已经正确转发！"
	[ "${use_udp_node_resolve_dns}" = "1" ] && echolog "  * 要求代理 DNS 请求，如上游 DNS 非直连地址，确保 UDP 代理打开，并且已经正确转发！"

	[ "$CHINADNS_NG" = "1" ] && [ -n "$(first_type chinadns-ng)" ] && ([ -n "$chnlist" ] || [ -n "$gfwlist" ]) && {
		[ "$FILTER_PROXY_IPV6" = "1" ] && {
			local _no_ipv6_rules="gt"
		}
		local china_ng_listen_port=$(expr $dns_listen_port + 1)
		local china_ng_listen="127.0.0.1#${china_ng_listen_port}"
		run_chinadns_ng \
			_listen_port=${china_ng_listen_port} \
			_dns_china=$(echo -n $(echo "${LOCAL_DNS}" | sed "s/,/\n/g" | head -n2) | tr " " ",") \
			_dns_trust="${TUN_DNS}" \
			_chnlist="${chnlist}" \
			_gfwlist="${gfwlist}" \
			_no_ipv6_rules="${_no_ipv6_rules}" \
			_log_path="${TMP_PATH}/chinadns-ng.log"

		WHEN_CHNROUTE_DEFAULT_DNS="chinadns_ng"
	}

	[ "$DNS_SHUNT" = "dnsmasq" ] && {
		[ "$WHEN_CHNROUTE_DEFAULT_DNS" = "remote" ] && {
			dnsmasq_version=$(dnsmasq -v | grep -i "Dnsmasq version " | awk '{print $3}')
			[ "$(expr $dnsmasq_version \>= 2.87)" == 0 ] && echolog "Dnsmasq版本低于2.87，有可能无法正常使用！！！"
		}
		source $APP_PATH/helper_dnsmasq.sh stretch
		lua $APP_PATH/helper_dnsmasq_add.lua -FLAG "default" -TMP_DNSMASQ_PATH ${TMP_DNSMASQ_PATH} \
			-DNSMASQ_CONF_FILE "/tmp/dnsmasq.d/dnsmasq-passwall.conf" -DEFAULT_DNS ${DEFAULT_DNS} -LOCAL_DNS ${LOCAL_DNS} \
			-TUN_DNS ${TUN_DNS} -REMOTE_FAKEDNS ${fakedns:-0} -CHNROUTE_MODE_DEFAULT_DNS "${WHEN_CHNROUTE_DEFAULT_DNS:-direct}" -CHINADNS_DNS ${china_ng_listen:-0} \
			-TCP_NODE ${TCP_NODE} -PROXY_MODE "${TCP_PROXY_MODE}${LOCALHOST_TCP_PROXY_MODE}${ACL_TCP_PROXY_MODE}" -NO_PROXY_IPV6 ${FILTER_PROXY_IPV6:-0} -NFTFLAG ${nftflag:-0} \
			-NO_LOGIC_LOG ${NO_LOGIC_LOG:-0}
	}
}

add_ip2route() {
	local ip=$(get_host_ip "ipv4" $1)
	[ -z "$ip" ] && {
		echolog "  - 无法解析[${1}]，路由表添加失败！"
		return 1
	}
	local remarks="${1}"
	[ "$remarks" != "$ip" ] && remarks="${1}(${ip})"

	. /lib/functions/network.sh
	local gateway device
	network_get_gateway gateway "$2"
	network_get_device device "$2"
	[ -z "${device}" ] && device="$2"

	if [ -n "${gateway}" ]; then
		route add -host ${ip} gw ${gateway} dev ${device} >/dev/null 2>&1
		echo "$ip" >> $TMP_ROUTE_PATH/${device}
		echolog "  - [${remarks}]添加到接口[${device}]路由表成功！"
	else
		echolog "  - [${remarks}]添加到接口[${device}]路由表失功！原因是找不到[${device}]网关。"
	fi
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
	[ "$(config_t_get global_haproxy balancing_enable 0)" != "1" ] && return
	haproxy_path=${TMP_PATH}/haproxy
	haproxy_conf="config.cfg"
	lua $APP_PATH/haproxy.lua -path ${haproxy_path} -conf ${haproxy_conf} -dns ${LOCAL_DNS}
	ln_run "$(first_type haproxy)" haproxy "/dev/null" -f "${haproxy_path}/${haproxy_conf}"
}

kill_all() {
	kill -9 $(pidof "$@") >/dev/null 2>&1
}

acl_app() {
	local items=$(uci show ${CONFIG} | grep "=acl_rule" | cut -d '.' -sf 2 | cut -d '=' -sf 1)
	[ -n "$items" ] && {
		local item
		local socks_port redir_port dns_port dnsmasq_port chinadns_port
		local msg msg2
		socks_port=11100
		redir_port=11200
		dns_port=11300
		dnsmasq_port=11400
		chinadns_port=11500
		for item in $items; do
			local enabled sid remarks sources tcp_proxy_mode udp_proxy_mode tcp_node udp_node filter_proxy_ipv6 dns_mode remote_dns v2ray_dns_mode remote_dns_doh dns_client_ip
			local _ip _mac _iprange _ipset _ip_or_mac rule_list tcp_port udp_port config_file _extra_param
			sid=$(uci -q show "${CONFIG}.${item}" | grep "=acl_rule" | awk -F '=' '{print $1}' | awk -F '.' '{print $2}')
			eval $(uci -q show "${CONFIG}.${item}" | cut -d'.' -sf 3-)
			[ "$enabled" = "1" ] || continue

			[ -z "${sources}" ] && continue
			for s in $sources; do
				is_iprange=$(lua_api "iprange(\"${s}\")")
				if [ "${is_iprange}" = "true" ]; then
					rule_list="${rule_list}\niprange:${s}"
				elif [ -n "$(echo ${s} | grep '^ipset:')" ]; then
					rule_list="${rule_list}\nipset:${s}"
				else
					_ip_or_mac=$(lua_api "ip_or_mac(\"${s}\")")
					if [ "${_ip_or_mac}" = "ip" ]; then
						rule_list="${rule_list}\nip:${s}"
					elif [ "${_ip_or_mac}" = "mac" ]; then
						rule_list="${rule_list}\nmac:${s}"
					fi
				fi
			done
			[ -z "${rule_list}" ] && continue
			mkdir -p $TMP_ACL_PATH/$sid
			echo -e "${rule_list}" | sed '/^$/d' > $TMP_ACL_PATH/$sid/rule_list

			tcp_proxy_mode=${tcp_proxy_mode:-default}
			udp_proxy_mode=${udp_proxy_mode:-default}
			tcp_node=${tcp_node:-default}
			udp_node=${udp_node:-default}
			filter_proxy_ipv6=${filter_proxy_ipv6:-0}
			dns_mode=${dns_mode:-dns2socks}
			remote_dns=${remote_dns:-1.1.1.1}
			chinadns_ng=${chinadns_ng:-0}
			when_chnroute_default_dns=${when_chnroute_default_dns:-direct}
			[ "$dns_mode" = "sing-box" ] && {
				[ "$v2ray_dns_mode" = "doh" ] && remote_dns=${remote_dns_doh:-https://1.1.1.1/dns-query}
			}
			[ "$tcp_proxy_mode" = "default" ] && tcp_proxy_mode=$TCP_PROXY_MODE
			[ "$udp_proxy_mode" = "default" ] && udp_proxy_mode=$UDP_PROXY_MODE

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
									run_dns2socks flag=acl_${sid} socks_address=127.0.0.1 socks_port=$socks_port listen_address=0.0.0.0 listen_port=${_dns_port} dns=$remote_dns cache=1
								elif [ "$dns_mode" = "sing-box" -o "$dns_mode" = "xray" ]; then
									config_file=$TMP_ACL_PATH/${tcp_node}_SOCKS_${socks_port}_DNS.json
									[ "$dns_mode" = "xray" ] && [ "$v2ray_dns_mode" = "tcp+doh" ] && remote_dns_doh=${remote_dns_doh:-https://1.1.1.1/dns-query}
									local type=${dns_mode}
									[ "${dns_mode}" = "sing-box" ] && type="singbox"
									run_${type} flag=acl_${sid} type=$dns_mode dns_socks_address=127.0.0.1 dns_socks_port=$socks_port dns_listen_port=${_dns_port} remote_dns_protocol=${v2ray_dns_mode} remote_dns_tcp_server=${remote_dns} remote_dns_doh="${remote_dns_doh}" remote_dns_query_strategy=${DNS_QUERY_STRATEGY} dns_client_ip=${dns_client_ip} dns_query_strategy=${DNS_QUERY_STRATEGY} config_file=$config_file
								fi
								eval node_${tcp_node}_$(echo -n "${remote_dns}" | md5sum | cut -d " " -f1)=${_dns_port}
							}

							[ "$chinadns_ng" = "1" ] && [ -n "$(first_type chinadns-ng)" ] && ([ "$tcp_proxy_mode" = "chnroute" ] || [ "$tcp_proxy_mode" = "gfwlist" ]) && {
								[ "$filter_proxy_ipv6" = "1" ] && {
									local _no_ipv6_rules="gt"
								}
								chinadns_port=$(expr $chinadns_port + 1)
								_china_ng_listen="127.0.0.1#${chinadns_port}"

								run_chinadns_ng \
									_listen_port=${chinadns_port} \
									_dns_china=$(echo -n $(echo "${LOCAL_DNS}" | sed "s/,/\n/g" | head -n2) | tr " " ",") \
									_dns_trust="127.0.0.1#${_dns_port}" \
									_chnlist=$(echo "${tcp_proxy_mode}" | grep "chnroute") \
									_gfwlist=$(echo "${tcp_proxy_mode}" | grep "gfwlist") \
									_no_ipv6_rules="${_no_ipv6_rules}" \
									_log_path="${TMP_ACL_PATH}/${sid}/chinadns-ng.log" \
									_no_logic_log=1

								when_chnroute_default_dns="chinadns_ng"
							}

							dnsmasq_port=$(get_new_port $(expr $dnsmasq_port + 1))
							redirect_dns_port=$dnsmasq_port
							mkdir -p $TMP_ACL_PATH/$sid/dnsmasq.d
							default_dnsmasq_cfgid=$(uci show dhcp.@dnsmasq[0] |  awk -F '.' '{print $2}' | awk -F '=' '{print $1}'| head -1)
							[ -s "/tmp/etc/dnsmasq.conf.${default_dnsmasq_cfgid}" ] && {
								cp -r /tmp/etc/dnsmasq.conf.${default_dnsmasq_cfgid} $TMP_ACL_PATH/$sid/dnsmasq.conf
								sed -i "/ubus/d" $TMP_ACL_PATH/$sid/dnsmasq.conf
								sed -i "/dhcp/d" $TMP_ACL_PATH/$sid/dnsmasq.conf
								sed -i "/port=/d" $TMP_ACL_PATH/$sid/dnsmasq.conf
								sed -i "/conf-dir/d" $TMP_ACL_PATH/$sid/dnsmasq.conf
							}
							echo "port=${dnsmasq_port}" >> $TMP_ACL_PATH/$sid/dnsmasq.conf
							d_server=127.0.0.1
							[ "$tcp_proxy_mode" = "global" ] && {
								d_server=${d_server}#${_dns_port}
								sed -i "/no-poll/d" $TMP_ACL_PATH/$sid/dnsmasq.conf
								sed -i "/no-resolv/d" $TMP_ACL_PATH/$sid/dnsmasq.conf
								echo "server=${d_server}" >> $TMP_ACL_PATH/$sid/dnsmasq.conf
								echo "no-poll" >> $TMP_ACL_PATH/$sid/dnsmasq.conf
								echo "no-resolv" >> $TMP_ACL_PATH/$sid/dnsmasq.conf
							}
							[ "$when_chnroute_default_dns" = "remote" ] && {
								dnsmasq_version=$(dnsmasq -v | grep -i "Dnsmasq version " | awk '{print $3}')
								[ "$(expr $dnsmasq_version \>= 2.87)" == 0 ] && echolog "Dnsmasq版本低于2.87，有可能无法正常使用！！！"
							}
							lua $APP_PATH/helper_dnsmasq_add.lua -FLAG ${sid} -TMP_DNSMASQ_PATH $TMP_ACL_PATH/$sid/dnsmasq.d \
								-DNSMASQ_CONF_FILE $TMP_ACL_PATH/$sid/dnsmasq.conf -DEFAULT_DNS $DEFAULT_DNS -LOCAL_DNS $LOCAL_DNS \
								-TUN_DNS "127.0.0.1#${_dns_port}" -REMOTE_FAKEDNS 0 -CHNROUTE_MODE_DEFAULT_DNS "${when_chnroute_default_dns:-direct}" -CHINADNS_DNS ${_china_ng_listen:-0} \
								-TCP_NODE $tcp_node -PROXY_MODE ${tcp_proxy_mode} -NO_PROXY_IPV6 ${filter_proxy_ipv6:-0} -NFTFLAG ${nftflag:-0} \
								-NO_LOGIC_LOG 1
							ln_run "$(first_type dnsmasq)" "dnsmasq_${sid}" "/dev/null" -C $TMP_ACL_PATH/$sid/dnsmasq.conf -x $TMP_ACL_PATH/$sid/dnsmasq.pid
							eval node_${tcp_node}_$(echo -n "${tcp_proxy_mode}${remote_dns}" | md5sum | cut -d " " -f1)=${dnsmasq_port}
						}
						if [ "$tcp_node" = "$TCP_NODE" ]; then
							tcp_port=$TCP_REDIR_PORT
						else
							_redir_port=$(eval echo \${node_${tcp_node}_redir_port})
							_socks_port=$(eval echo \${node_${tcp_node}_socks_port})
							if [ -n "${_socks_port}" ] && [ -n "${_redir_port}" ]; then
								socks_port=${_socks_port}
								tcp_port=${_redir_port}
								_dnsmasq_port=$(eval echo \${node_${tcp_node}_$(echo -n "${tcp_proxy_mode}${remote_dns}" | md5sum | cut -d " " -f1)})
								if [ -z "${_dnsmasq_port}" ]; then
									_dns_port=$(eval echo \${node_${tcp_node}_$(echo -n "${remote_dns}" | md5sum | cut -d " " -f1)})
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

								local type=$(echo $(config_n_get $tcp_node type) | tr 'A-Z' 'a-z')
								if [ -n "${type}" ] && ([ "${type}" = "sing-box" ] || [ "${type}" = "xray" ]); then
									config_file="acl/${tcp_node}_TCP_${redir_port}.json"
									_extra_param="socks_address=127.0.0.1 socks_port=$socks_port"
									if [ "$dns_mode" = "sing-box" ] || [ "$dns_mode" = "xray" ]; then
										dns_port=$(get_new_port $(expr $dns_port + 1))
										_dns_port=$dns_port
										config_file=$(echo $config_file | sed "s/TCP_/DNS_${_dns_port}_TCP_/g")
										remote_dns_doh=${remote_dns}
										[ "$dns_mode" = "xray" ] && [ "$v2ray_dns_mode" = "tcp+doh" ] && remote_dns_doh=${remote_dns_doh:-https://1.1.1.1/dns-query}
										_extra_param="dns_listen_port=${_dns_port} remote_dns_protocol=${v2ray_dns_mode} remote_dns_tcp_server=${remote_dns} remote_dns_doh=${remote_dns_doh} remote_dns_query_strategy=${DNS_QUERY_STRATEGY} dns_client_ip=${dns_client_ip} dns_query_strategy=${DNS_QUERY_STRATEGY}"
									fi
									[ "$udp_node" != "nil" ] && ([ "$udp_node" = "tcp" ] || [ "$udp_node" = "$tcp_node" ]) && {
										config_file=$(echo $config_file | sed "s/TCP_/TCP_UDP_/g")
										_extra_param="${_extra_param} udp_redir_port=$redir_port"
									}
									config_file="$TMP_PATH/$config_file"
									[ "${type}" = "sing-box" ] && type="singbox"
									run_${type} flag=$tcp_node node=$tcp_node tcp_redir_port=$redir_port ${_extra_param} config_file=$config_file
								else
									config_file="acl/${tcp_node}_SOCKS_${socks_port}.json"
									run_socks flag=$tcp_node node=$tcp_node bind=127.0.0.1 socks_port=$socks_port config_file=$config_file
									local log_file=$TMP_ACL_PATH/ipt2socks_${tcp_node}_${redir_port}.log
									log_file="/dev/null"
									run_ipt2socks flag=acl_${tcp_node} tcp_tproxy=${is_tproxy} local_port=$redir_port socks_address=127.0.0.1 socks_port=$socks_port log_file=$log_file
								fi
								run_dns ${_dns_port}
							fi
							echo "${tcp_node}" > $TMP_ACL_PATH/$sid/var_tcp_node
						fi
					}
				fi
				echo "${tcp_port}" > $TMP_ACL_PATH/$sid/var_tcp_port
			}
			[ "$udp_node" != "nil" ] && {
				[ "$udp_node" = "tcp" ] && udp_node=$tcp_node
				if [ "$udp_node" = "default" ]; then
					if [ "$TCP_UDP" = "0" ] && [ "$UDP_NODE" = "nil" ]; then
						udp_node="nil"
						unset udp_port
					elif [ "$TCP_UDP" = "1" ] && [ "$udp_node" = "nil" ]; then
						udp_node=$TCP_NODE
						udp_port=$TCP_REDIR_PORT
					else
						udp_node=$UDP_NODE
						udp_port=$UDP_REDIR_PORT
					fi
				elif [ "$udp_node" = "$tcp_node" ]; then
					udp_node=$tcp_node
					udp_port=$tcp_port
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

								local type=$(echo $(config_n_get $udp_node type) | tr 'A-Z' 'a-z')
								if [ -n "${type}" ] && ([ "${type}" = "sing-box" ] || [ "${type}" = "xray" ]); then
									config_file="acl/${udp_node}_UDP_${redir_port}.json"
									config_file="$TMP_PATH/$config_file"
									[ "${type}" = "sing-box" ] && type="singbox"
									run_${type} flag=$udp_node node=$udp_node udp_redir_port=$redir_port config_file=$config_file
								else
									config_file="acl/${udp_node}_SOCKS_${socks_port}.json"
									run_socks flag=$udp_node node=$udp_node bind=127.0.0.1 socks_port=$socks_port config_file=$config_file
									local log_file=$TMP_ACL_PATH/ipt2socks_${udp_node}_${redir_port}.log
									log_file="/dev/null"
									run_ipt2socks flag=acl_${udp_node} local_port=$redir_port socks_address=127.0.0.1 socks_port=$socks_port log_file=$log_file
								fi
							fi
							echo "${udp_node}" > $TMP_ACL_PATH/$sid/var_udp_node
						fi
					}
				fi
				echo "${udp_port}" > $TMP_ACL_PATH/$sid/var_udp_port
				udp_flag=1
			}
			[ -n "$redirect_dns_port" ] && echo "${redirect_dns_port}" > $TMP_ACL_PATH/$sid/var_redirect_dns_port
			unset enabled sid remarks sources tcp_proxy_mode udp_proxy_mode tcp_node udp_node filter_proxy_ipv6 dns_mode remote_dns v2ray_dns_mode remote_dns_doh dns_client_ip
			unset _ip _mac _iprange _ipset _ip_or_mac rule_list tcp_port udp_port config_file _extra_param
			unset _china_ng_listen _china_ng_chn _china_ng_gfw _gfwlist_file _chnlist_file _china_ng_log_file _no_ipv6_rules _china_ng_extra_param
			unset redirect_dns_port
		done
		unset socks_port redir_port dns_port dnsmasq_port chinadns_port
	}
}

start() {
	ulimit -n 65535
	start_haproxy
	start_socks
	nftflag=0
	local use_nft=$(config_t_get global_forwarding use_nft 0)
	local USE_TABLES
	if [ "$use_nft" == 0 ]; then
		if [ -n "$(command -v iptables-legacy || command -v iptables)" ] && [ -n "$(command -v ipset)" ] && [ -n "$(dnsmasq --version | grep 'Compile time options:.* ipset')" ]; then
			USE_TABLES="iptables"
		else
			if [ -n "$(command -v fw4)" ] && [ -n "$(command -v nft)" ] && [ -n "$(dnsmasq --version | grep 'Compile time options:.* nftset')" ]; then
				echolog "检测到fw4，使用nftables进行透明代理。"
				USE_TABLES="nftables"
				nftflag=1
				config_t_set global_forwarding use_nft 1
				uci commit ${CONFIG}
			else
				echolog "系统未安装iptables或ipset或Dnsmasq没有开启ipset支持，无法透明代理！"
			fi
		fi
	else
		if [ -n "$(dnsmasq --version | grep 'Compile time options:.* nftset')" ]; then
			USE_TABLES="nftables"
			nftflag=1
		else
			echolog "Dnsmasq软件包不满足nftables透明代理要求，如需使用请确保dnsmasq版本在2.87以上并开启nftset支持。"
		fi
	fi

	check_depends $USE_TABLES

	[ "$ENABLED_DEFAULT_ACL" == 1 ] && {
		start_redir TCP
		start_redir UDP
		start_dns
	}
	[ -n "$USE_TABLES" ] && source $APP_PATH/${USE_TABLES}.sh start
	[ "$ENABLED_DEFAULT_ACL" == 1 ] && source $APP_PATH/helper_${DNS_N}.sh logic_restart
	start_crontab
	echolog "运行完成！\n"
}

stop() {
	clean_log
	[ -n "$($(source $APP_PATH/iptables.sh get_ipt_bin) -t mangle -t nat -L -nv 2>/dev/null | grep "PSW")" ] && source $APP_PATH/iptables.sh stop
	[ -n "$(nft list chains 2>/dev/null | grep "PSW")" ] && source $APP_PATH/nftables.sh stop
	delete_ip2route
	kill_all v2ray-plugin obfs-local
	pgrep -f "sleep.*(6s|9s|58s)" | xargs kill -9 >/dev/null 2>&1
	pgrep -af "${CONFIG}/" | awk '! /app\.sh|subscribe\.lua|rule_update\.lua/{print $1}' | xargs kill -9 >/dev/null 2>&1
	unset V2RAY_LOCATION_ASSET
	unset XRAY_LOCATION_ASSET
	stop_crontab
	source $APP_PATH/helper_dnsmasq.sh del
	source $APP_PATH/helper_dnsmasq.sh restart no_log=1
	[ -s "$TMP_PATH/bridge_nf_ipt" ] && sysctl -w net.bridge.bridge-nf-call-iptables=$(cat $TMP_PATH/bridge_nf_ipt) >/dev/null 2>&1
	[ -s "$TMP_PATH/bridge_nf_ip6t" ] && sysctl -w net.bridge.bridge-nf-call-ip6tables=$(cat $TMP_PATH/bridge_nf_ip6t) >/dev/null 2>&1
	rm -rf ${TMP_PATH}
	rm -rf /tmp/lock/${CONFIG}_socks_auto_switch*
	echolog "清空并关闭相关程序和缓存完成。"
	exit 0
}

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
[ "$ENABLED" == 1 ] && {
	[ "$TCP_NODE" != "nil" ] && [ "$(config_get_type $TCP_NODE nil)" != "nil" ] && ENABLED_DEFAULT_ACL=1
	[ "$UDP_NODE" != "nil" ] && [ "$(config_get_type $UDP_NODE nil)" != "nil" ] && ENABLED_DEFAULT_ACL=1
}
ENABLED_ACLS=$(config_t_get global acl_enable 0)
[ "$ENABLED_ACLS" == 1 ] && {
	[ "$(uci show ${CONFIG} | grep "@acl_rule" | grep "enabled='1'" | wc -l)" == 0 ] && ENABLED_ACLS=0
}

tcp_proxy_way=$(config_t_get global_forwarding tcp_proxy_way redirect)
TCP_REDIR_PORTS=$(config_t_get global_forwarding tcp_redir_ports '80,443')
UDP_REDIR_PORTS=$(config_t_get global_forwarding udp_redir_ports '1:65535')
TCP_NO_REDIR_PORTS=$(config_t_get global_forwarding tcp_no_redir_ports 'disable')
UDP_NO_REDIR_PORTS=$(config_t_get global_forwarding udp_no_redir_ports 'disable')
TCP_PROXY_DROP_PORTS=$(config_t_get global_forwarding tcp_proxy_drop_ports 'disable')
UDP_PROXY_DROP_PORTS=$(config_t_get global_forwarding udp_proxy_drop_ports '80,443')
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
DNS_SHUNT="dnsmasq"
DNS_MODE=$(config_t_get global dns_mode dns2tcp)
DNS_CACHE=$(config_t_get global dns_cache 0)
REMOTE_DNS=$(config_t_get global remote_dns 1.1.1.1:53 | sed 's/#/:/g' | sed -E 's/\:([^:]+)$/#\1/g')
CHINADNS_NG=$(config_t_get global chinadns_ng 0)
WHEN_CHNROUTE_DEFAULT_DNS=$(config_t_get global when_chnroute_default_dns direct)
FILTER_PROXY_IPV6=$(config_t_get global filter_proxy_ipv6 0)
dns_listen_port=${DNS_PORT}

REDIRECT_LIST="socks ss ss-rust ssr sing-box xray trojan-go trojan-plus naiveproxy hysteria2"
TPROXY_LIST="brook socks ss ss-rust ssr sing-box xray trojan-go trojan-plus hysteria2"
RESOLVFILE=/tmp/resolv.conf.d/resolv.conf.auto
[ -f "${RESOLVFILE}" ] && [ -s "${RESOLVFILE}" ] || RESOLVFILE=/tmp/resolv.conf.auto

ISP_DNS=$(cat $RESOLVFILE 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | sort -u | grep -v 0.0.0.0 | grep -v 127.0.0.1)
ISP_DNS6=$(cat $RESOLVFILE 2>/dev/null | grep -E "([A-Fa-f0-9]{1,4}::?){1,7}[A-Fa-f0-9]{1,4}" | awk -F % '{print $1}' | awk -F " " '{print $2}'| sort -u | grep -v -Fx ::1 | grep -v -Fx ::)

DEFAULT_DNS=$(uci show dhcp | grep "@dnsmasq" | grep "\.server=" | awk -F '=' '{print $2}' | sed "s/'//g" | tr ' ' '\n' | grep -v "\/" | head -2 | sed ':label;N;s/\n/,/;b label')
[ -z "${DEFAULT_DNS}" ] && [ "$(echo $ISP_DNS | tr ' ' '\n' | wc -l)" -le 2 ] && DEFAULT_DNS=$(echo -n $ISP_DNS | tr ' ' '\n' | head -2 | tr '\n' ',')
LOCAL_DNS="${DEFAULT_DNS:-119.29.29.29,223.5.5.5}"

PROXY_IPV6=$(config_t_get global_forwarding ipv6_tproxy 0)
DNS_QUERY_STRATEGY="UseIPv4"
[ "$PROXY_IPV6" = "1" ] && DNS_QUERY_STRATEGY="UseIP"

export V2RAY_LOCATION_ASSET=$(config_t_get global_rules v2ray_location_asset "/usr/share/v2ray/")
export XRAY_LOCATION_ASSET=$V2RAY_LOCATION_ASSET
mkdir -p /tmp/etc $TMP_PATH $TMP_BIN_PATH $TMP_SCRIPT_FUNC_PATH $TMP_ID_PATH $TMP_PORT_PATH $TMP_ROUTE_PATH $TMP_ACL_PATH $TMP_IFACE_PATH $TMP_PATH2

arg1=$1
shift
case $arg1 in
add_ip2route)
	add_ip2route $@
	;;
get_new_port)
	get_new_port $@
	;;
run_socks)
	run_socks $@
	;;
run_redir)
	run_redir $@
	;;
socks_node_switch)
	socks_node_switch $@
	;;
echolog)
	echolog $@
	;;
stop)
	stop
	;;
start)
	start
	;;
esac
