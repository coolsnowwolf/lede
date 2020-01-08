#!/bin/bash
# Copyright (C) 2017 XiaoShan https://www.mivm.cn

. /usr/share/libubox/jshn.sh

urlsafe_b64decode() {
    local d="====" data=$(echo $1 | sed 's/_/\//g; s/-/+/g')
    local mod4=$((${#data}%4))
    [ $mod4 -gt 0 ] && data=${data}${d:mod4}
    echo $data | base64 -d
}

urldecode() { : "${*//+/ }"; echo -e "${_//%/\\x}"; }

echo_date(){
	echo $(TZ=UTC-8 date -R +%Y-%m-%d\ %X):$1
}

Server_Update() {
    local uci_set="uci -q set $name.$1."
    ${uci_set}grouphashkey="$ssr_grouphashkey"
    ${uci_set}hashkey="$ssr_hashkey"
    ${uci_set}alias="[$ssr_group] $ssr_remarks"
    ${uci_set}auth_enable="0"
    ${uci_set}switch_enable="1"
    ${uci_set}type="$ssr_type"
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
    
	if [ "$ssr_type" = "v2ray" ]; then
    #v2ray
    ${uci_set}alter_id="$ssr_alter_id"
    ${uci_set}vmess_id="$ssr_vmess_id"
    ${uci_set}transport="$ssr_transport"
    ${uci_set}tcp_guise="$ssr_tcp_guise"
    ${uci_set}ws_host="$ssr_ws_host"
    ${uci_set}ws_path="$ssr_ws_path"
    ${uci_set}h2_host="$ssr_h2_host"
    ${uci_set}h2_path="$ssr_h2_path"
    ${uci_set}tls="$ssr_tls"
    ${uci_set}security=$ssr_security
    ${uci_set}alias="$ssr_remarks"
	fi
	
	if [ "$ssr_type" = "ss" ]; then
    ${uci_set}encrypt_method_ss="$ss_method"
    ${uci_set}alias="$ssr_remarks"
	fi

}

name=shadowsocksr
subscribe_url=($(uci get $name.@server_subscribe[0].subscribe_url)) #订阅服务器地址
[ ${#subscribe_url[@]} -eq 0 ] && exit 1
[ $(uci -q get $name.@server_subscribe[0].proxy || echo 0) -eq 0 ] && /etc/init.d/$name stop >/dev/null 2>&1
log_name=${name}_subscribe
for ((o=0;o<${#subscribe_url[@]};o++))
do
	echo_date "从 ${subscribe_url[o]} 获取订阅"
	echo_date "开始更新在线订阅列表..."
	echo_date "尝试下载订阅链接到本地临时文件，请稍等..."
	subscribe_data=$(wget-ssl --no-check-certificate -t 3 -T 30 -O- ${subscribe_url[o]})
	curl_code=$?
	# 计算group的hashkey
	ssr_grouphashkey=$(echo "${subscribe_url[o]}" | md5sum | cut -d ' ' -f1)
	if [ ! $curl_code -eq 0 ];then
		echo_date "下载订阅失败，自动重试中..."
		subscribe_data=$(wget-ssl --user-agent="Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/77.0.3865.90 Safari/537.36"  --no-check-certificate -t 3 -T 30 -O- ${subscribe_url[o]})
		curl_code=$?
	fi
	if [ $curl_code -eq 0 ];then
		echo_date "下载订阅成功，开始解析节点信息..."
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
					[ $temp_x -lt ${#ssr_url[@]} -a -z "$(echo "${subscribe_max_x[*]}" | grep -w $temp_x)" ] && subscribe_max_x[${#subscribe_max_x[@]}]="$temp_x"
				done
			else
				subscribe_max=${#ssr_url[@]}
			fi
			echo_date "共计$subscribe_max个节点"

			ssr_group=$(urlsafe_b64decode $(urlsafe_b64decode ${ssr_url[$((${#ssr_url[@]} - 1))]//ssr:\/\//} | sed 's/&/\n/g' | grep group= | awk -F = '{print $2}'))
			if [ -z "$ssr_group" ]; then
				ssr_group="default"
			fi
			if [ -n "$ssr_group" ]; then
				subscribe_i=0
				subscribe_n=0
				subscribe_o=0
				subscribe_x=""
				temp_host_o=()
				curr_ssr=$(uci show $name | grep @servers | grep -c server=)
				for ((x=0;x<$curr_ssr;x++)) # 循环已有服务器信息，匹配当前订阅群组
				do
					temp_alias=$(uci -q get $name.@servers[$x].grouphashkey | grep "$ssr_grouphashkey")
					[ -n "$temp_alias" ] && temp_host_o[${#temp_host_o[@]}]=$(uci get $name.@servers[$x].hashkey)
				done

				for ((x=0;x<$subscribe_max;x++)) # 循环链接
				do
					[ ${#subscribe_max_x[@]} -eq 0 ] && temp_x=$x || temp_x=${subscribe_max_x[x]}
					result=$(echo ${ssr_url[temp_x]} | grep "ss")
					subscribe_url_type=$(echo "$ssr_url" | awk -F ':' '{print $1}')
					
					if [ "$subscribe_url_type" = "ss" ]; then
				    		temp_info=${ssr_url[temp_x]//ss:\/\//} # 解码 SS 链接
					   	# 计算hashkey
						ssr_hashkey=$(echo "$temp_info" | md5sum | cut -d ' ' -f1)

						info=$(urlsafe_b64decode $(echo "$temp_info" | awk -F '@' '{print $1}'))
						temp_info_array=(${info//:/ })
						ssr_type="ss"
						ss_method=${temp_info_array[0]}
						ssr_passwd=${temp_info_array[1]}
						info=$(echo "$temp_info" | awk -F '@' '{print $2}' | awk -F '#' '{print $1}')
						temp_info_array=(${info//:/ })
						ssr_host=${temp_info_array[0]}
						ssr_port=${temp_info_array[1]}
						ssr_remarks=$(urldecode $(echo "$temp_info" | awk -F '#' '{print $2}'))
				    	fi
				    
					if [ "$subscribe_url_type" = "ssr" ]; then
						temp_info=$(urlsafe_b64decode ${ssr_url[temp_x]//ssr:\/\//}) # 解码 SSR 链接
						# 计算hashkey
						ssr_hashkey=$(echo "$temp_info" | md5sum | cut -d ' ' -f1)

						info=${temp_info///?*/}
						temp_info_array=(${info//:/ })
						ssr_type="ssr"
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
						    		ssr_protoparam=$(urlsafe_b64decode ${temp_info[1]});;
                                				obfsparam)
                                				ssr_obfsparam=$(urlsafe_b64decode ${temp_info[1]});;
                                				remarks)
                                				ssr_remarks=$(urlsafe_b64decode ${temp_info[1]});;
                            				esac
                        			done
                    			fi
				    
				   	if [ "$subscribe_url_type" = "vmess" ]; then
                        			temp_info=$(urlsafe_b64decode ${ssr_url[temp_x]//vmess:\/\//}) # 解码 Vmess 链接
                        			# 计算hashkey
                        			ssr_hashkey=$(echo "$temp_info" | md5sum | cut -d ' ' -f1)
			         	
                        			ssr_type="v2ray"
                       				json_load "$temp_info"
                        			json_get_var ssr_host add
                        			json_get_var ssr_port port
                        			json_get_var ssr_alter_id aid
                        			json_get_var ssr_vmess_id id
                        			json_get_var ssr_security security
                        			if [ "$ssr_security" == "" ]; then
                         			   ssr_security="auto"
                        			fi
                        			json_get_var ssr_transport net
                        			json_get_var ssr_remarks ps
                        			ssr_tcp_guise="none"
                        			json_get_var ssr_ws_host host
                        			json_get_var ssr_ws_path path
                        			json_get_var ssr_h2_host host
                        			json_get_var ssr_h2_path path
                        			json_get_var ssr_tls tls
                        			if [ "$ssr_tls" == "tls" -o "$ssr_tls" == "1" ]; then
                         			   ssr_tls="1"
                        			else
                         			   ssr_tls="0"
                        			fi
                   			fi
                   	
                   			  if [ -z "ssr_remarks" ]; then # 没有备注的话则生成一个
                        			ssr_remarks="$ssr_host:$ssr_port";
                    			fi

              # 丢弃没有host的无效服务器信息，例如：剩余流量xxx
              if [[ "$ssr_remarks" =~ "过期时间" ]] || [[ "$ssr_remarks" =~ "剩余流量" ]] || [[ "$ssr_remarks" =~ "防失联QQ群" ]] || [ -z "$ssr_host" ];then
								echo_date "丢弃无效节点：【$ssr_remarks】"
								continue
							fi

                    			uci_name_tmp=$(uci show $name | grep -w "$ssr_hashkey" | awk -F . '{print $2}')
		    	   		if [ -z "$uci_name_tmp" ]; then # 判断当前服务器信息是否存在
                        			uci_name_tmp=$(uci add $name servers)
                       		 		subscribe_n=$(($subscribe_n + 1))
                    			fi
                    			Server_Update $uci_name_tmp
                    			subscribe_x=$subscribe_x$ssr_hashkey" "
                    			ssrtype=$(echo $ssr_type | tr '[a-z]' '[A-Z]')
                    			echo_date "$ssrtype节点：【$ssr_remarks】"
		
                    			# SSR
                    			# echo "服务器地址: $ssr_host"
                    			# echo "服务器端口 $ssr_port"
                    			# echo "密码: $ssr_passwd"
                    			# echo "SS加密: $ss_method"
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
                		echo_date "本次更新订阅来源 【$ssr_group】 服务器数量: ${#ssr_url[@]} 新增服务器: $subscribe_n 删除服务器: $subscribe_o"
                		echo_date "在线订阅列表更新完成!请等待网页自动刷新!"
                		subscribe_log="$ssr_group 服务器订阅更新成功 服务器数量: ${#ssr_url[@]} 新增服务器: $subscribe_n 删除服务器: $subscribe_o"
                		logger -st $log_name[$$] -p6 "$subscribe_log"
                		uci commit $name
            		else
                	echo_date "${subscribe_url[$o]} 订阅数据解析失败 无法获取 Group"
                	logger -st $log_name[$$] -p3 "${subscribe_url[$o]} 订阅数据解析失败 无法获取 Group"
            	fi
    	else
        echo_date "${subscribe_url[$o]} 订阅数据获取失败 错误代码: $curl_code"
        logger -st $log_name[$$] -p3 "${subscribe_url[$o]} 订阅数据获取失败 错误代码: $curl_code"
	fi
done
/etc/init.d/$name restart >/dev/null 2>&1
