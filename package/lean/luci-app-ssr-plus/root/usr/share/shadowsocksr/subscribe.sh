#!/bin/bash
# Copyright (C) 2017 XiaoShan https://www.mivm.cn

urlsafe_b64decode() {
    local d="====" data=$(echo $1 | sed 's/_/\//g; s/-/+/g')
    local mod4=$((${#data}%4))
    [ $mod4 -gt 0 ] && data=${data}${d:mod4}
    echo $data | base64 -d
}

Server_Update() {
    local uci_set="uci -q set $name.$1."
    ${uci_set}alias="[$ssr_group] $ssr_remarks"
    ${uci_set}auth_enable="0"
    ${uci_set}switch_enable="1"
    ${uci_set}server="$ssr_host"
    ${uci_set}server_port="$ssr_port"
    ${uci_set}local_port="1234"
    uci -q get $name.@servers[$1].timeout >/dev/null || ${uci_set}timeout="60"
    ${uci_set}password="$ssr_passwd"
    ${uci_set}encrypt_method="$ssr_method"
    ${uci_set}protocol="$ssr_protocol"
    ${uci_set}protocol_param="$ssr_protoparam"
    ${uci_set}obfs="$ssr_obfs"
    ${uci_set}obfs_param="$ssr_obfsparam"
    ${uci_set}fast_open="0"
    ${uci_set}kcp_enable="0"
    ${uci_set}kcp_port="0"
    ${uci_set}kcp_param="--nocomp"
}

name=shadowsocksr
subscribe_url=($(uci get $name.@server_subscribe[0].subscribe_url))
[ ${#subscribe_url[@]} -eq 0 ] && exit 1
[ $(uci -q get $name.@server_subscribe[0].proxy || echo 0) -eq 0 ] && /etc/init.d/$name stop >/dev/null 2>&1
log_name=${name}_subscribe
for ((o=0;o<${#subscribe_url[@]};o++))
do
    subscribe_data=$(wget-ssl --user-agent="User-Agent: Mozilla" --no-check-certificate -T 3 -O- ${subscribe_url[o]})
    curl_code=$?
    if [ ! $curl_code -eq 0 ];then
      subscribe_data=$(wget-ssl --no-check-certificate -T 3 -O- ${subscribe_url[o]})
      curl_code=$?
    fi
    if [ $curl_code -eq 0 ];then
        ssr_url=($(echo $subscribe_data | base64 -d | sed 's/\r//g')) # 解码数据并删除 \r 换行符
        subscribe_max=$(echo ${ssr_url[0]} | grep -i MAX= | awk -F = '{print $2}') 
        subscribe_max_x=()
        if [ -n "$subscribe_max" ]; then
            while [ ${#subscribe_max_x[@]} -ne $subscribe_max ]
            do
                if [ ${#ssr_url[@]} -ge 10 ]; then
                    if [ $((${RANDOM:0:2}%2)) -eq 0 ]; then
                        temp_x=${RANDOM:0:1}
                    else
                        temp_x=${RANDOM:0:2}
                    fi
                else
                    temp_x=${RANDOM:0:1}
                fi
                [ $temp_x -lt ${#ssr_url[@]} -a -z "$(echo "${subscribe_max_x[*]}" | grep -w ${temp_x})" ] && subscribe_max_x[${#subscribe_max_x[@]}]="$temp_x"
            done
        else
            subscribe_max=${#ssr_url[@]}
        fi
        ssr_group=$(urlsafe_b64decode $(urlsafe_b64decode ${ssr_url[$((${#ssr_url[@]} - 1))]//ssr:\/\//} | sed 's/&/\n/g' | grep group= | awk -F = '{print $2}'))
        if [ -n "$ssr_group" ]; then
            subscribe_i=0
            subscribe_n=0
            subscribe_o=0
            subscribe_x=""
            temp_host_o=()
            curr_ssr=$(uci show $name | grep @servers | grep -c server=)
            for ((x=0;x<$curr_ssr;x++)) # 循环已有服务器信息，匹配当前订阅群组
            do
                temp_alias=$(uci -q get $name.@servers[$x].alias | grep "\[$ssr_group\]")
                [ -n "$temp_alias" ] && temp_host_o[${#temp_host_o[@]}]=$(uci get $name.@servers[$x].server)
            done
            for ((x=0;x<$subscribe_max;x++)) # 循环链接
            do
                [ ${#subscribe_max_x[@]} -eq 0 ] && temp_x=$x || temp_x=${subscribe_max_x[x]}
                temp_info=$(urlsafe_b64decode ${ssr_url[temp_x]//ssr:\/\//}) # 解码 SSR 链接
                # 依次获取基本信息
                info=${temp_info///?*/}
                temp_info_array=(${info//:/ })
                ssr_host=${temp_info_array[0]}
                ssr_port=${temp_info_array[1]}
                ssr_protocol=${temp_info_array[2]}
                ssr_method=${temp_info_array[3]}
                ssr_obfs=${temp_info_array[4]}
                ssr_passwd=$(urlsafe_b64decode ${temp_info_array[5]})
                info=${temp_info:$((${#info} + 2))}
                info=(${info//&/ })
                ssr_protoparam=""
                ssr_obfsparam=""
                ssr_remarks="$temp_x"
                for ((i=0;i<${#info[@]};i++)) # 循环扩展信息
                do
                    temp_info=($(echo ${info[i]} | sed 's/=/ /g'))
                    case "${temp_info[0]}" in
                        protoparam)
                            ssr_protoparam=$(urlsafe_b64decode ${temp_info[1]})
                        ;;
                        obfsparam)
                            ssr_obfsparam=$(urlsafe_b64decode ${temp_info[1]})
                        ;;
                        remarks)
                            ssr_remarks=$(urlsafe_b64decode ${temp_info[1]})
                        ;;
                    esac
                done
               
                uci_name_tmp=$(uci show $name | grep -w $ssr_host | awk -F . '{print $2}')
                if [ -z "$uci_name_tmp" ]; then # 判断当前服务器信息是否存在
                    uci_name_tmp=$(uci add $name servers)
                    subscribe_n=$(($subscribe_n + 1))
                fi
                Server_Update $uci_name_tmp
                subscribe_x=${subscribe_x}$ssr_host" "

                # echo "服务器地址: $ssr_host"
                # echo "服务器端口 $ssr_port"
                # echo "密码: $ssr_passwd"
                # echo "加密: $ssr_method"
                # echo "协议: $ssr_protocol"
                # echo "协议参数: $ssr_protoparam"
                # echo "混淆: $ssr_obfs"
                # echo "混淆参数: $ssr_obfsparam"
                # echo "备注: $ssr_remarks"
            done
            for ((x=0;x<${#temp_host_o[@]};x++)) # 新旧服务器信息匹配，如果旧服务器信息不存在于新服务器信息则删除
            do
                if [ -z "$(echo "$subscribe_x" | grep -w ${temp_host_o[x]})" ]; then
                    uci_name_tmp=$(uci show $name | grep ${temp_host_o[x]} | awk -F . '{print $2}')
                    uci delete $name.$uci_name_tmp
                    subscribe_o=$(($subscribe_o + 1))
                fi
            done
            subscribe_log="$ssr_group 服务器订阅更新成功 服务器数量: ${#ssr_url[@]} 新增服务器: $subscribe_n 删除服务器: $subscribe_o"
            logger -st $log_name[$$] -p6 "$subscribe_log"
            uci commit $name
        else
            logger -st $log_name[$$] -p3 "${subscribe_url[$o]} 订阅数据解析失败 无法获取 Group"
        fi
    else
        logger -st $log_name[$$] -p3 "${subscribe_url[$o]} 订阅数据获取失败 错误代码: $curl_code"
    fi
done
/etc/init.d/$name restart >/dev/null 2>&1
