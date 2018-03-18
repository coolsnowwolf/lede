#!/bin/sh

# 声明常量
readonly appName='com.xunlei.vip.swjsq'
readonly protocolVersion=108
readonly sdkVersion=17550
readonly agentVersion='1.6.1.177600'
readonly client_type_down='android-swjsq'
readonly client_version_down='2.0.3.4'
readonly client_type_up='android-uplink'
readonly client_version_up='2.3.3.9'
readonly client_os='android-7.0.24DUK-AL20'

# 声明全局变量
_http_cmd=
_http_args=
_devicesign=
_peerid=
_userid=
_sessionid=
_api_url_down=
_api_url_up=
_dial_account_down=
_dial_account_up=
_cur_down=
_max_down=
_cur_up=
_max_up=
access_url=
msg_to_en=
msg_to_cn=

# 包含用于解析 JSON 格式返回值的函数
. /usr/share/libubox/jshn.sh

# 读取 UCI 设置相关函数
uci_get_by_name() {
	local ret=$(uci get $NAME.$1.$2 2> /dev/null)
	echo -n ${ret:=$3}
}
uci_get_by_type() {
	local ret=$(uci get $NAME.@$1[-1].$2 2> /dev/null)
	echo -n ${ret:=$3}
}
uci_get_by_bool() {
	case $(uci_get_by_name "$1" "$2" "$3") in
		1|on|true|yes|enabled) echo -n 1;;
		*) echo -n 0;;
	esac
}

# 日志和状态栏输出。1 日志文件, 2 系统日志, 4 详细模式, 8 下行状态栏, 16 上行状态栏, 32 失败状态
_log() {
	local msg=$1
	local flag=$2
	[ -z "$msg" ] && return
	[ -z "$flag" ] && flag=1
	local timestamp=$(date +'%Y/%m/%d %H:%M:%S')

	[ $logging -eq 0 -a $(( $flag & 1 )) -ne 0 ] && flag=$(( $flag ^ 1 ))
	if [ $verbose -eq 0 -a $(( $flag & 4 )) -ne 0 ]; then
		[ $(( $flag & 1 )) -ne 0 ] && flag=$(( $flag ^ 1 ))
		[ $(( $flag & 2 )) -ne 0 ] && flag=$(( $flag ^ 2 ))
	fi
	if [ $down_acc -eq 0 -a $(( $flag & 8 )) -ne 0 ]; then
		flag=$(( $flag ^ 8 ))
		[ $up_acc -ne 0 ] && flag=$(( $flag | 16 ))
	fi
	if [ $up_acc -eq 0 -a $(( $flag & 16 )) -ne 0 ]; then
		flag=$(( $flag ^ 16 ))
		[ $down_acc -ne 0 ] && flag=$(( $flag | 8 ))
	fi

	[ $(( $flag & 1 )) -ne 0 ] && echo "$timestamp $msg" >> $LOGFILE 2> /dev/null
	[ $(( $flag & 2 )) -ne 0 ] && logger -p "daemon.info" -t "$NAME" "$msg"

	[ $(( $flag & 32 )) -eq 0 ] && local color="green" || local color="red"
	[ $(( $flag & 8 )) -ne 0 ] && echo -n "<font color=$color>$timestamp $msg</font>" > $down_state_file 2> /dev/null
	[ $(( $flag & 16 )) -ne 0 ] && echo -n "<font color=$color>$timestamp $msg</font>" > $up_state_file 2> /dev/null
}

# 清理日志
clean_log() {
	[ $logging -eq 1 -a -f "$LOGFILE" ] || return
	if [ $(wc -l "$LOGFILE" | awk '{print $1}') -gt 500 ]; then
		_log "清理日志文件"
		local logdata=$(tail -n 300 "$LOGFILE")
		echo "$logdata" > $LOGFILE 2> /dev/null
		unset logdata
	fi
}

# 获取接口IP地址
get_acc_ip() {
	local acc_ipaddr
	json_cleanup; json_load "$(ubus call network.interface.$network status 2> /dev/null)" >/dev/null 2>&1
	json_select "ipv4-address" >/dev/null 2>&1; json_select 1 >/dev/null 2>&1
	json_get_var acc_ipaddr "address"
	[ -z "$acc_ipaddr" ] && { _log "获取网络 $network IP地址出错"; return; }
	_log "acc_ipaddr is $acc_ipaddr" $(( 1 | 4 ))

	if [ "$acc_ipaddr" != "0.0.0.0" ]; then
		_log "绑定IP地址: $acc_ipaddr"
		echo -n "$acc_ipaddr"
	fi
}

# 定义基本 HTTP 命令和参数
gen_http_cmd() {
	_http_cmd="wget-ssl -nv -t 1 -O - --no-check-certificate"
	_http_cmd="$_http_cmd --bind-address=$1"
	_http_cmd="$_http_cmd --user-agent=android-async-http/xl-acc-sdk/version-$agentVersion"
	_log "_http_cmd is $_http_cmd" $(( 1 | 4 ))
}

# 生成设备签名
gen_device_sign() {
	local ifname macaddr
	while : ; do
		ifname=$(uci get "network.$network.ifname" 2> /dev/null)
		[ "${ifname:0:1}" == "@" ] && network="${ifname:1}" || break
	done
	[ -z "$ifname" ] && { _log "获取网络 $network 信息出错"; return; }
	json_cleanup; json_load "$(ubus call network.device status {\"name\":\"$ifname\"} 2> /dev/null)" >/dev/null 2>&1
	json_get_var macaddr "macaddr"
	[ -z "$macaddr" ] && { _log "获取网络 $network MAC地址出错"; return; }
	macaddr=$(echo -n "$macaddr" | awk '{print toupper($0)}')

	# 根据MAC地址生成peerid
	readonly _peerid="${macaddr//:/}004V"
	_log "_peerid is $_peerid" $(( 1 | 4 ))

	# 根据MAC地址生成devicesign
	local xlnetacc_vip="${appName}68700d1872b772946a6940e4b51827e8af"
	local fake_device_id_md5=$(echo -n "$macaddr" | md5sum | awk '{print $1}')
	local fake_device_id_sha1=$(echo -n "$fake_device_id_md5$xlnetacc_vip" | openssl sha1 -hmac | awk '{print $2}')
	readonly _devicesign="div100.$fake_device_id_md5"$(echo -n "$fake_device_id_sha1" | md5sum | awk '{print $1}')
	_log "_devicesign is $_devicesign" $(( 1 | 4 ))
}

# 帐号登录
xlnetacc_login() {
	# 生成POST数据
	json_init
	json_add_string userName "$account"
	json_add_int businessType 68
	json_add_string clientVersion "$client_version_down"
	json_add_string appName "ANDROID-$appName"
	json_add_int isCompressed 0
	json_add_int sequenceNo 1000001
	json_add_string sessionID
	json_add_int loginType 0
	json_add_object rsaKey
	json_add_string e '010001'
	json_add_string n 'AC69F5CCC8BDE47CD3D371603748378C9CFAD2938A6B021E0E191013975AD683F5CBF9ADE8BD7D46B4D2EC2D78AF146F1DD2D50DC51446BB8880B8CE88D476694DFC60594393BEEFAA16F5DBCEBE22F89D640F5336E42F587DC4AFEDEFEAC36CF007009CCCE5C1ACB4FF06FBA69802A8085C2C54BADD0597FC83E6870F1E36FD'
	json_close_object
	json_add_int cmdID 1
	json_add_string verifyCode
	json_add_string peerID "$_peerid"
	json_add_int protocolVersion $protocolVersion
	json_add_int platformVersion 1
	json_add_string passWord "$encrypt"
	json_add_string extensionList
	json_add_string verifyKey
	json_add_int sdkVersion $sdkVersion
	json_add_string devicesign "$_devicesign"
	json_close_object

	local ret=$($_http_cmd 'https://login.mobile.reg2t.sandai.net:443/' --post-data="$(json_dump)")
	_log "ret is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	local errno
	json_get_var errno "errorCode"
	json_get_var _userid "userID"
	_log "_userid is $_userid" $(( 1 | 4 ))
	json_get_var _sessionid "sessionID"
	_log "_sessionid is $_sessionid" $(( 1 | 4 ))

	if [ ${errno:=-1} -ne 0 ] || [ -z "$_userid" -o -z "$_sessionid" ]; then
		local errorDesc
		json_get_var errorDesc "errorDesc"
		local outmsg="帐号登录失败。错误代码: ${errno}"; \
			[ -n "$errorDesc" ] && outmsg="${outmsg}，原因: $errorDesc"; _log "$outmsg" $(( 1 | 8 | 16 | 32 ))
	else
		local outmsg="帐号登录成功"; _log "$outmsg" $(( 1 | 8 | 16 ))
	fi

	return $errno
}

# 获取提速API
xlnetacc_portal() {
	xlnetacc_var $1

	[ $1 -eq 1 ] && access_url='http://api.portal.swjsq.vip.xunlei.com:81/v2/queryportal' || \
		access_url='http://api.upportal.swjsq.vip.xunlei.com/v2/queryportal'
	local ret=$($_http_cmd "$access_url")
	_log "$msg_to_en portal is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	local errno portal_ip portal_port
	json_get_var errno "errno"
	json_get_var portal_ip "interface_ip"
	json_get_var portal_port "interface_port"

	if [ ${errno:=-1} -ne 0 ] || [ -z "$portal_ip" -o -z "$portal_port" ]; then
		local message
		json_get_var message "message"
		local outmsg="获取${msg_to_cn}API失败。错误代码: ${errno}"; \
			[ -n "$message" ] && outmsg="${outmsg}，原因: $message"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ))
	else
		local outmsg="获取${msg_to_cn}API成功"; _log "$outmsg" $(( 1 | $1 * 8 ))
		if [ $1 -eq 1 ]; then
			_api_url_down="http://$portal_ip:$portal_port/v2"
			_log "_api_url_down is $_api_url_down" $(( 1 | 4 ))
		else
			_api_url_up="http://$portal_ip:$portal_port/v2"
			_log "_api_url_up is $_api_url_up" $(( 1 | 4 ))
		fi
	fi

	return $errno
}

# 获取网络带宽信息
xlnetacc_bandwidth() {
	xlnetacc_var $1

	local ret=$($_http_cmd "$access_url/bandwidth?${_http_args}&time_and=$(date +%s)000")
	_log "$msg_to_en bandwidth is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	local errno can_upgrade dial_account richmessage
	json_get_var errno "errno"
	json_get_var can_upgrade "can_upgrade"
	json_get_var dial_account "dial_account"
	json_get_var richmessage "richmessage"

	# 获取带宽数据
	local cur_bandwidth max_bandwidth flag
	[ $1 -eq 1 ] && flag="downstream" || flag="upstream"
	json_select "bandwidth" >/dev/null 2>&1
	json_get_var cur_bandwidth "$flag"
	json_select; json_select "max_bandwidth" >/dev/null 2>&1
	json_get_var max_bandwidth "$flag"
	cur_bandwidth=$(expr $cur_bandwidth / 1024 2> /dev/null)
	max_bandwidth=$(expr $max_bandwidth / 1024 2> /dev/null)

	if [ ${errno:=-1} -ne 0 ]; then
		local outmsg="获取${msg_to_cn}带宽信息失败。错误代码: ${errno}"; \
			[ -n "$richmessage" ] && outmsg="${outmsg}，原因: $richmessage"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ))
	elif [ "$can_upgrade" -eq 0 ]; then
		local outmsg="${msg_to_cn}无法提速"; \
			[ -n "$richmessage" ] && outmsg="${outmsg}，原因: $richmessage"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ))
		[ $1 -eq 1 ] && down_acc=0 || up_acc=0
		errno=-2
	elif [ "$cur_bandwidth" -ge "$max_bandwidth" ]; then
		local outmsg="${msg_to_cn}无需提速。当前带宽 ${cur_bandwidth}M，超过最大可提升带宽 ${max_bandwidth}M"; \
			_log "$outmsg" $(( 1 | $1 * 8 ))
		[ $1 -eq 1 ] && down_acc=0 || up_acc=0
		errno=-3
	else
		local outmsg="${msg_to_cn}可以提速。当前带宽 ${cur_bandwidth}M，可提升至 ${max_bandwidth}M"; _log "$outmsg" $(( 1 | $1 * 8 ))
		if [ $1 -eq 1 ]; then
			_dial_account_down=$dial_account
			_log "_dial_account_down is $_dial_account_down" $(( 1 | 4 ))
			_cur_down=$cur_bandwidth
			_log "_cur_down is $_cur_down" $(( 1 | 4 ))
			_max_down=$max_bandwidth
			_log "_max_down is $_max_down" $(( 1 | 4 ))
		else
			_dial_account_up=$dial_account
			_log "_dial_account_up is $_dial_account_up" $(( 1 | 4 ))
			_cur_up=$cur_bandwidth
			_log "_cur_up is $_cur_up" $(( 1 | 4 ))
			_max_up=$max_bandwidth
			_log "_max_up is $_max_up" $(( 1 | 4 ))
		fi
	fi

	return $errno
}

# 发送带宽提速信号
xlnetacc_upgrade() {
	xlnetacc_var $1

	local ret=$($_http_cmd "$access_url/upgrade?${_http_args}&time_and=$(date +%s)000")
	_log "$msg_to_en upgrade is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	local errno
	json_get_var errno "errno"

	if [ ${errno:=-1} -ne 0 ]; then
		local richmessage
		json_get_var richmessage "richmessage"
		local outmsg="${msg_to_cn}提速失败。错误代码: ${errno}"; \
			[ -n "$richmessage" ] && outmsg="${outmsg}，原因: $richmessage"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ))
		[ $errno -ne -1 ] && { [ $1 -eq 1 ] && down_acc=0 || up_acc=0; }
	else
		[ $1 -eq 1 ] && local cur_bandwidth=$_cur_down || local cur_bandwidth=$_cur_up
		[ $1 -eq 1 ] && local max_bandwidth=$_max_down || local max_bandwidth=$_max_up
		local outmsg="${msg_to_cn}提速成功，带宽已从 ${cur_bandwidth}M 提升到 ${max_bandwidth}M"; _log "$outmsg" $(( 1 | $1 * 8 ))
		[ $1 -eq 1 ] && down_acc=2 || up_acc=2
	fi

	return $errno
}

# 发送提速心跳信号
xlnetacc_keepalive() {
	xlnetacc_var $1

	local ret=$($_http_cmd "$access_url/keepalive?${_http_args}&time_and=$(date +%s)000")
	_log "$msg_to_en keepalive is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	local errno
	json_get_var errno "errno"

	if [ ${errno:=-1} -ne 0 ]; then
		local richmessage
		json_get_var richmessage "richmessage"
		local outmsg="${msg_to_cn}提速失效。错误代码: ${errno}"; \
			[ -n "$richmessage" ] && outmsg="${outmsg}，原因: $richmessage"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ))
		[ $1 -eq 1 ] && down_acc=1 || up_acc=1
	else
		_log "${msg_to_cn}心跳信号返回正常"
	fi

	return $errno
}

# 发送带宽恢复信号
xlnetacc_recover() {
	xlnetacc_var $1

	local ret=$($_http_cmd "$access_url/recover?${_http_args}&time_and=$(date +%s)000")
	_log "$msg_to_en recover is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	local errno
	json_get_var errno "errno"

	if [ ${errno:=-1} -ne 0 ]; then
		local richmessage
		json_get_var richmessage "richmessage"
		local outmsg="${msg_to_cn}带宽恢复失败。错误代码: ${errno}"; \
			[ -n "$richmessage" ] && outmsg="${outmsg}，原因: $richmessage"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ))
	else
		_log "${msg_to_cn}带宽已恢复"
		[ $1 -eq 1 ] && down_acc=1 || up_acc=1
	fi

	return $errno
}

# 查询提速信息，未使用
xlnetacc_query() {
	xlnetacc_var $1

	local ret=$($_http_cmd "$access_url/query_try_info?${_http_args}&time_and=$(date +%s)000")
	_log "$msg_to_en query_try_info is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	local errno
	json_get_var errno "errno"

	return $errno
}

# 带宽提速处理
xlnetacc_acc() {
	_api_url_down=; _api_url_up=
	_dial_account_down=; _dial_account_up=
	_cur_down=0; _max_down=0; _cur_up=0; _max_up=0

	xlnetacc_portal $1 || return
	xlnetacc_bandwidth $1 || return
	xlnetacc_upgrade $1
}

# 设置参数变量
xlnetacc_var() {
	if [ $1 -eq 1 ]; then
		access_url=$_api_url_down
		_http_args="peerid=${_peerid}&userid=${_userid}&user_type=1&sessionid=${_sessionid}&dial_account=${_dial_account_down}&client_type=${client_type_down}-${client_version_down}&client_version=${client_type_down//-/}-${client_version_down}&os=${client_os}"
		msg_to_en="DownLink"
		msg_to_cn="下行"
	else
		access_url=$_api_url_up
		_http_args="peerid=${_peerid}&userid=${_userid}&user_type=1&sessionid=${_sessionid}&dial_account=${_dial_account_up}&client_type=${client_type_up}-${client_version_up}&client_version=${client_type_up//-/}-${client_version_up}&os=${client_os}"
		msg_to_en="UpLink"
		msg_to_cn="上行"
	fi
}

# 中止信号处理
sigterm() {
	_log "trap sigterm, exit" $(( 1 | 4 ))
	[ $down_acc -eq 2 ] && xlnetacc_recover 1
	[ $up_acc -eq 2 ] && xlnetacc_recover 2
	rm -f "$down_state_file" "$up_state_file"
	exit 0
}

# 初始化
xlnetacc_init() {
	[ "$1" != "--start" ] && return 1

	# 防止重复启动
	local pid
	for pid in $(pidof "${0##*/}"); do
		[ $pid -ne $$ ] && return 1
	done

	# 读取设置
	readonly NAME=xlnetacc
	readonly LOGFILE=/var/log/${NAME}.log
	readonly down_state_file=/var/state/${NAME}_down_state
	readonly up_state_file=/var/state/${NAME}_up_state
	down_acc=$(uci_get_by_bool "general" "down_acc" 0)
	up_acc=$(uci_get_by_bool "general" "up_acc" 0)
	readonly logging=$(uci_get_by_bool "general" "logging" 1)
	readonly verbose=$(uci_get_by_bool "general" "verbose" 0)
	readonly network=$(uci_get_by_name "general" "network" "wan")
	readonly account=$(uci_get_by_name "general" "account")
	readonly encrypt=$(uci_get_by_name "general" "encrypt")
	local enabled=$(uci_get_by_bool "general" "enabled" 0)
	local password=$(uci_get_by_name "general" "password")
	( [ $enabled -eq 0 ] || [ $down_acc -eq 0 -a $up_acc -eq 0 ] || [ -z "$account" -o -z "$encrypt" -o -z "$network" ] ) && return 2

	[ $logging -eq 1 ] && [ ! -d /var/log ] && mkdir -p /var/log
	_log "------------------------------"
	_log "迅雷快鸟正在启动..."
	_log "down_acc is $down_acc" $(( 1 | 4 ))
	_log "up_acc is $up_acc" $(( 1 | 4 ))
	_log "network is $network" $(( 1 | 4 ))
	_log "account is $account" $(( 1 | 4 ))
	_log "encrypt is $encrypt" $(( 1 | 4 ))

	# 检查外部调用工具
	command -v wget-ssl >/dev/null || { _log "GNU Wget 工具不存在"; return 3; }
	command -v md5sum >/dev/null || { _log "md5sum 工具不存在"; return 3; }
	command -v openssl >/dev/null || { _log "openssl 工具不存在"; return 3; }

	# 检查明文密码
	if [ -n "$password" ]; then
		_log "移除已保存的明文密码"
		uci delete "${NAME}.general.password"
		uci commit "$NAME"
	fi

	# 捕获中止信号
	trap "sigterm" INT
	trap "sigterm" TERM

	# 生成设备签名
	gen_device_sign
	[ -z "$_peerid" -o -z "$_devicesign" ] && return 4

	clean_log
	[ -d /var/state ] || mkdir -p /var/state
	return 0
}

# 程序主体
xlnetacc_main() {
	while : ; do
		# 获取外网IP地址
		while : ; do
			local bind_ip=$(get_acc_ip)
			if [ -z "$bind_ip" ]; then
				sleep 5s # 获取失败
			else
				gen_http_cmd "$bind_ip"
				break
			fi
		done

		# 帐号登录
		while : ; do
			xlnetacc_login
			case $? in
				0) break;; # 登录成功
				6) sleep 30m;; # 需要输入验证码，等待30分钟后重试
				-1) sleep 3m;; # 未返回有效数据，等待3分钟后重试
				*) return 5;; # 登录失败
			esac
		done

		# 10秒检查提速状态，300秒发送心跳包
		local timer_begin=$(date +%s); local timer_count=
		while : ; do
			[ $down_acc -eq 0 -a $up_acc -eq 0 ] && break 2
			[ $down_acc -eq 1 ] && xlnetacc_acc 1
			[ $up_acc -eq 1 ] && xlnetacc_acc 2

			sleep 10s
			timer_count=$(( $(date +%s) - $timer_begin ))
			[ $timer_count -lt 300 ] && continue
			timer_begin=$(date +%s)
			clean_log # 清理日志

			[ $down_acc -eq 2 ] && { xlnetacc_keepalive 1; [ $? -eq -1 ] && break; }
			[ $up_acc -eq 2 ] && { xlnetacc_keepalive 2; [ $? -eq -1 ] && break; }
		done
		[ $down_acc -eq 2 ] && { xlnetacc_recover 1; down_acc=1; }
		[ $up_acc -eq 2 ] && { xlnetacc_recover 2; up_acc=1; }
	done
	_log "程序异常，迅雷快鸟已停止。"
}

# 程序入口
xlnetacc_init $*
[ $? -eq 0 ] && xlnetacc_main
exit $?
