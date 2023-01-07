#!/bin/sh
#
# 用于阿里云解析的DNS更新脚本
# 2017-2018 Sense <sensec at gmail dot com>
# 阿里云解析API文档 https://help.aliyun.com/document_detail/29739.html
#
# 本脚本由 dynamic_dns_functions.sh 内的函数 send_update() 调用
#
# 需要在 /etc/config/ddns 中设置的选项
# option username - 阿里云API访问账号 Access Key ID。可通过 aliyun.com 帐号管理的 accesskeys 获取, 或者访问 https://ak-console.aliyun.com
# option password - 阿里云API访问密钥 Access Key Secret
# option domain   - 完整的域名。建议主机与域名之间使用 @符号 分隔，否则将以第一个 .符号 之前的内容作为主机名
#

# 检查传入参数
[ -z "$username" ] && write_log 14 "配置错误！保存阿里云API访问账号的'用户名'不能为空"
[ -z "$password" ] && write_log 14 "配置错误！保存阿里云API访问密钥的'密码'不能为空"

# 检查外部调用工具
WGET_SSL='wget'
[ -n "$WGET_SSL" ] || write_log 13 "使用阿里云API需要 GNU Wget 支持，请先安装"
command -v sed >/dev/null 2>&1 || write_log 13 "使用阿里云API需要 sed 支持，请先安装"
command -v openssl >/dev/null 2>&1 || write_log 13 "使用阿里云API需要 openssl-util 支持，请先安装"

# 包含用于解析 JSON 格式返回值的函数
. /usr/share/libubox/jshn.sh

# 变量声明
local __HOST __DOMAIN __TYPE __URLBASE __CMDBASE __URLARGS __SEPARATOR __RECID
[ $use_https -eq 0 ] && __URLBASE="http://alidns.aliyuncs.com/" || __URLBASE="https://alidns.aliyuncs.com/"
__SEPARATOR="&"

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
			write_log 13 "无法使用 '$run_prog $bind_network' 获取本地IP地址 - 错误代码: '$?'"
		write_log 7 "强制使用IP '$bind_ip' 通信"
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
			write_log 14 "在 '$cacert' 中未找到用于 HTTPS 通信的有效证书"
		fi
	fi
	# 如果没有设置，禁用代理 (这可能是 .wgetrc 或环境设置错误)
	[ -z "$proxy" ] && __CMDBASE="$__CMDBASE --no-proxy"
}

# 用于阿里云API的通信函数
aliyun_transfer() {
	local __PARAM=$*
	local __CNT=0
	local __RUNPROG __ERR PID_SLEEP

	[ $# -eq 0 ] && write_log 12 "'aliyun_transfer()' 出错 - 参数数量错误"

	while : ; do
		build_Request $__PARAM
		__RUNPROG="$__CMDBASE '${__URLBASE}?${__URLARGS}'"

		write_log 7 "#> $__RUNPROG"
		eval $__RUNPROG
		__ERR=$?
		[ $__ERR -eq 0 ] && return 0

		write_log 3 "wget 错误代码: '$__ERR'"
		write_log 7 "$(cat $ERRFILE)"

		if [ $VERBOSE -gt 1 ]; then
			write_log 4 "传输失败 - 详细模式: $VERBOSE - 出错后不再重试"
			return 1
		fi

		__CNT=$(( $__CNT + 1 ))
		[ $retry_count -gt 0 -a $__CNT -gt $retry_count ] && \
			write_log 14 "$retry_count 次重试后传输还是失败"

		write_log 4 "传输失败 - $__CNT/$retry_count 在 $RETRY_SECONDS 秒后重试"
		sleep $RETRY_SECONDS &
		PID_SLEEP=$!
		wait $PID_SLEEP
		PID_SLEEP=0
	done
}

# 百分号编码
percentEncode() {
	if [ -z "${1//[A-Za-z0-9_.~-]/}" ]; then
		echo -n "$1"
	else
		local string=$1; local i=0; local ret chr
		while [ $i -lt ${#string} ]; do
			chr=${string:$i:1}
			[ -z "${chr#[^A-Za-z0-9_.~-]}" ] && chr=$(printf '%%%02X' "'$chr")
			ret="$ret$chr"
			i=$(( $i + 1 ))
		done
		echo -n "$ret"
	fi
}

# 构造阿里云解析请求参数
build_Request() {
	local args=$*; local string
	local HTTP_METHOD="GET"

	# 添加请求参数
	__URLARGS=
	for string in $args; do
		case "${string%%=*}" in
			Format|TTL|Version|AccessKeyId|SignatureMethod|Timestamp|SignatureVersion|SignatureNonce|Signature) ;; # 过滤公共参数
			*) __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}");;
		esac
	done
	__URLARGS="${__URLARGS:1}"

	# 附加公共参数
	string="Format=JSON"; __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")
	string="TTL=600";__URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")
	string="Version=2015-01-09"; __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")
	string="AccessKeyId=$username"; __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")
	string="SignatureMethod=HMAC-SHA1"; __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")
	string="Timestamp="$(date -u '+%Y-%m-%dT%H:%M:%SZ'); __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")
	string="SignatureVersion=1.0"; __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")
	string="SignatureNonce="$(cat '/proc/sys/kernel/random/uuid'); __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")

	# 对请求参数进行排序，用于生成签名
	string=$(echo -n "$__URLARGS" | sed 's/\'"${__SEPARATOR}"'/\n/g' | sort | sed ':label; N; s/\n/\'"${__SEPARATOR}"'/g; b label')
	# 构造用于计算签名的字符串
	string="${HTTP_METHOD}${__SEPARATOR}"$(percentEncode "/")"${__SEPARATOR}"$(percentEncode "$string")
	# 字符串计算签名HMAC值
	local signature=$(echo -n "$string" | openssl dgst -sha1 -hmac "${password}&" -binary)
	# HMAC值编码成字符串，得到签名值
	signature=$(echo -n "$signature" | openssl base64)

	# 附加签名参数
	string="Signature=$signature"; __URLARGS="$__URLARGS${__SEPARATOR}"$(percentEncode "${string%%=*}")"="$(percentEncode "${string#*=}")
}

# 添加解析记录
add_domain() {
	local value
	aliyun_transfer "Action=AddDomainRecord" "DomainName=${__DOMAIN}" "RR=${__HOST}" "Type=${__TYPE}" "Value=${__IP}" || write_log 14 "服务器通信失败"
	json_cleanup; json_load "$(cat "$DATFILE" 2> /dev/null)" >/dev/null 2>&1
	json_get_var value "RecordId"
	[ -z "$value" ] && write_log 14 "添加新解析记录失败"
	write_log 7 "添加新解析记录成功"
	return 0
}

# 修改解析记录
update_domain() {
	local value
	aliyun_transfer "Action=UpdateDomainRecord" "RecordId=${__RECID}" "RR=${__HOST}" "Type=${__TYPE}" "Value=${__IP}" || write_log 14 "服务器通信失败"
	json_cleanup; json_load "$(cat "$DATFILE" 2> /dev/null)" >/dev/null 2>&1
	json_get_var value "RecordId"
	[ -z "$value" ] && write_log 14 "修改解析记录失败"
	write_log 7 "修改解析记录成功"
	return 0
}

# 启用解析记录
enable_domain() {
	local value
	aliyun_transfer "Action=SetDomainRecordStatus" "RecordId=${__RECID}" "Status=Enable" || write_log 14 "服务器通信失败"
	json_cleanup; json_load "$(cat "$DATFILE" 2> /dev/null)" >/dev/null 2>&1
	json_get_var value "Status"
	[ "$value" != "Enable" ] && write_log 14 "启用解析记录失败"
	write_log 7 "启用解析记录成功"
	return 0
}

# 获取子域名解析记录列表
describe_domain() {
	local count value; local ret=0
	aliyun_transfer "Action=DescribeSubDomainRecords" "SubDomain=${__HOST}.${__DOMAIN}"  "DomainName=${__DOMAIN}" || write_log 14 "服务器通信失败"
	write_log 7 "获取到解析记录: $(cat "$DATFILE" 2> /dev/null)" 
	json_cleanup; json_load "$(cat "$DATFILE" 2> /dev/null)" >/dev/null 2>&1
	json_get_var count "TotalCount"
	if [ $count -eq 0 ]; then
		write_log 7 "解析记录不存在"
		ret=1
	else
		local i=1;
		while [ $i -le $count ]; do
			json_cleanup; json_load "$(cat "$DATFILE" 2> /dev/null)" >/dev/null 2>&1
			json_select "DomainRecords" >/dev/null 2>&1
			json_select "Record" >/dev/null 2>&1
			json_select $i >/dev/null 2>&1
			i=$(( $i + 1 ))
			json_get_var value "Type"
			if [ "$value" != "${__TYPE}" ]; then
				write_log 7 "当前解析类型: ${__TYPE}, 获得不匹配类型: $value"
				ret=1; continue
			else
				ret=0
				json_get_var __RECID "RecordId"
				write_log 7 "获得解析记录ID: ${__RECID}, 类型: $value"
				json_get_var value "Locked"
				[ $value -ne 0 ] && write_log 14 "解析记录被锁定"
				json_get_var value "Status"
				[ "$value" != "ENABLE" ] && ret=$(( $ret | 2 )) && write_log 7 "解析记录被禁用"
				json_get_var value "Value"
				[ "$value" != "${__IP}" ] && ret=$(( $ret | 4 )) && write_log 7 "地址需要修改"
				break
			fi
		done
	fi
	return $ret
}

build_command
describe_domain
ret=$?
if [ $ret -eq 1 ]; then
	sleep 3 && add_domain
else
	[ $(( $ret & 2 )) -ne 0 ] && sleep 3 && enable_domain
	[ $(( $ret & 4 )) -ne 0 ] && sleep 3 && update_domain
fi

return 0
