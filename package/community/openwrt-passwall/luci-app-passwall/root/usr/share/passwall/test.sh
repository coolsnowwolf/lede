#!/bin/sh

CONFIG=passwall
LOG_FILE=/var/log/$CONFIG.log

echolog() {
	local d="$(date "+%Y-%m-%d %H:%M:%S")"
	#echo -e "$d: $1"
	echo -e "$d: $1" >> $LOG_FILE
}

config_n_get() {
	local ret=$(uci -q get "${CONFIG}.${1}.${2}" 2>/dev/null)
	echo "${ret:=$3}"
}

config_t_get() {
	local index=0
	[ -n "$4" ] && index=$4
	local ret=$(uci -q get $CONFIG.@$1[$index].$2 2>/dev/null)
	echo ${ret:=$3}
}

test_url() {
	local url=$1
	local try=1
	[ -n "$2" ] && try=$2
	local timeout=2
	[ -n "$3" ] && timeout=$3
	local extra_params=$4
	curl --help all | grep "\-\-retry-all-errors" > /dev/null
	[ $? == 0 ] && extra_params="--retry-all-errors ${extra_params}"
	status=$(/usr/bin/curl -I -o /dev/null -skL $extra_params --connect-timeout ${timeout} --retry ${try} -w %{http_code} "$url")
	case "$status" in
		204|\
		200)
			status=200
		;;
	esac
	echo $status
}

test_proxy() {
	result=0
	status=$(test_url "https://www.google.com/generate_204" ${retry_num} ${connect_timeout})
	if [ "$status" = "200" ]; then
		result=0
	else
		status2=$(test_url "https://www.baidu.com" ${retry_num} ${connect_timeout})
		if [ "$status2" = "200" ]; then
			result=1
		else
			result=2
			ping -c 3 -W 1 223.5.5.5 > /dev/null 2>&1
			[ $? -eq 0 ] && {
				result=1
			}
		fi
	fi
	echo $result
}

test_node() {
	local node_id=$1
	local _type=$(echo $(config_n_get ${node_id} type nil) | tr 'A-Z' 'a-z')
	[ "${_type}" != "nil" ] && {
		if [ "${_type}" == "socks" ]; then
			local _address=$(config_n_get ${node_id} address)
			local _port=$(config_n_get ${node_id} port)
			[ -n "${_address}" ] && [ -n "${_port}" ] && {
				local curlx="socks5h://${_address}:${_port}"
				local _username=$(config_n_get ${node_id} username)
				local _password=$(config_n_get ${node_id} password)
				[ -n "${_username}" ] && [ -n "${_password}" ] && curlx="socks5h://${_username}:${_password}@${_address}:${_port}"
			}
		else
			local _tmp_port=$(/usr/share/${CONFIG}/app.sh get_new_port 61080 tcp)
			/usr/share/${CONFIG}/app.sh run_socks flag=auto_switch node=$node_id bind=127.0.0.1 socks_port=${_tmp_port} config_file=/var/etc/${CONFIG}/test.json
			local curlx="socks5h://127.0.0.1:${_tmp_port}"
		fi
		_proxy_status=$(test_url "https://www.google.com/generate_204" ${retry_num} ${connect_timeout} "-x $curlx")
		pgrep -f "/var/etc/${CONFIG}/test\.json|auto_switch" | xargs kill -9 >/dev/null 2>&1
		rm -rf "/var/etc/${CONFIG}/test.json"
		if [ "${_proxy_status}" -eq 200 ]; then
			return 0
		fi
	}
	return 1
}

flag=0
main_node=$(config_t_get global tcp_node nil)

test_auto_switch() {
	flag=$(expr $flag + 1)
	local TYPE=$1
	local b_tcp_nodes=$2
	local now_node=$3
	[ -z "$now_node" ] && {
		if [ -f "/var/etc/$CONFIG/id/${TYPE}" ]; then
			now_node=$(cat /var/etc/$CONFIG/id/${TYPE})
			if [ "$(config_n_get $now_node protocol nil)" = "_shunt" ]; then
				if [ "$shunt_logic" == "1" ] && [ -f "/var/etc/$CONFIG/id/${TYPE}_default" ]; then
					now_node=$(cat /var/etc/$CONFIG/id/${TYPE}_default)
				elif [ "$shunt_logic" == "2" ] && [ -f "/var/etc/$CONFIG/id/${TYPE}_main" ]; then
					now_node=$(cat /var/etc/$CONFIG/id/${TYPE}_main)
				else
					shunt_logic=0
				fi
			else
				shunt_logic=0
			fi
		else
			#echolog "自动切换检测：未知错误"
			return 1
		fi
	}
	
	[ $flag -le 1 ] && {
		main_node=$now_node
	}

	status=$(test_proxy)
	if [ "$status" == 2 ]; then
		echolog "自动切换检测：无法连接到网络，请检查网络是否正常！"
		return 2
	fi
	
	#检测主节点是否能使用
	if [ "$restore_switch" == "1" ] && [ "$main_node" != "nil" ] && [ "$now_node" != "$main_node" ]; then
		test_node ${main_node}
		[ $? -eq 0 ] && {
			#主节点正常，切换到主节点
			echolog "自动切换检测：${TYPE}主节点【$(config_n_get $main_node type)：[$(config_n_get $main_node remarks)]】正常，切换到主节点！"
			/usr/share/${CONFIG}/app.sh node_switch ${TYPE} ${main_node} ${shunt_logic} 1
			[ $? -eq 0 ] && {
				echolog "自动切换检测：${TYPE}节点切换完毕！"
				[ "$shunt_logic" != "0" ] && {
					local tcp_node=$(config_t_get global tcp_node nil)
					[ "$(config_n_get $tcp_node protocol nil)" = "_shunt" ] && {
						if [ "$shunt_logic" == "1" ]; then
							uci set $CONFIG.$tcp_node.default_node="$main_node"
						elif [ "$shunt_logic" == "2" ]; then
							uci set $CONFIG.$tcp_node.main_node="$main_node"
						fi
						uci commit $CONFIG
					}
				}
			}
			return 0
		}
	fi
	
	if [ "$status" == 0 ]; then
		#echolog "自动切换检测：${TYPE}节点【$(config_n_get $now_node type)：[$(config_n_get $now_node remarks)]】正常。"
		return 0
	elif [ "$status" == 1 ]; then
		echolog "自动切换检测：${TYPE}节点【$(config_n_get $now_node type)：[$(config_n_get $now_node remarks)]】异常，切换到下一个备用节点检测！"
		local new_node
		in_backup_nodes=$(echo $b_tcp_nodes | grep $now_node)
		# 判断当前节点是否存在于备用节点列表里
		if [ -z "$in_backup_nodes" ]; then
			# 如果不存在，设置第一个节点为新的节点
			new_node=$(echo $b_tcp_nodes | awk -F ' ' '{print $1}')
		else
			# 如果存在，设置下一个备用节点为新的节点
			#local count=$(expr $(echo $b_tcp_nodes | grep -o ' ' | wc -l) + 1)
			local next_node=$(echo $b_tcp_nodes | awk -F "$now_node" '{print $2}' | awk -F " " '{print $1}')
			if [ -z "$next_node" ]; then
				new_node=$(echo $b_tcp_nodes | awk -F ' ' '{print $1}')
			else
				new_node=$next_node
			fi
		fi
		test_node ${new_node}
		if [ $? -eq 0 ]; then
			[ "$restore_switch" == "0" ] && {
				[ "$shunt_logic" == "0" ] && uci set $CONFIG.@global[0].tcp_node=$new_node
				[ -z "$(echo $b_tcp_nodes | grep $main_node)" ] && uci add_list $CONFIG.@auto_switch[0].tcp_node=$main_node
				uci commit $CONFIG
			}
			echolog "自动切换检测：${TYPE}节点【$(config_n_get $new_node type)：[$(config_n_get $new_node remarks)]】正常，切换到此节点！"
			/usr/share/${CONFIG}/app.sh node_switch ${TYPE} ${new_node} ${shunt_logic} 1
			[ $? -eq 0 ] && {
				[ "$restore_switch" == "1" ] && [ "$shunt_logic" != "0" ] && {
					local tcp_node=$(config_t_get global tcp_node nil)
					[ "$(config_n_get $tcp_node protocol nil)" = "_shunt" ] && {
						if [ "$shunt_logic" == "1" ]; then
							uci set $CONFIG.$tcp_node.default_node="$main_node"
						elif [ "$shunt_logic" == "2" ]; then
							uci set $CONFIG.$tcp_node.main_node="$main_node"
						fi
						uci commit $CONFIG
					}
				}
				echolog "自动切换检测：${TYPE}节点切换完毕！"
			}
			return 0
		else
			test_auto_switch ${TYPE} "${b_tcp_nodes}" ${new_node}
		fi
	fi
}

start() {
	if [ "$(pgrep -f $CONFIG/test.sh | wc -l)" -gt 2 ]; then
		exit 1
	fi
	ENABLED=$(config_t_get global enabled 0)
	[ "$ENABLED" != 1 ] && return 1
	ENABLED=$(config_t_get auto_switch enable 0)
	[ "$ENABLED" != 1 ] && return 1
	delay=$(config_t_get auto_switch testing_time 1)
	#sleep ${delay}m
	#sleep 9s
	connect_timeout=$(config_t_get auto_switch connect_timeout 3)
	retry_num=$(config_t_get auto_switch retry_num 3)
	restore_switch=$(config_t_get auto_switch restore_switch 0)
	shunt_logic=$(config_t_get auto_switch shunt_logic 0)
	while [ "$ENABLED" -eq 1 ]
	do
		TCP_NODE=$(config_t_get auto_switch tcp_node nil)
		[ -n "$TCP_NODE" -a "$TCP_NODE" != "nil" ] && {
			TCP_NODE=$(echo $TCP_NODE | tr -s ' ' '\n' | uniq | tr -s '\n' ' ')
			test_auto_switch TCP "$TCP_NODE"
		}
		sleep ${delay}m
	done
}

arg1=$1
shift
case $arg1 in
test_url)
	test_url $@
	;;
*)
	start
	;;
esac
