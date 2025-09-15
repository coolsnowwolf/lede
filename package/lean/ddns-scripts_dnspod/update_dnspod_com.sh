#!/bin/sh

#检查传入参数
[ -z "$username" ] && write_log 14 "Configuration error! [User name] cannot be empty"
[ -z "$password" ] && write_log 14 "Configuration error! [Password] cannot be empty"

#检查外部调用工具
WGET_SSL='wget'
[ -n "$WGET_SSL" ] || write_log 13 "GNU Wget support is required to use dnspod API. Please install first"

# 变量声明
local __URLBASE __HOST __DOMAIN __TYPE __CMDBASE __POST __POST1 __RECIP __RECID __value __TTL
__URLBASE="https://api.dnspod.com/"

# 从 $domain 分离主机和域名
[ "${domain:0:2}" == "@." ] && domain="${domain/./}" # 主域名处理
[ "$domain" == "${domain/@/}" ] && domain="${domain/./@}" # 未找到分隔符，兼容常用域名格式
__HOST="${domain%%@*}"
__DOMAIN="${domain#*@}"
[ -z "$__HOST" -o "$__HOST" == "$__DOMAIN" ] && __HOST="@"

# 设置记录类型
[ $use_ipv6 -eq 0 ] && __TYPE="A" || __TYPE="AAAA"

# 构造基本通信命令
build_command() {
	__CMDBASE="$WGET_SSL --no-hsts -nv -t 1 -O $DATFILE -o $ERRFILE"
	# 绑定用于通信的主机/IP
	if [ -n "$bind_network" ]; then
		local bind_ip run_prog
		[ $use_ipv6 -eq 0 ] && run_prog="network_get_ipaddr" || run_prog="network_get_ipaddr6"
		eval "$run_prog bind_ip $bind_network" || \
			write_log 13 "Unable to get local IP address with '$run_prog $ bind_network' - error code: '$?'"
		write_log 7 "Forced use of IP '$bind_ip' communication"
		__CMDBASE="$__CMDBASE --bind-address=$bind_ip"
	fi
	# 强制设定IP版本
	if [ $force_ipversion -eq 1 ]; then
		[ $use_ipv6 -eq 0 ] && __CMDBASE="$__CMDBASE -4" || __CMDBASE="$__CMDBASE -6"
	fi
	# 设置CA证书参数
	if [ $use_https -eq 1 ]; then
		if [ "$cacert" = "IGNORE" ]; then
			__CMDBASE="$__CMDBASE --no-check-certificate"
		elif [ -f "$cacert" ]; then
			__CMDBASE="$__CMDBASE --ca-certificate=${cacert}"
		elif [ -d "$cacert" ]; then
			__CMDBASE="$__CMDBASE --ca-directory=${cacert}"
		elif [ -n "$cacert" ]; then
			write_log 14 "A valid certificate for HTTPS communication was not found in '$cacert'"
		fi
	fi
	# 如果没有设置，禁用代理 (这可能是 .wgetrc 或环境设置错误)
	[ -z "$proxy" ] && __CMDBASE="$__CMDBASE --no-proxy"
	__CMDBASE="$__CMDBASE --post-data"
}

# 用于Dnspod API的通信函数
dnspod_transfer() {
	local __A=$1
	local __CNT=0
	local __ERR=0
	local __B PID_SLEEP
	case "$__A" in
		0)
		__B="$__CMDBASE '$__POST' ${__URLBASE}Record.List"
			;;
		1)
		__B="$__CMDBASE '$__POST1' ${__URLBASE}Record.Create"
			;;
		2)
		__B="$__CMDBASE '$__POST1&record_id=$__RECID&ttl=$__TTL' ${__URLBASE}Record.Modify"
			;;
	esac

	while : ; do
		write_log 7 "#> $__B"
		eval $__B
		__ERR=`jsonfilter -i $DATFILE -e "@.status.code"`

		[ $__ERR -eq 1 ] && return 0
		[ $__A -eq 0 ] && [ $__ERR -eq 10 ] && return 0
		write_log 3 "Error message:[$(jsonfilter -i $DATFILE -e "@.status.message")]"

		if [ $VERBOSE -gt 1 ]; then
			write_log 4 "Transfer failed - detailed mode: $VERBOSE - Do not try again after an error"
			return 1
		fi

		__CNT=$(( $__CNT + 1 ))
		[ $retry_count -gt 0 -a $__CNT -gt $retry_count ] && write_log 14 "Transfer failed after $retry_count retries"

		write_log 4 "Transfer failed - $__CNT Try again in $RETRY_SECONDS seconds"
		sleep $RETRY_SECONDS &
		PID_SLEEP=$!
		wait $PID_SLEEP
		PID_SLEEP=0
	done
}

#添加解析记录
add_domain() {
	dnspod_transfer 1
	write_log 7 "Add new parsing record [${__HOST}.${__DOMAIN}],[type:$__TYPE],[ip:$__IP] successfully!"
	return 0
}

#修改解析记录
update_domain() {
	dnspod_transfer 2
	write_log 7 "Modify new parsing record [${__HOST}.${__DOMAIN}],[type:$__TYPE],[ip:$__IP],[TTL:$__TTL] successfully!"
	return 0
}

#获取域名解析记录
describe_domain() {
	ret=0
	__POST="login_token=$username,$password&format=json&domain=$__DOMAIN&sub_domain=$__HOST&record_type=$__TYPE"
	__POST1="$__POST&value=$__IP&record_type=$__TYPE&record_line_id=0"
	dnspod_transfer 0
	__TMP=`jsonfilter -i $DATFILE -e "@.records[@.type!='NS']"`
	__value=`jsonfilter -s "$__TMP" -e "@.name"`
	if [ "$__value" == "" ]; then
		write_log 7 "Parsing record:[${__HOST}.${__DOMAIN}] does not exist"
		ret=1
	else
		__RECID=`jsonfilter -s "$__TMP" -e "@.id"`
		__value=`jsonfilter -s "$__TMP" -e "@.type"`
		__TTL=`jsonfilter -s "$__TMP" -e "@.ttl"`
		write_log 7 "Get parsing recordID:[$__RECID]"
		if [ "$__value" != "$__TYPE" ]; then
			write_log 7 "Current parsing [type:$__TYPE], get mismatched [type:$__value]"
			write_log 7 "Address needs to be modified, local address:[$__IP]"
			ret=2
		else
			__RECIP=`jsonfilter -s "$__TMP" -e "@.value"`
			if [ "$__RECIP" != "$__IP" ]; then
				write_log 7 "Address needs to be modified, local address:[$__IP]"
				ret=2
			fi
		fi
	fi
}

build_command
describe_domain
if [ $ret == 1 ];then
	sleep 3 && add_domain
elif [ $ret == 2 ];then
	sleep 3 && update_domain
else
	write_log 7 "Local IP:[$__IP] parsing records IP:[$__RECIP] address does not need to be modified"
fi

return 0
