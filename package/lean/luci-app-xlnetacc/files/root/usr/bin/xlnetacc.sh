#!/bin/sh

# 声明常量
readonly packageName='com.xunlei.vip.swjsq'
readonly protocolVersion=200
readonly businessType=68
readonly sdkVersion='2.1.1.177662'
readonly clientVersion='2.4.1.3'
readonly agent_xl="android-async-http/xl-acc-sdk/version-$sdkVersion"
readonly agent_down='okhttp/3.4.1'
readonly agent_up='android-async-http/xl-acc-sdk/version-1.0.0.1'
readonly client_type_down='android-swjsq'
readonly client_type_up='android-uplink'

# 声明全局变量
_bind_ip=
_http_cmd=
_peerid=
_devicesign=
_userid=
_loginkey=
_sessionid=
_portal_down=
_portal_up=
_dial_account=
access_url=
http_args=
user_agent=
link_cn=
lasterr=
sequence_xl=1000000
sequence_down=$(( $(date +%s) / 6 ))
sequence_up=$sequence_down

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
	[ $(wc -l "$LOGFILE" | awk '{print $1}') -le 800 ] && return
	_log "清理日志文件"
	local logdata=$(tail -n 500 "$LOGFILE")
	echo "$logdata" > $LOGFILE 2> /dev/null
	unset logdata
}

# 获取接口IP地址
get_bind_ip() {
	json_cleanup; json_load "$(ubus call network.interface.$network status 2> /dev/null)" >/dev/null 2>&1
	json_select "ipv4-address" >/dev/null 2>&1; json_select 1 >/dev/null 2>&1
	json_get_var _bind_ip "address"
	if [ -z "$_bind_ip" -o "$_bind_ip"x == "0.0.0.0"x ]; then
		_log "获取网络 $network IP地址失败"
		return 1
	else
		_log "绑定IP地址: $_bind_ip"
		return 0
	fi
}

# 定义基本 HTTP 命令和参数
gen_http_cmd() {
	_http_cmd="wget-ssl -nv -t 1 -T 5 -O - --no-check-certificate"
	_http_cmd="$_http_cmd --bind-address=$_bind_ip"
}

# 生成设备标识
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

	# 计算peerID
	local fake_peerid=$(awk -F- '{print toupper($5)}' '/proc/sys/kernel/random/uuid')
	readonly _peerid="${fake_peerid}004V"
	_log "_peerid is $_peerid" $(( 1 | 4 ))

	# 计算devicesign
	# sign = div.10?.device_id + md5(sha1(packageName + businessType + md5(a protocolVersion specific GUID)))
	local fake_device_id=$(echo -n "${macaddr//:/}" | openssl dgst -md5 | awk '{print $2}')
	local fake_device_sign=$(echo -n "${fake_device_id}${packageName}${businessType}c7f21687eed3cdb400ca11fc2263c998" \
		| openssl dgst -sha1 | awk '{print $2}')
	readonly _devicesign="div101.${fake_device_id}"$(echo -n "$fake_device_sign" | openssl dgst -md5 | awk '{print $2}')
	_log "_devicesign is $_devicesign" $(( 1 | 4 ))
}

# 快鸟帐号通用参数
swjsq_json() {
	let sequence_xl++
	# 生成POST数据
	json_init
	json_add_string protocolVersion "$protocolVersion"
	json_add_string sequenceNo "$sequence_xl"
	json_add_string platformVersion '2'
	json_add_string isCompressed '0'
	json_add_string businessType "$businessType"
	json_add_string clientVersion "$clientVersion"
	json_add_string peerID "$_peerid"
	json_add_string appName "ANDROID-$packageName"
	json_add_string sdkVersion "${sdkVersion##*.}"
	json_add_string devicesign "$_devicesign"
	json_add_string deviceModel 'MI'
	json_add_string deviceName 'Xiaomi Mi'
	json_add_string OSVersion "7.1.1"
}

# 帐号登录
swjsq_login() {
	swjsq_json
	if [ -z "$_userid" -o -z "$_loginkey" ]; then
		access_url='https://mobile-login.xunlei.com/login'
		json_add_string userName "$username"
		json_add_string passWord "$password"
		json_add_string verifyKey
		json_add_string verifyCode
	else
		access_url='https://mobile-login.xunlei.com/loginkey'
		json_add_string userName "$_userid"
		json_add_string loginKey "$_loginkey"
	fi
	json_close_object

	local ret=$($_http_cmd --user-agent="$agent_xl" "$access_url" --post-data="$(json_dump)")
	case $? in
		0)
			_log "login is $ret" $(( 1 | 4 ))
			json_cleanup; json_load "$ret" >/dev/null 2>&1
			json_get_var lasterr "errorCode"
			;;
		2) lasterr=-2;;
		4) lasterr=-3;;
		*) lasterr=-1;;
	esac

	case ${lasterr:=-1} in
		0)
			json_get_var _userid "userID"
			json_get_var _loginkey "loginKey"
			json_get_var _sessionid "sessionID"
			_log "_sessionid is $_sessionid" $(( 1 | 4 ))
			local outmsg="帐号登录成功"; _log "$outmsg" $(( 1 | 8 ))
			;;
		15) # 身份信息已失效
			_userid=; _loginkey=;;
		-1)
			local outmsg="帐号登录失败。迅雷服务器未响应，请稍候"; _log "$outmsg";;
		-2)
			local outmsg="Wget 参数解析错误，请更新 GNU Wget"; _log "$outmsg" $(( 1 | 8 | 32 ));;
		-3)
			local outmsg="Wget 网络通信失败，请稍候"; _log "$outmsg";;
		*)
			local errorDesc; json_get_var errorDesc "errorDesc"
			local outmsg="帐号登录失败。错误代码: ${lasterr}"; \
				[ -n "$errorDesc" ] && outmsg="${outmsg}，原因: $errorDesc"; _log "$outmsg" $(( 1 | 8 | 32 ));;
	esac

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 帐号注销
swjsq_logout() {
	swjsq_json
	json_add_string userID "$_userid"
	json_add_string sessionID "$_sessionid"
	json_close_object

	local ret=$($_http_cmd --user-agent="$agent_xl" 'https://mobile-login.xunlei.com/logout' --post-data="$(json_dump)")
	_log "logout is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	json_get_var lasterr "errorCode"

	case ${lasterr:=-1} in
		0)
			_sessionid=
			local outmsg="帐号注销成功"; _log "$outmsg" $(( 1 | 8 ));;
		-1)
			local outmsg="帐号注销失败。迅雷服务器未响应，请稍候"; _log "$outmsg";;
		*)
			local errorDesc; json_get_var errorDesc "errorDesc"
			local outmsg="帐号注销失败。错误代码: ${lasterr}"; \
				[ -n "$errorDesc" ] && outmsg="${outmsg}，原因: $errorDesc"; _log "$outmsg" $(( 1 | 8 | 32 ));;
	esac

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 获取用户信息
swjsq_getuserinfo() {
	[ $1 -eq 1 ] && local _vasid=14 || local _vasid=33
	swjsq_json
	json_add_string userID "$_userid"
	json_add_string sessionID "$_sessionid"
	json_add_string vasid "$_vasid"
	json_close_object

	local ret=$($_http_cmd --user-agent="$agent_xl" 'https://mobile-login.xunlei.com/getuserinfo' --post-data="$(json_dump)")
	_log "getuserinfo $1 is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	json_get_var lasterr "errorCode"

	[ $1 -eq 1 ] && local outmsg="下行提速会员" || local outmsg="上行提速会员"
	case ${lasterr:=-1} in
		0)
			local index vasid isVip isYear expireDate can_upgrade
			json_select "vipList" >/dev/null 2>&1
			while : ; do
				json_select ${index:=1} >/dev/null 2>&1
				[ $? -ne 0 ] && break
				json_get_var vasid "vasid"
				json_get_var isVip "isVip"
				json_get_var isYear "isYear"
				json_get_var expireDate "expireDate"
				json_select ".." >/dev/null 2>&1
				let index++
				([ $1 -eq 1 -a ${vasid:-0} -eq 2 ] || [ ${vasid:-0} -eq $_vasid ]) && \
					[ ${isVip:-0} -eq 1 -o ${isYear:-0} -eq 1 ] && { can_upgrade=1; break; }
			done
			if [ ${can_upgrade:-0} -eq 1 ]; then
				outmsg="获取${outmsg}信息成功。会员到期时间：${expireDate:0:4}-${expireDate:4:2}-${expireDate:6:2}"; \
					_log "$outmsg" $(( 1 | $1 * 8 ))
			else
				if [ ${#expireDate} -ge 8 ]; then
					outmsg="${outmsg}已到期。会员到期时间：${expireDate:0:4}-${expireDate:4:2}-${expireDate:6:2}"
				else
					outmsg="${outmsg}无效"
				fi
				_log "$outmsg" $(( 1 | $1 * 8 | 32 ))
				[ $1 -eq 1 ] && down_acc=0 || up_acc=0
			fi
			;;
		-1)
			outmsg="获取${outmsg}信息失败。迅雷服务器未响应，请稍候"; _log "$outmsg";;
		*)
			local errorDesc; json_get_var errorDesc "errorDesc"
			outmsg="获取${outmsg}信息失败。错误代码: ${lasterr}"; \
				[ -n "$errorDesc" ] && outmsg="${outmsg}，原因: $errorDesc"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
	esac

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 获取提速入口
swjsq_portal() {
	xlnetacc_var $1

	[ $1 -eq 1 ] && access_url='http://api.portal.swjsq.vip.xunlei.com:81/v2/queryportal' || \
		access_url='http://api.upportal.swjsq.vip.xunlei.com/v2/queryportal'
	local ret=$($_http_cmd --user-agent="$user_agent" "$access_url")
	_log "portal $1 is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	json_get_var lasterr "errno"

	case ${lasterr:=-1} in
		0)
			local interface_ip interface_port province sp
			json_get_var interface_ip "interface_ip"
			json_get_var interface_port "interface_port"
			json_get_var province "province_name"
			json_get_var sp "sp_name"
			if [ $1 -eq 1 ]; then
				_portal_down="http://$interface_ip:$interface_port/v2"
				_log "_portal_down is $_portal_down" $(( 1 | 4 ))
			else
				_portal_up="http://$interface_ip:$interface_port/v2"
				_log "_portal_up is $_portal_up" $(( 1 | 4 ))
			fi
			local outmsg="获取${link_cn}提速入口成功"; \
				[ -n "$province" -a -n "$sp" ] && outmsg="${outmsg}。运营商：${province}${sp}"; _log "$outmsg" $(( 1 | $1 * 8 ))
			;;
		-1)
			local outmsg="获取${link_cn}提速入口失败。迅雷服务器未响应，请稍候"; _log "$outmsg";;
		*)
			local message; json_get_var message "message"
			local outmsg="获取${link_cn}提速入口失败。错误代码: ${lasterr}"; \
				[ -n "$message" ] && outmsg="${outmsg}，原因: $message"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
	esac

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 获取网络带宽信息
isp_bandwidth() {
	xlnetacc_var $1

	local ret=$($_http_cmd --user-agent="$user_agent" "$access_url/bandwidth?${http_args%&dial_account=*}")
	_log "bandwidth $1 is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	json_get_var lasterr "errno"

	case ${lasterr:=-1} in
		0)
			# 获取带宽数据
			local can_upgrade bind_dial_account dial_account stream cur_bandwidth max_bandwidth
			[ $1 -eq 1 ] && stream="downstream" || stream="upstream"
			json_get_var can_upgrade "can_upgrade"
			json_get_var bind_dial_account "bind_dial_account"
			json_get_var dial_account "dial_account"
			json_select; json_select "bandwidth" >/dev/null 2>&1
			json_get_var cur_bandwidth "$stream"
			json_select; json_select "max_bandwidth" >/dev/null 2>&1
			json_get_var max_bandwidth "$stream"
			json_select
			cur_bandwidth=$(expr ${cur_bandwidth:-0} / 1024)
			max_bandwidth=$(expr ${max_bandwidth:-0} / 1024)

			if [ -n "$bind_dial_account" -a "$bind_dial_account" != "$dial_account" ]; then
				local outmsg="绑定宽带账号 $bind_dial_account 与当前宽带账号 $dial_account 不一致，请联系迅雷客服解绑（每月仅一次）"; \
					_log "$outmsg" $(( 1 | 8 | 32 ))
				down_acc=0; up_acc=0
			elif [ $can_upgrade -eq 0 ]; then
				local message; json_get_var message "richmessage"; [ -z "$message" ] && json_get_var message "message"
				local outmsg="${link_cn}无法提速"; \
					[ -n "$message" ] && outmsg="${outmsg}，原因: $message"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ))
				[ $1 -eq 1 ] && down_acc=0 || up_acc=0
			elif [ $cur_bandwidth -ge $max_bandwidth ]; then
				local outmsg="${link_cn}无需提速。当前带宽 ${cur_bandwidth}M，超过最大可提升带宽 ${max_bandwidth}M"; \
					_log "$outmsg" $(( 1 | $1 * 8 ))
				[ $1 -eq 1 ] && down_acc=0 || up_acc=0
			else
				if [ -z "$_dial_account" -a -n "$dial_account" ]; then
					_dial_account=$dial_account
					_log "_dial_account is $_dial_account" $(( 1 | 4 ))
				fi
				local outmsg="${link_cn}可以提速。当前带宽 ${cur_bandwidth}M，可提升至 ${max_bandwidth}M"; _log "$outmsg" $(( 1 | $1 * 8 ))
			fi
			;;
		724) # 724 账号存在异常
			lasterr=-2
			local outmsg="获取${link_cn}网络带宽信息失败。原因: 您的账号存在异常，请联系迅雷客服反馈"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
		3103) # 3103 线路暂不支持
			lasterr=0
			local province sp
			json_get_var province "province_name"; json_get_var sp "sp_name"
			local outmsg="${link_cn}无法提速。原因: ${province}${sp}线路暂不支持"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ))
			[ $1 -eq 1 ] && down_acc=0 || up_acc=0
			;;
		-1)
			local outmsg="获取${link_cn}网络带宽信息失败。运营商服务器未响应，请稍候"; _log "$outmsg";;
		*)
			local message; json_get_var message "richmessage"; [ -z "$message" ] && json_get_var message "message"
			local outmsg="获取${link_cn}网络带宽信息失败。错误代码: ${lasterr}"; \
				[ -n "$message" ] && outmsg="${outmsg}，原因: $message"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
	esac

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 发送带宽提速信号
isp_upgrade() {
	xlnetacc_var $1

	local ret=$($_http_cmd --user-agent="$user_agent" "$access_url/upgrade?$http_args")
	_log "upgrade $1 is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	json_get_var lasterr "errno"

	case ${lasterr:=-1} in
		0)
			local bandwidth
			json_select "bandwidth" >/dev/null 2>&1
			json_get_var bandwidth "downstream"
			[ ${bandwidth:=0} -ge 1024 ] && bandwidth=$(( $bandwidth / 1024 ))
			local outmsg="${link_cn}提速成功，带宽已提升到 ${bandwidth}M"; _log "$outmsg" $(( 1 | $1 * 8 ))
			[ $1 -eq 1 ] && down_acc=2 || up_acc=2
			;;
		812) # 812 已处于提速状态
			lasterr=0
			local outmsg="${link_cn}提速成功，当前宽带已处于提速状态"; _log "$outmsg" $(( 1 | $1 * 8 ))
			[ $1 -eq 1 ] && down_acc=2 || up_acc=2
			;;
		724) # 724 账号存在异常
			lasterr=-2
			local outmsg="${link_cn}提速失败。原因: 您的账号存在异常，请联系迅雷客服反馈"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
		-1)
			local outmsg="${link_cn}提速失败。运营商服务器未响应，请稍候"; _log "$outmsg";;
		*)
			local message; json_get_var message "richmessage"; [ -z "$message" ] && json_get_var message "message"
			local outmsg="${link_cn}提速失败。错误代码: ${lasterr}"; \
				[ -n "$message" ] && outmsg="${outmsg}，原因: $message"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
	esac

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 发送提速心跳信号
isp_keepalive() {
	xlnetacc_var $1

	local ret=$($_http_cmd --user-agent="$user_agent" "$access_url/keepalive?$http_args")
	_log "keepalive $1 is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	json_get_var lasterr "errno"

	case ${lasterr:=-1} in
		0)
			local outmsg="${link_cn}心跳信号返回正常"; _log "$outmsg";;
		513) # 513 提速通道不存在
			lasterr=-2
			local outmsg="${link_cn}提速超时，提速通道不存在"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
		-1)
			local outmsg="${link_cn}心跳信号发送失败。运营商服务器未响应，请稍候"; _log "$outmsg";;
		*)
			local message; json_get_var message "richmessage"; [ -z "$message" ] && json_get_var message "message"
			local outmsg="${link_cn}提速失效。错误代码: ${lasterr}"; \
				[ -n "$message" ] && outmsg="${outmsg}，原因: $message"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
	esac

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 发送带宽恢复信号
isp_recover() {
	xlnetacc_var $1

	local ret=$($_http_cmd --user-agent="$user_agent" "$access_url/recover?$http_args")
	_log "recover $1 is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	json_get_var lasterr "errno"

	case ${lasterr:=-1} in
		0)
			local outmsg="${link_cn}带宽已恢复"; _log "$outmsg" $(( 1 | $1 * 8 ))
			[ $1 -eq 1 ] && down_acc=1 || up_acc=1;;
		-1)
			local outmsg="${link_cn}带宽恢复失败。运营商服务器未响应，请稍候"; _log "$outmsg";;
		*)
			local message; json_get_var message "richmessage"; [ -z "$message" ] && json_get_var message "message"
			local outmsg="${link_cn}带宽恢复失败。错误代码: ${lasterr}"; \
				[ -n "$message" ] && outmsg="${outmsg}，原因: $message"; _log "$outmsg" $(( 1 | $1 * 8 | 32 ));;
	esac

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 查询提速信息，未使用
isp_query() {
	xlnetacc_var $1

	local ret=$($_http_cmd --user-agent="$user_agent" "$access_url/query_try_info?$http_args")
	_log "query_try_info $1 is $ret" $(( 1 | 4 ))
	json_cleanup; json_load "$ret" >/dev/null 2>&1
	json_get_var lasterr "errno"

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 设置参数变量
xlnetacc_var() {
	if [ $1 -eq 1 ]; then
		let sequence_down++
		access_url=$_portal_down
		http_args="sequence=${sequence_down}&client_type=${client_type_down}-${clientVersion}&client_version=${client_type_down//-/}-${clientVersion}&chanel=umeng-10900011&time_and=$(date +%s)000"
		user_agent=$agent_down
		link_cn="下行"
	else
		let sequence_up++
		access_url=$_portal_up
		http_args="sequence=${sequence_up}&client_type=${client_type_up}-${clientVersion}&client_version=${client_type_up//-/}-${clientVersion}"
		user_agent=$agent_up
		link_cn="上行"
	fi
	http_args="${http_args}&peerid=${_peerid}&userid=${_userid}&sessionid=${_sessionid}&user_type=1&os=android-7.1.1"
	[ -n "$_dial_account" ] && http_args="${http_args}&dial_account=${_dial_account}"
}

# 重试循环
xlnetacc_retry() {
	if [ $# -ge 3 -a $3 -ne 0 ]; then
		[ $2 -eq 1 -a $down_acc -ne $3 ] && return 0
		[ $2 -eq 2 -a $up_acc -ne $3 ] && return 0
	fi

	local retry=1
	while : ; do
		lasterr=
		eval $1 $2 && break # 成功
		[ $# -ge 4 -a $retry -ge $4 ] && break || let retry++ # 重试超时
		case $lasterr in
			-1) sleep 5s;; # 服务器未响应
			-2) break;; # 严重错误
			*) sleep 3s;; # 其它错误
		esac
	done

	[ ${lasterr:-0} -eq 0 ] && return 0 || return 1
}

# 注销已登录帐号
xlnetacc_logout() {
	[ -z "$_sessionid" ] && return 2
	[ $# -ge 1 ] && local retry=$1 || local retry=1

	xlnetacc_retry 'isp_recover' 1 2 $retry
	xlnetacc_retry 'isp_recover' 2 2 $retry
	xlnetacc_retry 'swjsq_logout' 0 0 $retry
	[ $down_acc -ne 0 ] && down_acc=1; [ $up_acc -ne 0 ] && up_acc=1
	_sessionid=; _dial_account=

	[ $lasterr -eq 0 ] && return 0 || return 1
}

# 中止信号处理
sigterm() {
	_log "trap sigterm, exit" $(( 1 | 4 ))
	xlnetacc_logout
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
	network=$(uci_get_by_name "general" "network" "wan")
	readonly username=$(uci_get_by_name "general" "account")
	readonly password=$(uci_get_by_name "general" "password")
	local enabled=$(uci_get_by_bool "general" "enabled" 0)
	([ $enabled -eq 0 ] || [ $down_acc -eq 0 -a $up_acc -eq 0 ] || [ -z "$username" -o -z "$password" -o -z "$network" ]) && return 2

	[ $logging -eq 1 ] && [ ! -d /var/log ] && mkdir -p /var/log
	[ -f "$LOGFILE" ] && _log "------------------------------"
	_log "迅雷快鸟正在启动..."

	# 检查外部调用工具
	command -v wget-ssl >/dev/null || { _log "GNU Wget 未安装"; return 3; }
	local opensslchk=$(echo -n 'openssl' | openssl dgst -sha1 | awk '{print $2}')
	[ "$opensslchk" != 'c898fa1e7226427010e329971e82c669f8d8abb4' ] && { _log "openssl-util 未安装或计算错误"; return 3; }

	# 捕获中止信号
	trap 'sigterm' INT # Ctrl-C
	trap 'sigterm' QUIT # Ctrl-\
	trap 'sigterm' TERM # kill

	# 生成设备标识
	gen_device_sign
	[ ${#_peerid} -ne 16 -o ${#_devicesign} -ne 71 ] && return 4

	clean_log
	[ -d /var/state ] || mkdir -p /var/state
	return 0
}

# 程序主体
xlnetacc_main() {
	while : ; do
		# 获取外网IP地址
		xlnetacc_retry 'get_bind_ip'
		gen_http_cmd

		# 注销快鸟帐号
		xlnetacc_logout 3 && sleep 3s

		# 登录快鸟帐号
		while : ; do
			lasterr=
			swjsq_login
			case $lasterr in
				0) break;; # 登录成功
				-1) sleep 5s;; # 服务器未响应
				-2) return 7;; # Wget 参数解析错误
				-3) sleep 3s;; # Wget 网络通信失败
				6) sleep 130m;; # 需要输入验证码
				8) sleep 3m;; # 服务器系统维护
				15) sleep 1s;; # 身份信息已失效
				*) return 5;; # 登录失败
			esac
		done

		# 获取用户信息
		xlnetacc_retry 'swjsq_getuserinfo' 1 1
		xlnetacc_retry 'swjsq_getuserinfo' 2 1
		[ $down_acc -eq 0 -a $up_acc -eq 0 ] && break
		# 获取提速入口
		xlnetacc_retry 'swjsq_portal' 1 1
		xlnetacc_retry 'swjsq_portal' 2 1
		# 获取带宽信息
		xlnetacc_retry 'isp_bandwidth' 1 1 10 || { sleep 3m; continue; }
		xlnetacc_retry 'isp_bandwidth' 2 1 10 || { sleep 3m; continue; }
		[ $down_acc -eq 0 -a $up_acc -eq 0 ] && break
		# 带宽提速
		xlnetacc_retry 'isp_upgrade' 1 1 10 || { sleep 3m; continue; }
		xlnetacc_retry 'isp_upgrade' 2 1 10 || { sleep 3m; continue; }

		# 心跳保持
#		local retry=1
		while : ; do
			clean_log # 清理日志
			sleep 10m
#			[ $retry -ge 144 ] && break || let retry++ # 心跳最多保持24小时，144=24*60/10
			xlnetacc_retry 'isp_keepalive' 1 2 5 || break
			xlnetacc_retry 'isp_keepalive' 2 2 5 || break
		done
	done
	xlnetacc_logout
	_log "无法提速，迅雷快鸟已停止。"
	return 6
}

# 程序入口
xlnetacc_init "$@" && xlnetacc_main
exit $?
