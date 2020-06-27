#!/bin/sh

#####################################
#
#更新日期2018.1.14
#修改你的SCKEY
#Log文件在 /var/log/server_chan.log 默认保存500条，可修改
#缓存文件在/root/var/ 可修改
#${dir}mac_name文件保存已知设备，格式为【mac 名称】（中间是个空格，不含方括号）
#starttime endtime 脚本运行起止时间
#默认检测时间60s
#开启设备上下线提醒必须开新设备加入
#
#####################################
serverchan_sckey=`uci get /etc/config/serverchan.@serverchan[0].serverchan_sckey`
serverchan_notify_1=`uci get /etc/config/serverchan.@serverchan[0].new_ip` #外网IP变化
serverchan_notify_2=`uci get /etc/config/serverchan.@serverchan[0].new_equipment` #新设备加入
serverchan_notify_3=`uci get /etc/config/serverchan.@serverchan[0].equipment` #设备上下线
serverchan_notify_4=`uci get /etc/config/serverchan.@serverchan[0].proxy` #代理状态
serverchan_enable=`uci get /etc/config/serverchan.@serverchan[0].enabled`
serverchan_enable=${serverchan_enable:-"0"}
hostname=`uci get /etc/config/system.@system[0].hostname`
starttime=`uci get /etc/config/serverchan.@serverchan[0].starttime`
endtime=`uci get /etc/config/serverchan.@serverchan[0].endtime`
sleeptime=60
dir="/root/var/"
mkdir -p ${dir}
touch ${dir}arIpAddress
sed -i '1,100d' ${dir}arIpAddress
touch ${dir}lastIPAddress
touch ${dir}mac_name
logfile="/var/log/server_chan.log"
scfile="/var/log/serverchan.file"
touch ${logfile}
curl -s "http://sc.ftqq.com/$serverchan_sckey.send?text=%e3%80%90${hostname}%e3%80%91%e8%b7%af%e7%94%b1%e6%88%96%e6%8f%92%e4%bb%b6%e9%87%8d%e5%90%af"
echo `date "+%H:%M:%S"` >> ${logfile}
echo "路由或插件重启！" >> ${logfile}

new_ip(){
wan_ip=$(ubus call network.interface.wan status | grep "address" | grep -oE '[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}')
echo "$wan_ip" >> ${dir}arIpAddress

hostIP=$(cat ${dir}arIpAddress|sed -n '1p')

lastIP=$(cat ${dir}lastIPAddress|sed -n '1p')

}
	
clean_log(){
logrow=$(grep -c "" ${logfile})
scfilerow=$(grep -c "" ${scfile})
if [ $logrow -ge 500 ];then
    cat /dev/null > ${logfile}
    echo "$curtime Log条数超限，清空处理！" >> ${logfile}
fi
if [ $scfilerow -ge 500 ];then
    cat /dev/null > ${scfile}
    echo "$curtime Log条数超限，清空处理！" >> ${logfile}
fi
}

while [ "$serverchan_enable" = "1" ];
do
serverchan_enable=1
serverchan_enable=${serverchan_enable:-"0"}
curltest=`which curl`
nowtime=`date "+%Y-%m-%d %H:%M:%S"`
if [ -z "$curltest" ] ; then
    wget --no-check-certificate  -q -T 10 http://www.163.com
    [ "$?" == "0" ] && check=200 || check=404
else
    check=`curl -k -s -w "%{http_code}" "http://www.163.com" -o /dev/null`
fi

if [ "$check" == "200" ] ; then
curtime=`date "+%H:%M:%S"`

echo "$curtime online! "
currtime=`date "+%H"`
	if [ "$currtime" -gt "$starttime" ] && [ "$currtime" -lt "$endtime" ]; then
	clean_log
		#外网IP变化
		if [ "$serverchan_notify_1" = "1" ]; then
			new_ip
			if [ "$lastIP" != "$hostIP" ] && [ ! -z "$hostIP" ]; then
				sleep ${sleeptime}
				new_ip
			fi
			if [ "$lastIP" != "$hostIP" ] && [ ! -z "$hostIP" ] ; then
			    echo "$curtime 【外网IP变化】 当前 IP: $hostIP" >> ${logfile}
				echo "$curtime 【外网IP变化】 上次 IP: $lastIP" >> ${logfile}
				title="%e3%80%90${hostname}%e3%80%91%e5%a4%96%e7%bd%91IP%e5%9c%b0%e5%9d%80%e5%8f%98%e5%8c%96"
				content="%e5%bd%93%e5%89%8dIP%ef%bc%9a${hostIP}"
				curl -s "http://sc.ftqq.com/$serverchan_sckey.send?text=${title}" -d "&desp=%e6%a3%80%e6%b5%8b%e6%97%b6%e9%97%b4%ef%bc%9a${nowtime}%0D%0A%0D%0A${content}"
				echo "$curtime 【微信推送】 当前IP：${hostIP}" >> ${logfile}
				echo -n $hostIP > ${dir}lastIPAddress
				sed -i '1,100d' ${dir}arIpAddress
			else
				sed -i '1,100d' ${dir}arIpAddress
			fi
			
		fi


		#新设备加入
		if [ "$serverchan_notify_2" = "1" ] ; then
			cat /proc/net/arp | grep -v "^$" | awk '{if($0~"br-lan") print}' | awk -F "[ ]+" '{print $4}' | sort -u > ${dir}tmpfile
			sed -i '$d' ${dir}tmpfile
			touch ${dir}mac_all
			grep -F -v -f "${dir}mac_all" "${dir}tmpfile" | sort | uniq > ${dir}mac_add
			if [ -s "${dir}mac_add" ] ; then
			    cat ${dir}mac_add >> ${dir}mac_all
			    title="%e3%80%90${hostname}%e3%80%91%e6%96%b0%e8%ae%be%e5%a4%87%e6%8f%90%e9%86%92"
				content=`cat ${dir}mac_add | grep -v "^$" | sed 's/$/ %0D%0A%0D%0A/'`
				curl -s "http://sc.ftqq.com/$serverchan_sckey.send?text=${title}" -d "&desp=%e6%a3%80%e6%b5%8b%e6%97%b6%e9%97%b4%ef%bc%9a${nowtime}%0D%0A%0D%0A${content}"
				echo "$curtime 【微信推送】 新设备加入 " >> ${logfile}
				cat ${dir}mac_add | grep -v "^$"  >> ${logfile}
			fi
		fi	
			
			
		#设备上下线提醒
		if [ "$serverchan_notify_3" = "1" ] ; then
		    cat /proc/net/arp | grep -v "^$" | awk '{if($0~"br-lan") print}' | awk -F "[ ]+" '{print $4,$3,$1}' | sort -u > ${dir}arp_now
			sed -i '$d' ${dir}arp_now
			cat ${dir}mac_all | awk -F' ' 'BEGIN{OFS=" "}{if($2==""){$2="0xF";}print;}' | awk -F' ' 'BEGIN{OFS=" "}{if($3==""){$3="0.0.0.0";}print;}' > ${dir}tmpfile
			awk -F' ' 'NR==FNR{a[$1]=$2" "$3;next}{for(i in a){if($1==i)$2=a[$1]}}1' ${dir}arp_now  ${dir}tmpfile >  ${dir}tmpfile2
			cat ${dir}tmpfile2 | awk '{ $4=null;print $0 }' >${dir}tmpfile
			#echo "$curtime check! "
			#sleep ${sleeptime}
			#awk -F "[ ]" 'NR==FNR{a[$1]=$2" "$3;next}{print $1" "a[$1]}'  ${dir}arp_now  ${dir}tmpfile >  ${dir}tmpfile2
			awk -F "[ ]" 'NR==FNR{a[$1]=$2;next}{print $2" "$1" "$3" "a[$1]}'  ${dir}mac_name  ${dir}tmpfile >  ${dir}tmpfile2
			awk -F' ' 'BEGIN{OFS=" "}{if($4==""){$4="unknown";}print;}' ${dir}tmpfile2 > ${dir}mac_state_now_name
			touch ${dir}mac_state_last_name
			grep -F -v -f "${dir}mac_state_last_name" "${dir}mac_state_now_name" | sort | uniq > ${dir}mac_state_change_name
			if [ -s "${dir}mac_state_change_name" ] ; then
				cp ${dir}mac_state_now_name ${dir}mac_state_last_name
				echo "$curtime 【微信推送】 设备状态变化" >> ${logfile}
				cat ${dir}mac_state_change_name | sed 's/0x2/在线/g' | sed 's/0x0/离线/g' | sed 's/0xF/未知/g' >> ${logfile}
				echo "======↓历史设备状态↓======" >> ${logfile}
				cat ${dir}mac_state_now_name | sed 's/0x2/在线/g' | sed 's/0x0/离线/g' | sed 's/0xF/未知/g' >> ${logfile}
				sed -i 's/0x2/%e5%9c%a8%e7%ba%bf/g' ${dir}mac_state_change_name
				sed -i 's/0x0/%e7%a6%bb%e7%ba%bf/g' ${dir}mac_state_change_name
				sed -i 's/0xF/%e6%9c%aa%e7%9f%a5/g' ${dir}mac_state_change_name
				sed -i 's/0x2/%e5%9c%a8%e7%ba%bf/g' ${dir}mac_state_now_name
				sed -i 's/0x0/%e7%a6%bb%e7%ba%bf/g' ${dir}mac_state_now_name
				sed -i 's/0xF/%e6%9c%aa%e7%9f%a5/g' ${dir}mac_state_now_name
				title="%e3%80%90${hostname}%e3%80%91%e8%ae%be%e5%a4%87%e7%8a%b6%e6%80%81%e5%8f%98%e5%8c%96"
				content1=`cat ${dir}mac_state_change_name  |awk '{ $2=null;print $0 }' | grep -v "^$" | sed 's/$/ %0D%0A%0D%0A/'`
				content2=`cat ${dir}mac_state_now_name |awk '{ $2=null;print $0 }' | grep -v "^$" | sed 's/$/ %0D%0A%0D%0A/'`
				curl -s "http://sc.ftqq.com/$serverchan_sckey.send?text=${title}" -d "&desp=%e6%a3%80%e6%b5%8b%e6%97%b6%e9%97%b4%ef%bc%9a${nowtime}%0D%0A%0D%0A${content1}%0D%0A%0D%0A%3d%3d%3d%3d%3d%3d%e2%86%93%e5%8e%86%e5%8f%b2%e8%ae%be%e5%a4%87%e7%8a%b6%e6%80%81%e2%86%93%3d%3d%3d%3d%3d%3d%0D%0A%0D%0A${content2}"
			fi
		fi
		
		#代理状态提醒
		if [ "$serverchan_notify_4" = "1" ] ; then
			/usr/bin/wget --spider --quiet --tries=1 --timeout=3 www.gstatic.com/generate_204
			if [ "$?" == "0" ]; then
				echo "$curtime 【微信推送】 代理正常" >> ${logfile}
			else
				/usr/bin/wget --spider --quiet --tries=1 --timeout=3 www.baidu.com
				if [ "$?" == "0" ]; then
					echo "$curtime 【微信推送】 代理出现问题等待插件重启" >> ${logfile}
					title="%e3%80%90${hostname}%e3%80%91%e4%bb%a3%e7%90%86%e7%8a%b6%e6%80%81%e5%8f%98%e5%8c%96"
					curl -s "http://sc.ftqq.com/$serverchan_sckey.send?text=${title}" -d "&desp=%e6%a3%80%e6%b5%8b%e6%97%b6%e9%97%b4%ef%bc%9a${nowtime}%e4%bb%a3%e7%90%86%e5%87%ba%e7%8e%b0%e9%97%ae%e9%a2%98%e7%ad%89%e5%be%85%e6%8f%92%e4%bb%b6%e9%87%8d%e5%90%af"
				else
					echo "$curtime 【路由断网】 当前网络不通！ " >> ${logfile}
				fi
			fi
		fi
	fi

else
	echo "$curtime 【路由断网】 当前网络不通！ " >> ${logfile}
fi

sleep ${sleeptime}
continue
done

